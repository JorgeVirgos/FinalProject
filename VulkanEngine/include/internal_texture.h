#ifndef __INTERNAL_TEXTURE_H__
#define __INTERNAL_TEXTURE_H__ 1

#include "internal_base_resource.h"
#include "platform_types.h"
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

		static void loadImageToMemory(std::vector<std::string> tex_names, std::vector<uchar8*>& tex_data, std::vector<int32>& widths, std::vector<int32>& heights, bool adjacent_memory = false);
		static void resetSampler(RenderContext* render_ctx);
		static VkSampler default_sampler_;

		void reset(RenderContext* render_ctx) override;

		void init(RenderContext* render_ctx, std::string tex_name, TextureType tex_type);
		void init(RenderContext* render_ctx, void* data, uint32 width, uint32 height, TextureType tex_type);


	private:

		void load(uchar8* pixels);

		static bool has_created_defailt_sample_;
		static VkFormat depth_format_;

		TextureType tex_type_;
		VkImageType type_;
		VkFormat format_;
		
		std::string tex_name_;

		int32 width_;
		int32 height_;

		VkImage image_;
		VkImageView image_view_;
		VkDeviceMemory image_memory_;
		VkSampler custom_sampler_;

		bool has_created_custom_sampler_;

		friend class RenderContext;
		friend class InternalMaterial;

	};

}

#endif //__INTERNAL_TEXTURE_H__