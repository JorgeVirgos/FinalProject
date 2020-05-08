#ifndef __INTERNAL_BUFFER_H__
#define __INTERNAL_BUFFER_H__ 1

#include "internal_base_resource.h"
#include "constants.h"
#include <vector>

namespace VKE {

	class RenderContext;
	class InternalBuffer;


	//Dispatching a Buffer int handle corresponding to vector of InternalBuffer
	struct Buffer : public BaseHandle {
		Buffer() {};
		~Buffer() {};

		typedef InternalBuffer internal_class;
		InternalBuffer& getInternalRsc(RenderContext*);

	private:
		Buffer(uint32 id) { id_ = id; }
		friend class RenderContext;
	};


	class InternalBuffer : public BaseResource {
	public:
		InternalBuffer();
		~InternalBuffer();

		void reset(RenderContext* render_ctx) override;

		void init(RenderContext* render_ctx, uint32 element_size, size_t element_count, BufferType buffer_type);
		void uploadData(void* data);

		//VkBuffer getBufferHandle() { return buffer_; }
		uint32 getElementCount() { return element_count_; }
		BufferType getBufferType() { return buffer_type_; }

	private:

		BufferType buffer_type_;

		uint32 element_count_;
		uint32 element_size_;

		VkBuffer buffer_;
		VkWriteDescriptorSet buffer_descriptor_;
		VkDeviceMemory buffer_memory_;
		VkMemoryPropertyFlags mem_properties_;
		VkDescriptorSet uniform_desc_set_;

		friend class RenderContext;

	};
}

#endif //__INTERNAL_BUFFER_H__