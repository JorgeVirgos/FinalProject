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
		InternalMaterial& getInternalRsc(RenderContext*);


	private:
		Material(uint32 id) { id_ = id; }
		friend class RenderContext;
	};

	class InternalMaterial : BaseResource {
	public:
		InternalMaterial();
		~InternalMaterial();

		void reset(RenderContext* render_ctx) override;

		void init(RenderContext* render_ctx, MaterialInfo mat_info);
		void recreatePipeline(MaterialInfo* mat_info = nullptr);
		void UpdateDynamicStates(float dynamic_line_width_, VkViewport dynamic_viewport_, VkRect2D dynamic_scissors);

		void UpdateTextures(Texture texture);
		void UpdateTextures(InternalTexture& texture);

	private:

		MaterialInfo mat_info_;

		VkPipelineLayout pipeline_layout_;
		VkPipeline graphical_pipeline_;

		VkGraphicsPipelineCreateInfo pipeline_creation_info_;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssembly;
		VkPipelineViewportStateCreateInfo viewportState;
		VkPipelineRasterizationStateCreateInfo rasterizer;
		VkPipelineMultisampleStateCreateInfo multisampling;
		VkPipelineDepthStencilStateCreateInfo depthStencil;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlending;
		VkPipelineDynamicStateCreateInfo dynamicState;
		VkDynamicState dynamicStates[3] = {
			VK_DYNAMIC_STATE_LINE_WIDTH,
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
		};

		VkVertexInputBindingDescription bindingDescription;
		std::array<VkVertexInputAttributeDescription, 3> AttributeDescriptions;

		VkDescriptorSet textures_desc_set_;
		VkDescriptorSet light_desc_set_;
		Texture albedo_texture_;

		friend class RenderContext;
		friend class Manager<RenderComponent>;

	};


}
#endif //__INTERNAL_MATERIAL_H__