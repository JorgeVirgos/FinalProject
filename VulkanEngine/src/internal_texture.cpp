#include "internal_texture.h"
#include "render_context.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

VkSampler VKE::InternalTexture::default_sampler_ = VK_NULL_HANDLE;
bool VKE::InternalTexture::has_created_sample_ = false;

VKE::InternalTexture::InternalTexture() {
	width_ = 0;
	height_ = 0;

	image_ = VK_NULL_HANDLE;
	image_view_ = VK_NULL_HANDLE;
	image_memory_ = VK_NULL_HANDLE;

	has_been_initialised_ = false;
	has_been_allocated_ = false;
}

VKE::InternalTexture::~InternalTexture() {

}

void VKE::InternalTexture::reset(RenderContext* render_ctx) {

	if (has_been_allocated_) {
		vkDestroyImageView(render_ctx->getDevice(), image_view_, nullptr);
		vkFreeMemory(render_ctx->getDevice(), image_memory_, nullptr);
	}

	if (has_been_initialised_) {
		vkDestroyImage(render_ctx->getDevice(), image_, nullptr);
	}

	has_been_initialised_ = false;
	has_been_allocated_ = false;
}

void VKE::InternalTexture::resetSampler(RenderContext* render_ctx) {
	if (has_created_sample_) {
		vkDestroySampler(render_ctx->getDevice(), default_sampler_, nullptr);
		has_created_sample_ = false;
	}
}

void VKE::InternalTexture::init(RenderContext* render_ctx, std::string tex_name, VkImageType type, VkFormat format) {

	render_ctx_ = render_ctx;
	tex_name_ = tex_name;
	type_ = type;
	format_ = format;

	int channels;
	stbi_uc* pixels = stbi_load(tex_name.data(), &width_, &height_, &channels, STBI_rgb_alpha);

	if (!pixels) {
		throw std::runtime_error("failed to load texture!");
	}

	load(pixels);

}

void VKE::InternalTexture::init(RenderContext* render_ctx, void* data, uint32 width, uint32 height, VkImageType type, VkFormat format) {
	
	render_ctx_ = render_ctx;
	tex_name_ = "";
	type_ = type;
	format_ = format;

	width_ = width;
	height_ = height;

	uchar8* pixels = (uchar8*)data;
	load(pixels);

}

void VKE::InternalTexture::load(uchar8* pixels) {

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = type_;
	imageInfo.extent.width = width_;
	imageInfo.extent.height = height_;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format_;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (format_ == VK_FORMAT_R8G8B8A8_SRGB)
		imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	else
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;


	if (vkCreateImage(render_ctx_->getDevice(), &imageInfo, nullptr, &image_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	has_been_initialised_ = true;


	VkMemoryRequirements memRequirements = {};
	vkGetImageMemoryRequirements(render_ctx_->getDevice(), image_, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = render_ctx_->findMemoryType(
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(render_ctx_->getDevice(), &allocInfo, nullptr, &image_memory_) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(render_ctx_->getDevice(), image_, image_memory_, 0);

	if (pixels != nullptr) {
		render_ctx_->uploadToStaging(pixels, width_ * height_ * 4);
		render_ctx_->transitionImageLayout(image_, format_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		render_ctx_->copyBufferToImage(render_ctx_->getStagingBuffer(), *this, height_, width_);
		render_ctx_->transitionImageLayout(image_, format_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		render_ctx_->clearStaging();
	}


	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image_;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.pNext = nullptr;
	viewInfo.format = format_;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if(format_ == VK_FORMAT_R8G8B8A8_SRGB)viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	else viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	if (vkCreateImageView(render_ctx_->getDevice(), &viewInfo, nullptr, &image_view_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	has_been_allocated_ = true;

	if (!has_created_sample_) {
		//Default Sampler, will be able to customize with a secondary func
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(render_ctx_->getDevice(), &samplerInfo, nullptr, &VKE::InternalTexture::default_sampler_) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create default sampler!");
		}

		has_created_sample_ = true;
	}
}