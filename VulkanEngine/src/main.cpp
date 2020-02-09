#include <window.h>
#include <platform_types.h>
#include <constants.h>
#include <render_context.h>

int main(){

	VKE::Window	window;
	VKE::RenderContext render_ctx;

	window.initWindow(0, 0, &render_ctx);
	render_ctx.init(window.window);

	while (!window.ShouldWindowClose()) {
		window.PollEvents();
		render_ctx.draw();
	}

	return 0;
}