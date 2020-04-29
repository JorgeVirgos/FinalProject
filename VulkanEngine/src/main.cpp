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
	texture.init(&render_ctx, "./../../resources/textures/default_placeholder.jpg", VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB);

	VKE::InternalMaterial& mat = rend->getMaterial();
	mat.init(&render_ctx);
	mat.UpdateTextures(tex);


	VKE::Entity& cube_node = hier_ctx.getEntity();
	auto cube_trans = cube_node.AddComponent<VKE::TransformComponent>(&hier_ctx, *trans);
	auto cube_rend = cube_node.AddComponent<VKE::RenderComponent>(&hier_ctx);

	VKE::InternalMaterial& cube_mat = cube_rend->getMaterial();
	cube_mat.init(&render_ctx);
	cube_mat.UpdateTextures(tex);

	VKE::GeoPrimitives::Sphere(&render_ctx, cube_rend);
	cube_trans->setPosition(2.0f, 0.0f, -2.0f);
	cube_trans->setRotation(0.0f, 0.0f, 0.0f);



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
		render_ctx.UpdateCameraViewMatrix(render_ctx.getCamera().viewMatrix());
		render_ctx.UpdateCameraProjectionMatrix(render_ctx.getCamera().projectionMatrix());

		hier_ctx.UpdateManagers();
		render_ctx.draw();
	}


	return 0;
}