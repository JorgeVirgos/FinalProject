#ifndef __WINDOW_CLASS_H__
#define __WINDOW_CLASS_H__ 1

#include "platform_types.h"

class GLFWwindow;

namespace VKE {

	class RenderContext;

	class Window {
	public:

		Window();
		~Window();
		Window(const Window& other);

		void initWindow(uint32 width, uint32 height, RenderContext* render_ctx);

		bool ShouldWindowClose();
		void PollEvents();

		uint32 width() { return win_width; }
		uint32 height() { return win_height; }

		void shutdownWindow();

		GLFWwindow* window = nullptr;

	private:

		const uint32 win_width = 800;
		const uint32 win_height = 600;

	};
}
#endif /__PLATFORM_TYPES_H__
