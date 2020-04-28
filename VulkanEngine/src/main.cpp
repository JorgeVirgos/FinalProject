#include <window.h>
#include "platform_types.h"
#include "constants.h"
#include "hierarchy_context.h"
#include "render_context.h"
#include <chrono>



int main(){

	VKE::Window	window;
	VKE::RenderContext render_ctx;
	VKE::HierarchyContext hier_ctx(&render_ctx);

	window.initWindow(720, 640, &hier_ctx);
	render_ctx.init(&window, &hier_ctx);

	static auto startTime = std::chrono::high_resolution_clock::now();

	VKE::Entity& base_node = hier_ctx.getEntity();
	auto trans = base_node.AddComponent<VKE::TransformComponent>(&hier_ctx);
	auto rend = base_node.AddComponent<VKE::RenderComponent>(&hier_ctx);

	VKE::InternalBuffer& vertex_internal_buffer = rend->getVertexBuffer();
	vertex_internal_buffer.init(&render_ctx, sizeof(VKE::Vertex), indexed_square_vertices.size(), VKE::BufferType_Vertex);
	vertex_internal_buffer.uploadData((void*)indexed_square_vertices.data());

	VKE::InternalBuffer& index_internal_buffer = rend->getIndexBuffer();
	index_internal_buffer.init(&render_ctx, sizeof(ushort16), indexed_square_indices.size(), VKE::BufferType_Index);
	index_internal_buffer.uploadData((void*)indexed_square_indices.data());

	VKE::Texture tex = render_ctx.getResource<VKE::Texture>();
	VKE::InternalTexture& texture = render_ctx.getInternalRsc<VKE::Texture::internal_class>(tex);
	texture.init(&render_ctx, "./../../resources/textures/default_placeholder.jpg", VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB);

	VKE::InternalMaterial& mat = rend->getMaterial();
	mat.init(&render_ctx);
	mat.UpdateTextures(tex);

	VKE::Entity& copy_node = hier_ctx.getEntity();
	auto copy_trans = copy_node.AddComponent<VKE::TransformComponent>(&hier_ctx, *trans);
	copy_node.AddComponent<VKE::RenderComponent>(&hier_ctx, *rend);

	copy_trans->setPosition(0.0f, -0.5f, 0.0f);

	while (!window.ShouldWindowClose()) {
		window.PollEvents();
		window.Update();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float32 time = std::chrono::duration<float32, std::chrono::seconds::period>(currentTime - startTime).count();
		trans->setRotation(0.0f, 0.0f, time);

		hier_ctx.UpdateManagers();
		render_ctx.draw();
	}


	return 0;
}