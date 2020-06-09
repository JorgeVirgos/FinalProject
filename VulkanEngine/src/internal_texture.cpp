#include "internal_texture.h"
#include "render_context.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

VkSampler VKE::InternalTexture::default_sampler_ = VK_NULL_HANDLE;
bool VKE::InternalTexture::has_created_defailt_sample_ = false;
VkFormat VKE::InternalTexture::depth_format_ = VK_FORMAT_D32_SFLOAT;

VKE::InternalTexture& VKE::Texture::getInternalRsc(VKE::RenderContext* render_ctx) {
	return render_ctx->getInternalRsc<VKE::Texture::internal_class>(*this);
}

VKE::InternalTexture::InternalTexture() {
	width_ = 0;
	height_ = 0;

	image_ = VK_NULL_HANDLE;
	image_view_ = VK_NULL_HANDLE;
	image_memory_ = VK_NULL_HANDLE;

	has_been_initialised_ = false;
	has_been_allocated_ = false;
	has_created_custom_sampler_ = false;
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

	if (has_created_custom_sampler_) {
		vkDestroySampler(render_ctx->getDevice(), custom_sampler_, nullptr);
	}

	has_been_initialised_ = false;
	has_been_allocated_ = false;
	has_created_custom_sampler_ = false;
}

void VKE::InternalTexture::loadImageToMemory(std::vector<std::string> tex_names, std::vector<uchar8*>& tex_data, std::vector<int32>& widths, std::vector<int32>& heights, bool adjacent_memory){
	
	tex_data.clear();
	widths.clear();
	heights.clear();

	uint64 total_size = 0; 

	for (auto tex_name : tex_names) {
		int32 channels, width, height;
		uchar8* pixels = stbi_load(tex_name.data(), &width, &height, &channels, STBI_rgb_alpha);
		if (!pixels) throw std::runtime_error("failed to load texture on loadImageToMemory");

		widths.push_back(width);
		heights.push_back(height);
		tex_data.push_back(pixels);
		if (adjacent_memory) total_size += sizeof(uchar8) * width * height * 4;
	}

	if (adjacent_memory) {
		uchar8* adjacent_memory_block = (uchar8*)malloc(total_size);
		uint64 current_offset = 0;

		for (uint32 i = 0; i < tex_data.size(); ++i) {
			uint64 current_image_size = sizeof(uchar8) * widths[i] * heights[i] * 4;
			memcpy_s(adjacent_memory_block + current_offset, total_size, tex_data[i], current_image_size);
			current_offset += current_image_size;
			delete tex_data[i];
		}

		tex_data.clear();
		tex_data.push_back(adjacent_memory_block);
	}
}

void VKE::InternalTexture::resetSampler(RenderContext* render_ctx) {
	if (has_created_defailt_sample_) {
		vkDestroySampler(render_ctx->getDevice(), default_sampler_, nullptr);
		has_created_defailt_sample_ = false;
	}
}

void VKE::InternalTexture::init(RenderContext* render_ctx, std::string tex_name, TextureType tex_type) {

	render_ctx_ = render_ctx;
	tex_name_ = tex_name;
	tex_type_ = tex_type;

	int channels;
	stbi_uc* pixels = stbi_load(tex_name.data(), &width_, &height_, &channels, STBI_rgb_alpha);

	if (!pixels) {
		throw std::runtime_error("failed to load texture!");
	}

	load(pixels);
	delete pixels;

}

void VKE::InternalTexture::init(RenderContext* render_ctx, void* data, uint32 width, uint32 height, TextureType tex_type) {
	
	render_ctx_ = render_ctx;
	tex_name_ = "";
	tex_type_ = tex_type;

	width_ = width;
	height_ = height;

	uchar8* pixels = (uchar8*)data;
	load(pixels);
	delete pixels;
}

void VKE::InternalTexture::load(uchar8* pixels) {

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.extent.width = width_;
	imageInfo.extent.height = height_;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	switch (tex_type_) {
	case TextureType_Sampler:
		type_ = VK_IMAGE_TYPE_2D;
		format_ = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		break;
	case TextureType_ColorAttachment:
		type_ = VK_IMAGE_TYPE_2D;
		format_ = VK_FORMAT_R32G32B32A32_SFLOAT;// render_ctx_->getCurrentSwapChainTexture().format_;
		imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		break;
	case TextureType_DepthAttachment:
		type_ = VK_IMAGE_TYPE_2D;
		format_ = depth_format_;
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		break;
	case TextureType_Cubemap:
		type_ = VK_IMAGE_TYPE_2D;
		format_ = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		imageInfo.arrayLayers = 6;
		break;
	default:
		throw std::runtime_error("Unsupported texture type on InternalTexture::load()");
		break;
	};

	imageInfo.imageType = type_;
	imageInfo.format = format_;


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

		render_ctx_->uploadToStaging(pixels, width_ * height_ * 4 * imageInfo.arrayLayers);
		render_ctx_->transitionImageLayout(this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		render_ctx_->copyBufferToImage(render_ctx_->getStagingBuffer(), *this, height_, width_);
		render_ctx_->transitionImageLayout(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		render_ctx_->clearStaging();
	}


	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image_;
	viewInfo.pNext = nullptr;
	viewInfo.format = format_;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if(tex_type_ == TextureType_Cubemap)
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	else 
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	if(format_ == depth_format_) 
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	else 
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	if (vkCreateImageView(render_ctx_->getDevice(), &viewInfo, nullptr, &image_view_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	has_been_allocated_ = true;

	if (tex_type_ == TextureType_Cubemap) {

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = samplerInfo.addressModeU;
		samplerInfo.addressModeW = samplerInfo.addressModeU;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.maxAnisotropy = 8;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		if (vkCreateSampler(render_ctx_->getDevice(), &samplerInfo, nullptr, &custom_sampler_) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create default sampler!");
		}

		has_created_custom_sampler_ = true;
	}

	if (!has_created_defailt_sample_) {
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

		has_created_defailt_sample_ = true;
	}
}