#include "internal_buffer.h"
#include "render_context.h"

VKE::InternalBuffer::InternalBuffer(){
	
	has_been_initialised_ = false;
	has_been_allocated_ = false;

	render_ctx_ = nullptr;

	buffer_ = VK_NULL_HANDLE;
	buffer_memory_ = VK_NULL_HANDLE;

	vertex_count_ = 0;
}

VKE::InternalBuffer::~InternalBuffer() {
	//reset(render_ctx_);
}

void VKE::InternalBuffer::init(RenderContext* render_ctx, uint32 vertex_count, VkBufferUsageFlags usage) {

	if (render_ctx == nullptr) return; //ERROR
	if (vertex_count == 0);	//WARN ???

	render_ctx_ = render_ctx;
	vertex_count_ = vertex_count;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(VKE::Vertex) * vertex_count_;
	bufferInfo.usage = usage; // VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	if (vkCreateBuffer(render_ctx_->getDevice(), &bufferInfo, nullptr, &buffer_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	has_been_initialised_ = true;
}

void VKE::InternalBuffer::uploadData(std::vector<VKE::Vertex> vertex_data, VkMemoryPropertyFlags mem_properties) {

	if (render_ctx_->getDevice() == VK_NULL_HANDLE) return; //ERROR
	if (vertex_data.size() > vertex_count_) return; //ERROR
	if (vertex_data.size() < vertex_count_); //WARN

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(render_ctx_->getDevice(), buffer_, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = render_ctx_->findMemoryType(
		memRequirements.memoryTypeBits,
		mem_properties); 	//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT



	if (vkAllocateMemory(render_ctx_->getDevice(), &allocInfo, nullptr, &buffer_memory_) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(render_ctx_->getDevice(), buffer_, buffer_memory_, 0);

	//may not be immediate, we sacrifice efficiency but assure it to be synced by adding VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	void* data = nullptr;
	vkMapMemory(render_ctx_->getDevice(), buffer_memory_, 0, vertex_count_ * sizeof(VKE::Vertex)/*VK_WHOLE_MEMORY*/, 0, &data);
	memcpy(data, vertex_data.data(), (size_t)(vertex_count_ * sizeof(VKE::Vertex) ));
	vkUnmapMemory(render_ctx_->getDevice(), buffer_memory_);

	has_been_allocated_ = true;
}

void VKE::InternalBuffer::reset(RenderContext* render_ctx) {

	if (has_been_allocated_) {
		vkFreeMemory(render_ctx_->getDevice(), buffer_memory_, nullptr);
		has_been_allocated_ = false;
	}

	if (has_been_initialised_) {
		vkDestroyBuffer(render_ctx_->getDevice(), buffer_, nullptr);
		has_been_initialised_ = false;
	}
}