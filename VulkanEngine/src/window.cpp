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

void VKE::Window::shutdownWindow() {
	glfwDestroyWindow(window);
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto hier_ctx = reinterpret_cast<VKE::HierarchyContext*>(glfwGetWindowUserPointer(window));
	if (hier_ctx != nullptr) {
		hier_ctx->getRenderContext()->recreateSwapChain();
		hier_ctx->UpdateManagers();
		hier_ctx->getRenderContext()->getCamera().calculateStaticMatrices(70.0f, width, height, 0.1f, 100.0f);
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
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
}

bool VKE::Window::ShouldWindowClose() {
	return glfwWindowShouldClose(window);
}
void VKE::Window::PollEvents() {
	glfwPollEvents();
}

void VKE::Window::Update() {
	glfwGetWindowSize(window, &win_width, &win_height);

	previous_input_key_states_ = input_key_states_;
	previous_input_mouse_key_states_ = input_mouse_key_states_;

	for (uint64 key_id = 32; key_id < GLFW_KEY_LAST; ++key_id) {
		input_key_states_[key_id] = (KeyState)glfwGetKey(window, key_id);
	}

	for (uint64 mouse_button_id = GLFW_MOUSE_BUTTON_1; mouse_button_id < GLFW_MOUSE_BUTTON_8 + 1; ++mouse_button_id) {
		input_mouse_key_states_[mouse_button_id] = (KeyState)glfwGetMouseButton(window, mouse_button_id);
	}

}

VKE::KeyState VKE::Window::GetKeyState(uint64 input_key) {
	return input_key_states_[input_key];
}
bool VKE::Window::IsKeyUp(uint64 input_key) {
	return previous_input_key_states_[input_key] == VKE::KeyState_Pressed &&
		input_key_states_[input_key] == VKE::KeyState_Released;
}
bool VKE::Window::IsKeyDown(uint64 input_key) {
	return previous_input_key_states_[input_key] == VKE::KeyState_Released &&
		input_key_states_[input_key] == VKE::KeyState_Pressed;
}
bool VKE::Window::IsKeyPressed(uint64 input_key) {
	return input_key_states_[input_key] == VKE::KeyState_Pressed;
}

VKE::KeyState VKE::Window::GetMouseButtonState(uint64 input_key) {
	return input_mouse_key_states_[input_key];
}
bool VKE::Window::IsMouseButtonUp(uint64 input_key) {
	return previous_input_mouse_key_states_[input_key] == VKE::KeyState_Pressed &&
		input_mouse_key_states_[input_key] == VKE::KeyState_Released;
}
bool VKE::Window::IsMouseButtonDown(uint64 input_key) {
	return previous_input_mouse_key_states_[input_key] == VKE::KeyState_Released &&
		input_mouse_key_states_[input_key] == VKE::KeyState_Pressed;
}
bool VKE::Window::IsMouseButtonPressed(uint64 input_key) {
	return input_mouse_key_states_[input_key] == VKE::KeyState_Pressed;
}

void VKE::Window::SetMousePosition(float64 x, float64 y) {
	glfwSetCursorPos(window, x, y);
}
void VKE::Window::getMousePosition(float64& x, float64& y) {
	glfwGetCursorPos(window, &x, &y);
}