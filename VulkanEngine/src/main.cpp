#include <window.h>
#include "platform_types.h"
#include "constants.h"
#include "hierarchy_context.h"
#include "render_context.h"
#include "geometry_primitives.h"
#include <chrono>



int main(){

	VKE::Window	window;
	VKE::RenderContext render_ctx;
	VKE::HierarchyContext hier_ctx(&render_ctx);

	window.initWindow(720, 640, &hier_ctx);
	render_ctx.init(&window, &hier_ctx);

	render_ctx.getCamera().calculateStaticMatrices(70.0f, 720, 640, 0.1f, 100.0f);
	render_ctx.getCamera().changeSpeed(10.0f, 50.0f, 0.10f);


	VKE::Entity& base_node = hier_ctx.getEntity();
	auto trans = base_node.AddComponent<VKE::TransformComponent>(&hier_ctx);
	auto rend = base_node.AddComponent<VKE::RenderComponent>(&hier_ctx);

	VKE::GeoPrimitives::Quad(&render_ctx, rend);

	VKE::Texture tex = render_ctx.getResource<VKE::Texture>();
	VKE::InternalTexture& texture = render_ctx.getInternalRsc<VKE::Texture::internal_class>(tex);
	texture.init(&render_ctx, "./../../resources/textures/default_placeholder.jpg", VKE::TextureType_Sampler);

	VKE::InternalMaterial& mat = rend->getMaterial();
	VKE::MaterialInfo quad_mat_info_;
	quad_mat_info_.cull_mode_ = VK_CULL_MODE_NONE;
	mat.init(&render_ctx, "", "", quad_mat_info_);
	mat.UpdateTextures(tex);


	VKE::Entity& cube_node = hier_ctx.getEntity();
	auto cube_trans = cube_node.AddComponent<VKE::TransformComponent>(&hier_ctx, *trans);
	auto cube_rend = cube_node.AddComponent<VKE::RenderComponent>(&hier_ctx);

	VKE::InternalMaterial& cube_mat = cube_rend->getMaterial();
	cube_mat.init(&render_ctx, "test_vert.spv", "test_frag.spv", VKE::MaterialInfo());
	cube_mat.UpdateTextures(tex);

	VKE::GeoPrimitives::Sphere(&render_ctx, cube_rend);
	cube_trans->setPosition(2.0f, 0.0f, -2.0f);
	cube_trans->setRotation(0.0f, 0.0f, 0.0f);

	std::vector<int32> widths;
	std::vector<int32> heights;
	std::vector<uchar8*> cubemap_data;

	std::vector<std::string> cubemap_tex_names = {
		"./../../resources/textures/Cubemaps/Yokohama/posx.jpg",
		"./../../resources/textures/Cubemaps/Yokohama/negx.jpg",
		"./../../resources/textures/Cubemaps/Yokohama/posy.jpg",
		"./../../resources/textures/Cubemaps/Yokohama/negy.jpg",
		"./../../resources/textures/Cubemaps/Yokohama/posz.jpg",
		"./../../resources/textures/Cubemaps/Yokohama/negz.jpg",
	};

	VKE::InternalTexture::loadImageToMemory(cubemap_tex_names, cubemap_data, widths, heights, true);
	VKE::Texture cubemap_tex = render_ctx.getResource<VKE::Texture>();
	VKE::InternalTexture& cubemap_texture = render_ctx.getInternalRsc<VKE::Texture::internal_class>(cubemap_tex);
	cubemap_texture.init(&render_ctx, cubemap_data[0], widths[0], heights[0], VKE::TextureType_Cubemap);
	

	VKE::Entity& cubemap_node = hier_ctx.getEntity();
	auto cubemap_trans = cubemap_node.AddComponent<VKE::TransformComponent>(&hier_ctx, *trans);
	auto cubemap_rend = cubemap_node.AddComponent<VKE::RenderComponent>(&hier_ctx);
	VKE::GeoPrimitives::Cube(&render_ctx, cubemap_rend);

	VKE::MaterialInfo mat_info;
	mat_info.cull_mode_ = VK_CULL_MODE_FRONT_BIT;
	VKE::InternalMaterial& cubemap_mat = cubemap_rend->getMaterial();
	cubemap_mat.init(&render_ctx, "cubemap_vert.spv", "cubemap_frag.spv", mat_info);
	cubemap_mat.UpdateTextures(cubemap_tex);


	static auto startTime = std::chrono::high_resolution_clock::now();
	auto current_time = startTime;
	auto prev_time = startTime;

	while (!window.ShouldWindowClose()) {
		window.PollEvents();
		window.Update();
		render_ctx.getCamera().keyboardInput(&window);
		render_ctx.getCamera().mouseInput(&window, false);

		prev_time = current_time;
		current_time = std::chrono::high_resolution_clock::now();

		float32 time = std::chrono::duration<float32, std::chrono::seconds::period>(current_time - startTime).count();
		trans->setRotation(0.0f, 0.0f, time);

		float32 delta = std::chrono::duration<float32, std::chrono::seconds::period>(current_time - prev_time).count();
		render_ctx.getCamera().logic(delta);


		hier_ctx.UpdateManagers();
		render_ctx.draw();
	}


	return 0;
}