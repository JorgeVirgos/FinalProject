#include "internal_material.h"
#include "render_context.h"

VKE::InternalMaterial::InternalMaterial() {
	pipeline_layout_ = VK_NULL_HANDLE;
	graphical_pipeline_ = VK_NULL_HANDLE;
	textures_desc_set_ = VK_NULL_HANDLE;

	mat_info_ = VKE::MaterialInfo();

}

VKE::InternalMaterial::~InternalMaterial() {
	
}

void VKE::InternalMaterial::reset(RenderContext* render_ctx) {

	if (has_been_initialised_) {
		vkDestroyPipeline(render_ctx->getDevice(), graphical_pipeline_, nullptr);
		vkDestroyPipelineLayout(render_ctx->getDevice(), pipeline_layout_, nullptr);
	}

	albedo_texture_ = VKE::Texture();
	pipeline_layout_ = VK_NULL_HANDLE;
	graphical_pipeline_ = VK_NULL_HANDLE;
	textures_desc_set_ = VK_NULL_HANDLE;

	mat_info_ = VKE::MaterialInfo();
	has_been_initialised_ = false;
	in_use_ = false;
}

void VKE::InternalMaterial::init(RenderContext* render_ctx, std::string vert_name, std::string frag_name, MaterialInfo mat_info) {
	
	mat_info_ = mat_info;
	render_ctx_ = render_ctx;
	vert_name_ = "./../../resources/shaders/" + (vert_name == "" ? "test_vert.spv" : vert_name);
	frag_name_ = "./../../resources/shaders/" + (frag_name == "" ? "test_frag.spv" : frag_name);

	if (mat_info_.dynamic_viewport_.width == 0 && mat_info_.dynamic_viewport_.height == 0) {
		mat_info_.dynamic_viewport_.x = 0.0f;
		mat_info_.dynamic_viewport_.y = 0.0f;
		mat_info_.dynamic_viewport_.width = (float32)render_ctx_->getSwapChainExtent().width;
		mat_info_.dynamic_viewport_.height = (float32)render_ctx_->getSwapChainExtent().height;
		mat_info_.dynamic_viewport_.minDepth = 0.0f;
		mat_info_.dynamic_viewport_.maxDepth = 1.0f;
	}

	if (mat_info_.dynamic_scissors_.extent.height == 0 && mat_info_.dynamic_scissors_.extent.height == 0) {
		mat_info_.dynamic_scissors_.offset = { 0, 0 };
		mat_info_.dynamic_scissors_.extent = render_ctx_->getSwapChainExtent();
	}

	if (mat_info_.subpass_num_ > 1) mat_info_.subpass_num_ = 0;

	
	auto vertShaderCode = readFile(vert_name_);
	auto fragShaderCode = readFile(frag_name_);

	VkShaderModule vertShaderModule = render_ctx_->createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = render_ctx_->createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


	// GEOMETRY
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = VKE::Vertex::getBindingDescription();
	auto AttributeDescriptions = VKE::Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32>(AttributeDescriptions.size());

	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
	vertexInputInfo.pVertexAttributeDescriptions = AttributeDescriptions.data(); // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;


	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &mat_info_.dynamic_viewport_;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &mat_info_.dynamic_scissors_;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = mat_info_.cull_mode_;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional

	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	// Usual treatment for color blending
	//colorBlendAttachment.blendEnable = VK_TRUE;
	//colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	//colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	//colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	//colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkDynamicState dynamicStates[] = {
	VK_DYNAMIC_STATE_LINE_WIDTH,
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 3;
	dynamicState.pDynamicStates = dynamicStates;

	std::vector<VkDescriptorSetLayout> desc_layouts = render_ctx_->getDescSetLayouts();
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32>(desc_layouts.size());
	pipelineLayoutInfo.pSetLayouts = desc_layouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(render_ctx_->getDevice(), &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS) {
		std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState; // Optional

	pipelineInfo.layout = pipeline_layout_;
	pipelineInfo.subpass = mat_info_.subpass_num_;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if(vert_name == "quadscreen_vert.spv")
		pipelineInfo.renderPass = render_ctx_->getQuadRenderPass();
	else
		pipelineInfo.renderPass = render_ctx_->getRenderPass();

	if (vkCreateGraphicsPipelines(render_ctx_->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphical_pipeline_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(render_ctx_->getDevice(), vertShaderModule, nullptr);
	vkDestroyShaderModule(render_ctx_->getDevice(), fragShaderModule, nullptr);

	VkDescriptorSetAllocateInfo alloc_info = render_ctx_->getDescriptorAllocationInfo(VKE::DescriptorType_Textures);
	vkAllocateDescriptorSets(render_ctx_->getDevice(), &alloc_info, &textures_desc_set_);

	has_been_initialised_ = true;
	has_been_allocated_ = true;

	albedo_texture_ = render_ctx_->getDefaultTexture();
	VKE::InternalTexture& default_texture = render_ctx_->getDefaultInternalTexture();
	render_ctx_->UpdateDescriptor(textures_desc_set_, DescriptorType_Textures, (void*)&default_texture);

}

void VKE::InternalMaterial::UpdateDynamicStates(float dynamic_line_width, VkViewport dynamic_viewport, VkRect2D dynamic_scissors) {
	mat_info_.dynamic_line_width_ = dynamic_line_width;
	mat_info_.dynamic_viewport_ = dynamic_viewport;
	mat_info_.dynamic_scissors_ = dynamic_scissors;
}

void VKE::InternalMaterial::UpdateTextures(Texture texture) {
	albedo_texture_ = texture;
	VKE::InternalTexture& tex = render_ctx_->getInternalRsc<VKE::Texture::internal_class>(albedo_texture_);
	render_ctx_->UpdateDescriptor(textures_desc_set_, DescriptorType_Textures, (void*)&tex);
}

void VKE::InternalMaterial::UpdateTextures(InternalTexture& texture) {
	render_ctx_->UpdateDescriptor(textures_desc_set_, DescriptorType_Textures, (void*)&texture);
}
