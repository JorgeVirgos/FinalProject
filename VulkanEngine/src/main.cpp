#include <window.h>
#include <platform_types.h>
#include <constants.h>
#include <render_context.h>

const std::vector<VKE::Vertex> square_vertices = {
	{{-0.5f,-0.5f,0.0f},{1.0f,0.0f,0.0f},{0.5f,0.0f}},
	{{0.5f,-0.5f,0.0f},{0.0f,1.0f,0.0f},{1.0f,1.0f}},
	{{-0.5f,0.5f,0.0f},{0.0f,0.0f,1.0f},{0.0f,1.0f}},

	{{0.5f,-0.5f,0.0f},{0.0f,1.0f,0.0f},{1.0f,1.0f}},
	{{0.5f,0.5f,0.0f},{0.0f,0.0f,1.0f},{0.0f,1.0f}},
	{{-0.5f,0.5f,0.0f},{0.0f,0.0f,1.0f},{0.0f,1.0f}}
};

int main(){

	VKE::Window	window;
	VKE::RenderContext render_ctx;

	window.initWindow(0, 0, &render_ctx);
	render_ctx.init(window.window);



	int it = 0;
	VKE::Buffer square_buf;


	while (!window.ShouldWindowClose()) {
		window.PollEvents();

		if (it == 2500) {
			square_buf = render_ctx.getBuffer();
			VKE::InternalBuffer& internal_buffer = render_ctx.accessInternalBuffer(square_buf);
			internal_buffer.init(&render_ctx, square_vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
			internal_buffer.uploadData(square_vertices, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}
		//if (it > 5000) {
		//	VKE::InternalBuffer& internal_buffer = render_ctx.accessInternalBuffer(square_buf);
		//	vkDeviceWaitIdle(render_ctx.getDevice());
		//	internal_buffer.reset(&render_ctx);
		//	it = 0;
		//}

		render_ctx.draw();
		it++;
	}

	return 0;
}