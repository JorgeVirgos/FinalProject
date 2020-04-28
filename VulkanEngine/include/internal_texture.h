#ifndef __INTERNAL_TEXTURE_H__
#define __INTERNAL_TEXTURE_H__ 1

#include "internal_base_resource.h"
#include "constants.h"
#include <string.h>

namespace VKE {

	class InternalTexture;

	struct Texture : BaseHandle {
		Texture() {};
		~Texture() {};
		typedef InternalTexture internal_class;

	private:
		Texture(uint32 id) { id_ = id; }
		friend class RenderContext;
	};

	class InternalTexture : public BaseResource {
	public:
		InternalTexture();
		~InternalTexture();

		void reset(RenderContext* render_ctx) override;

		void init(RenderContext* render_ctx, std::string tex_name, VkImageType type, VkFormat format);
		void init(RenderContext* render_ctx, void* data, uint32 width, uint32 height, VkImageType type, VkFormat format);

		static void resetSampler(RenderContext* render_ctx);
		static VkSampler default_sampler_;

	private:

		void load(uchar8* pixels);

		static bool has_created_sample_;

		VkImageType type_;
		VkFormat format_;
		
		std::string tex_name_;

		int32 width_;
		int32 height_;

		VkImage image_;
		VkImageView image_view_;
		VkDeviceMemory image_memory_;

		friend class RenderContext;
		friend class InternalMaterial;

	};

}

#endif //__INTERNAL_TEXTURE_H__