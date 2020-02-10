#ifndef __INTERNAL_BUFFER_H__
#define __INTERNAL_BUFFER_H__ 1

#include "platform_types.h"
#include "constants.h"
#include <vector>

namespace VKE {

	class RenderContext;

	//Dispatching a Buffer int handle corresponding to vector of InternalBuffer
	struct Buffer {
		uint32 id_;
	};

	class InternalBuffer {
	public:
		InternalBuffer();
		~InternalBuffer();

		void init(RenderContext* render_ctx, uint32 vertex_count, VkBufferUsageFlags usage);
		void uploadData(std::vector<VKE::Vertex> vertex_data, VkMemoryPropertyFlags mem_properties);

		void reset(RenderContext* render_ctx);

		VkBuffer getBufferHandle() { return buffer_; }
		uint32 getVertexCount() { return vertex_count_; }

		bool isInitialised() { return has_been_initialised_; }
		bool isAllocated() { return has_been_allocated_; }

	private:

		VKE::RenderContext* render_ctx_;

		VkBuffer buffer_;
		bool has_been_initialised_;

		VkDeviceMemory buffer_memory_;
		bool has_been_allocated_;

		uint32 vertex_count_;
	};
}

#endif //__INTERNAL_BUFFER_H__