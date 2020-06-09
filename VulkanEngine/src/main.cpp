#include <window.h>
#include "platform_types.h"
#include "constants.h"
#include "hierarchy_context.h"
#include "render_context.h"
#include "geometry_primitives.h"
#include <functional>
#include <imgui/imgui.h>


int main(){

	VKE::Window	window;
	VKE::RenderContext render_ctx;
	VKE::HierarchyContext hier_ctx(&render_ctx);

	ImGui::CreateContext();
	window.initWindow(1920, 1080, &hier_ctx);
	render_ctx.init(&window, &hier_ctx);

	render_ctx.getCamera().calculateStaticMatrices(70.0f, window.width(), window.height(), 0.1f, 100.0f);
	render_ctx.getCamera().changeSpeed(10.0f, 50.0f, 0.10f);


	VKE::Entity& base_node = hier_ctx.getEntity("Heighmap");
	auto trans = base_node.AddComponent<VKE::TransformComponent>(&hier_ctx);
	auto rend = base_node.AddComponent<VKE::RenderComponent>(&hier_ctx);

	VKE::GeoPrimitives::Heighmap(&render_ctx, rend, 200,200, 2.0);

	VKE::Texture tex = render_ctx.getResource<VKE::Texture>();
	VKE::InternalTexture& texture = tex.getInternalRsc(&render_ctx);
	texture.init(&render_ctx, "./../../resources/textures/default_placeholder.jpg", VKE::TextureType_Sampler);

	VKE::InternalMaterial& mat = rend->getMaterial();
	VKE::MaterialInfo quad_mat_info_;
	//quad_mat_info_.cull_mode_ = VK_CULL_MODE_NONE;
	quad_mat_info_.vert_shader_name_ = "gerstner_vert.spv";
	quad_mat_info_.frag_shader_name_ = "gerstner_frag.spv";
	mat.init(&render_ctx, quad_mat_info_);
	mat.UpdateTextures(tex);


	VKE::Entity& cube_node = hier_ctx.getEntity("Sphere");
	auto cube_trans = cube_node.AddComponent<VKE::TransformComponent>(&hier_ctx, *trans);
	auto cube_rend = cube_node.AddComponent<VKE::RenderComponent>(&hier_ctx);

	VKE::Entity& cube2_node = hier_ctx.getEntity("Cube");
	auto cube2_trans = cube2_node.AddComponent<VKE::TransformComponent>(&hier_ctx);
	auto cube2_rend = cube2_node.AddComponent<VKE::RenderComponent>(&hier_ctx);

	VKE::GeoPrimitives::Cube(&render_ctx, cube2_rend);
	cube2_trans->setPosition(-10.0f, 0.0f, 0.0f);


	VKE::InternalMaterial& cube_mat = cube_rend->getMaterial();
	VKE::InternalMaterial& cube2_mat = cube2_rend->getMaterial();
	VKE::MaterialInfo cube_mat_info;
	cube_mat_info.vert_shader_name_ = "test_vert.spv";
	cube_mat_info.frag_shader_name_ = "test_frag.spv";
	cube_mat.init(&render_ctx, cube_mat_info);
	cube_mat.UpdateTextures(tex);
	cube2_mat.init(&render_ctx, cube_mat_info);
	cube2_mat.UpdateTextures(tex);

	VKE::GeoPrimitives::Sphere(&render_ctx, cube_rend);
	cube_trans->setPosition(2.0f, 0.0f, -2.0f);
	cube_trans->setRotation(0.0f, 0.0f, 0.0f);

	std::vector<int32> widths;
	std::vector<int32> heights;
	std::vector<uchar8*> cubemap_data;

	std::vector<std::string> cubemap_tex_names = {
		"./../../resources/textures/Cubemaps/Forest/posx.jpg",
		"./../../resources/textures/Cubemaps/Forest/negx.jpg",
		"./../../resources/textures/Cubemaps/Forest/posy.jpg",
		"./../../resources/textures/Cubemaps/Forest/negy.jpg",
		"./../../resources/textures/Cubemaps/Forest/posz.jpg",
		"./../../resources/textures/Cubemaps/Forest/negz.jpg",
	};

	VKE::InternalTexture::loadImageToMemory(cubemap_tex_names, cubemap_data, widths, heights, true);
	VKE::Texture cubemap_tex = render_ctx.getResource<VKE::Texture>();
	cubemap_tex.getInternalRsc(&render_ctx).init(&render_ctx, cubemap_data[0], widths[0], heights[0], VKE::TextureType_Cubemap);
	
	
	VKE::Entity& cubemap_node = hier_ctx.getEntity("Skybox");
	auto cubemap_trans = cubemap_node.AddComponent<VKE::TransformComponent>(&hier_ctx, *trans);
	auto cubemap_rend = cubemap_node.AddComponent<VKE::RenderComponent>(&hier_ctx);
	
	VKE::GeoPrimitives::Cube(&render_ctx, cubemap_rend);

	VKE::MaterialInfo mat_info;
	mat_info.cull_mode_ = VK_CULL_MODE_FRONT_BIT;
	mat_info.vert_shader_name_ = "cubemap_vert.spv";
	mat_info.frag_shader_name_ = "cubemap_frag.spv";

	VKE::InternalMaterial& cubemap_mat = cubemap_rend->getMaterial();
	cubemap_mat.init(&render_ctx,  mat_info);
	cubemap_mat.UpdateTextures(cubemap_tex);

	cube2_trans->setLambda(
		[](VKE::HierarchyContext* hier_ctx, VKE::TransformComponent& trans) {
			float64 time = hier_ctx->getRenderContext()->getWindow()->getTimeSinceStartup();
			trans.setRotation(time/6.28, time/3.14, time);
	});
	trans->setPosition(0.0f, -10.0f, 0.0f);
	cube_trans->setLambda(
		[](VKE::HierarchyContext* hier_ctx, VKE::TransformComponent& trans) {
		float64 time = hier_ctx->getRenderContext()->getWindow()->getTimeSinceStartup();
		trans.setRotation(0.0, time * 2.0, 0.0);
	});


	while (!window.ShouldWindowClose()) {

		window.PollEvents();
		render_ctx.getCamera().keyboardInput(&window);
		render_ctx.getCamera().mouseInput(&window, false);

		window.Update();
		render_ctx.getCamera().logic(window.getDeltaTime());

		render_ctx.updateLightData();
		hier_ctx.UpdateManagers();

		window.UI(&render_ctx, &hier_ctx);
		render_ctx.draw();
	}


	return 0;
}