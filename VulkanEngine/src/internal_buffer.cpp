#include "internal_buffer.h"
#include "render_context.h"

VKE::InternalBuffer& VKE::Buffer::getInternalRsc(VKE::RenderContext* render_ctx) {
	return render_ctx->getInternalRsc<VKE::Buffer::internal_class>(*this);
}

VKE::InternalBuffer::InternalBuffer(){
	
	has_been_initialised_ = false;
	has_been_allocated_ = false;

	render_ctx_ = nullptr;

	buffer_ = VK_NULL_HANDLE;
	buffer_memory_ = VK_NULL_HANDLE;
	uniform_desc_set_ = VK_NULL_HANDLE;

	element_count_ = 0;
	element_size_ = 0;
	buffer_type_ = BufferType_MAX;
}

VKE::InternalBuffer::~InternalBuffer() {

	// Already done in the cleanup phase of the RenderContext destruction

	//if (has_been_allocated_) {
	//	vkFreeMemory(render_ctx_->getDevice(), buffer_memory_, nullptr);
	//}

	//if (has_been_initialised_) {
	//	vkDestroyBuffer(render_ctx_->getDevice(), buffer_, nullptr);
	//}
}

void VKE::InternalBuffer::reset(RenderContext* render_ctx) {

	if (has_been_allocated_) {
		vkFreeMemory(render_ctx->getDevice(), buffer_memory_, nullptr);
		has_been_allocated_ = false;
	}

	if (has_been_initialised_) {
		vkDestroyBuffer(render_ctx->getDevice(), buffer_, nullptr);
		has_been_initialised_ = false;
	}

	in_use_ = false;
	render_ctx_ = nullptr;
	buffer_ = VK_NULL_HANDLE;
	buffer_memory_ = VK_NULL_HANDLE;
	uniform_desc_set_ = VK_NULL_HANDLE;
	element_count_ = 0;
	element_size_ = 0;
	buffer_type_ = BufferType_MAX;
	mem_properties_ = VkMemoryPropertyFlags();

	has_been_initialised_ = false;
	has_been_allocated_ = false;
}

void VKE::InternalBuffer::init(RenderContext* render_ctx, uint32 element_size, size_t element_count, BufferType buffer_type) {

	if (render_ctx == nullptr) return; //ERROR
	if (element_size == 0);	//WARN ???
	if (element_count == 0);	//WARN ???

	render_ctx_ = render_ctx;
	element_size_ = element_size;
	element_count_ = static_cast<uint32>(element_count);
	buffer_type_ = buffer_type;

	VkBufferUsageFlags buffer_usage;

	switch (buffer_type_) {
	case(VKE::BufferType_Staging):
		mem_properties_ = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		break;
	case(VKE::BufferType_Vertex):
		mem_properties_ = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		break;
	case(VKE::BufferType_Index):
		mem_properties_ = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		buffer_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		break;
	case(VKE::BufferType_Uniform):
	case(VKE::BufferType_ExternalUniform):
		mem_properties_ = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		buffer_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		break;
	default:
		throw std::runtime_error("VKE::InternalBuffer::init - Invalid / Unsupported BufferType");
	}

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = element_size_ * element_count_;
	bufferInfo.usage = buffer_usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	if (vkCreateBuffer(render_ctx_->getDevice(), &bufferInfo, nullptr, &buffer_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}



	has_been_initialised_ = true;
}

void VKE::InternalBuffer::uploadData(void* data) {

	if (render_ctx_->getDevice() == VK_NULL_HANDLE) return; //ERROR
	if (!has_been_initialised_) return; //ERROR

	if (!has_been_allocated_) {

		VkMemoryRequirements memRequirements = {};
		vkGetBufferMemoryRequirements(render_ctx_->getDevice(), buffer_, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = render_ctx_->findMemoryType(
			memRequirements.memoryTypeBits,
			mem_properties_);

		if (vkAllocateMemory(render_ctx_->getDevice(), &allocInfo, nullptr, &buffer_memory_) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(render_ctx_->getDevice(), buffer_, buffer_memory_, 0);

		if (buffer_type_ == VKE::BufferType_Uniform) {
			VkDescriptorSetAllocateInfo alloc_info = render_ctx_->getDescriptorAllocationInfo(VKE::DescriptorType_Matrices);
			vkAllocateDescriptorSets(render_ctx_->getDevice(), &alloc_info, &uniform_desc_set_);
			render_ctx_->UpdateDescriptor(uniform_desc_set_, VKE::DescriptorType_Matrices, (void*)&buffer_);
		}

		has_been_allocated_ = true;

	}

	if (data == nullptr || buffer_type_ == BufferType_Staging) return; // Data to be uploaded later 

	if (buffer_type_ == BufferType_Uniform || buffer_type_ == BufferType_ExternalUniform) {
		void* mapped_data = nullptr;
		vkMapMemory(render_ctx_->getDevice(), buffer_memory_, 0, VK_WHOLE_SIZE, 0, &mapped_data);
		memcpy(mapped_data, data, element_size_ * element_count_);
		vkUnmapMemory(render_ctx_->getDevice(), buffer_memory_);

	} else {
		render_ctx_->uploadToStaging(data, element_size_ * element_count_);
		render_ctx_->copyBuffer(render_ctx_->getStagingBuffer(), *this, element_count_ * element_size_);
		render_ctx_->clearStaging();
	}

}