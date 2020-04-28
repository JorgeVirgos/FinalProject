#include <GLFW/glfw3.h>
#include "window.h"
#include <iostream>
#include "render_context.h"
#include "hierarchy_context.h"

VKE::Window::Window() {

}
VKE::Window::~Window() {
	system("pause");
}
VKE::Window::Window(const Window& other) {
	*this = other;
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto hier_ctx = reinterpret_cast<VKE::HierarchyContext*>(glfwGetWindowUserPointer(window));
	if (hier_ctx != nullptr) {
		hier_ctx->getRenderContext()->recreateSwapChain();
		hier_ctx->UpdateManagers();
		hier_ctx->getRenderContext()->draw();
	}
}

void VKE::Window::initWindow(uint32 width, uint32 height, HierarchyContext* hier_ctx) {

	win_height = height;
	win_width = width;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(win_width, win_height, "VKE Main Window", nullptr, nullptr);
	glfwSetWindowUserPointer(window, hier_ctx);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

bool VKE::Window::ShouldWindowClose() {
	return glfwWindowShouldClose(window);
}
void VKE::Window::PollEvents() {
	glfwPollEvents();
}

void VKE::Window::Update() {
	glfwGetWindowSize(window, &win_width, &win_height);
}

void VKE::Window::shutdownWindow() {
	glfwDestroyWindow(window);
}