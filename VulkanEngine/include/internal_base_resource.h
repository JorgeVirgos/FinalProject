#ifndef __INTERNAL_BASE_RESOURCE_H__
#define __INTERNAL_BASE_RESOURCE_H__ 1

#include "platform_types.h"

namespace VKE {

	class RenderContext;

	struct BaseHandle {
		uint32 id() { return id_; }
	protected:
		uint32 id_;
		BaseHandle() { id_ = UINT32_MAX; }
		~BaseHandle() {};
		BaseHandle(uint32 id) { id_ = id; };
	};

	class BaseResource {
	public:
		BaseResource() {};
		~BaseResource() {};

		virtual void reset(RenderContext* render_ctx) { in_use_ = false; render_ctx_ = render_ctx; };
		bool isInUse() { return in_use_; }
		bool isInitialised() { return has_been_initialised_; }
		bool isAllocated() { return has_been_allocated_; }

	protected:

		VKE::RenderContext* render_ctx_;

		bool in_use_ = false;
		bool has_been_initialised_ = false;
		bool has_been_allocated_ = false;

		// TO BE REPLACED WITH MANAGER
		friend class RenderContext;

	};

}


#endif //__INTERNAL_BASE_RESOURCE_H__