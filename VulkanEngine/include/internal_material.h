#ifndef __INTERNAL_MATERIAL_H__
#define __INTERNAL_MATERIAL_H__ 1

#include "internal_base_resource.h"
#include "constants.h"
#include "internal_texture.h"
#include <vector>

namespace VKE {

	class RenderContext;
	class InternalMaterial;
	struct Texture;

	template<class Component> class Manager;
	class RenderComponent;

	struct Material : public BaseHandle {
		Material() {};
		~Material() {};
		typedef InternalMaterial internal_class;

	private:
		Material(uint32 id) { id_ = id; }
		friend class RenderContext;
	};

	class InternalMaterial : BaseResource {
	public:
		InternalMaterial();
		~InternalMaterial();

		void reset(RenderContext* render_ctx) override;

		void init(RenderContext* render_ctx, float dynamic_line_width = 1.0f, VkViewport dynamic_viewport = {}, VkRect2D dynamic_scissors = {});
		void UpdateDynamicStates(float dynamic_line_width_, VkViewport dynamic_viewport_, VkRect2D dynamic_scissors);

		void UpdateTextures(Texture texture);

	private:

		float dynamic_line_width_;
		VkViewport dynamic_viewport_;
		VkRect2D dynamic_scissors_;

		VkPipelineLayout pipeline_layout_;
		VkPipeline graphical_pipeline_;

		VkDescriptorSet textures_desc_set_;
		Texture albedo_texture_;

		friend class RenderContext;
		friend class Manager<RenderComponent>;

	};


}
#endif //__INTERNAL_MATERIAL_H__