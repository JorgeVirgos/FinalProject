#include "window.h"
#include <GLFW/glfw3.h>
//#include "render_context.h"

VKE::Window::Window() {

}
VKE::Window::~Window() {

}
VKE::Window::Window(const Window& other) {

}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	//auto render_ctx = reinterpret_cast<RenderContext*>(glfwGetWindowUserPointer(window));
	//if (render_ctx != nullptr) render_ctx->framebufferResized = true;
}

void VKE::Window::initWindow(uint32 width, uint32 height, RenderContext* render_ctx) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(win_width, win_height, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, render_ctx);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

bool VKE::Window::ShouldWindowClose() {
	return glfwWindowShouldClose(window);
}
void VKE::Window::PollEvents() {
	glfwPollEvents();
}

void VKE::Window::shutdownWindow() {
	glfwDestroyWindow(window);
}