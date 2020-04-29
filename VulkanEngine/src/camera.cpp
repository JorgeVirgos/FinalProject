#include "camera.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "constants.h"
#include "window.h"

VKE::Camera::Camera() {
	speed_ = 2.0f;
	rot_speed_ = 50.0f;
	sensitivity_ = 0.05f;

	camera_position_ = glm::vec3(0.0f, 0.0f, 3.0f);
	camera_up_ = glm::vec3(0.0f, 0.0f, 0.0f);

	yaw_ = -90.0f;
	pitch_ = 0.0f;

	//previous_mouse_x_position_ = 0.0f;
	//previous_mouse_y_position_ = 0.0f;

	key_x_rotation_ = 0;
	key_y_rotation_ = 0;

	//mouse_x_rotation_ = 0.0f;
	//mouse_y_rotation_ = 0.0f;

	camera_control_key_ = GLFW_KEY_LEFT_SHIFT;
	mouse_controlling_camera_ = true;

	updateVectors();

}

VKE::Camera::~Camera() {

}

void VKE::Camera::init(glm::vec3 camera_position, glm::vec3 camera_up, float32 yaw, float32 pitch) {
	camera_position_ = camera_position;
	camera_up_ = camera_up;

	yaw_ = yaw;
	pitch_ = pitch;

	updateVectors();
}

void VKE::Camera::changeSpeed(float32 speed, float32 rotation_speed, float32 mouse_s) {
	speed_ = speed;
	rot_speed_ = rotation_speed;
	sensitivity_ = mouse_s;
}

void VKE::Camera::setCameraControlKey(uint64 key) {
	camera_control_key_ = key;
}


void VKE::Camera::keyboardInput(Window *win) {

	if (!mouse_controlling_camera_) return;

	up_ = win->GetKeyState(GLFW_KEY_W) == VKE::KeyState_Pressed;
	down_ = win->GetKeyState(GLFW_KEY_S) == VKE::KeyState_Pressed;
	right_ = win->GetKeyState(GLFW_KEY_D) == VKE::KeyState_Pressed;
	left_ = win->GetKeyState(GLFW_KEY_A) == VKE::KeyState_Pressed;

	key_y_rotation_ = 0;
	key_x_rotation_ = 0;

	key_y_rotation_ += win->GetKeyState(GLFW_KEY_UP) == VKE::KeyState_Pressed;
	key_y_rotation_ -= win->GetKeyState(GLFW_KEY_DOWN) == VKE::KeyState_Pressed;
	key_x_rotation_ += win->GetKeyState(GLFW_KEY_RIGHT) == VKE::KeyState_Pressed;
	key_x_rotation_ -= win->GetKeyState(GLFW_KEY_LEFT) == VKE::KeyState_Pressed;
}

void VKE::Camera::mouseInput(Window *win, bool reset_mouse) {

	if (win->GetKeyState(camera_control_key_)) {
		mouse_controlling_camera_ = true;
	}
	else {
		mouse_controlling_camera_ = false;
	}

	if (win->GetKeyState(camera_control_key_) && reset_mouse) {
		//win->setMousePosition(win->width() / 2, win->height() / 2);
		//win->captureMouse(true);
	}

	if (win->GetKeyState(camera_control_key_) && reset_mouse) {
		//win->captureMouse(false);
	}

	if (!mouse_controlling_camera_) return;

	float64 x_pos, y_pos;
	//win->mousePosition(&x_pos, &y_pos);

	glm::clamp<int32>(x_pos, 0, win->width());
	glm::clamp<int32>(y_pos, 0, win->height());


	if (reset_mouse) {

		int32 scr_width_h = win->width() / 2;
		int32 scr_height_h = win->height() / 2;

		//mouse_x_rotation_ = x_pos - scr_width_h;
		//mouse_y_rotation_ = y_pos - scr_height_h;
		//win->setMousePosition(scr_width_h, scr_width_h);

	}
	else {

		if (win->GetKeyState(camera_control_key_)) {
			//previous_mouse_x_position_ = x_pos;
			//previous_mouse_y_position_ = y_pos;
		}

		//mouse_x_rotation_ = x_pos - previous_mouse_x_position_;
		//mouse_y_rotation_ = y_pos - previous_mouse_y_position_;
		//previous_mouse_x_position_ = x_pos;
		//previous_mouse_y_position_ = y_pos;
		//win->captureMouse(false);


		//int32 new_x_pos = 0, new_y_pos = 0;

		//if (previous_mouse_x_position_ >= win->width()) new_x_pos = 10;
		//else if (previous_mouse_x_position_ <= 0) new_x_pos = win->width() - 10;
		//else new_x_pos = previous_mouse_x_position_;

		//if (previous_mouse_y_position_ >= win->height()) new_y_pos = 10;
		//else if (previous_mouse_y_position_ <= 0) new_y_pos = win->height() - 10;
		//else new_y_pos = previous_mouse_y_position_;

		//if (new_x_pos != previous_mouse_x_position_ || new_y_pos != previous_mouse_y_position_) {
		//	win->setMousePosition(new_x_pos, new_y_pos);
		//	previous_mouse_x_position_ = new_x_pos;
		//	previous_mouse_y_position_ = new_y_pos;
		//}
	}
}


void VKE::Camera::resetInput() {
	up_ = false;
	down_ = false;
	left_ = false;
	right_ = false;

	key_x_rotation_ = 0;
	key_y_rotation_ = 0;

	//mouse_x_rotation_ = 0.0f;
	//mouse_y_rotation_ = 0.0f;
}

void VKE::Camera::logic(float32 delta_time) {

	float32 mov_speed = speed_ * delta_time;
	if (up_)
		camera_position_ += camera_front_ * mov_speed;
	if (down_)
		camera_position_ -= camera_front_ * mov_speed;
	if (left_)
		camera_position_ -= camera_right_ * mov_speed;
	if (right_)
		camera_position_ += camera_right_ * mov_speed;


	if (key_x_rotation_ != 0) {
		yaw_ += key_x_rotation_ * delta_time * rot_speed_;
	}
	if (key_y_rotation_ != 0) {
		pitch_ += key_y_rotation_ * delta_time * rot_speed_;
	}

	//yaw_ += mouse_x_rotation_ * sensitivity_;
	//pitch_ -= mouse_y_rotation_ * sensitivity_;

	// This fps camera clamps the y axis to evade gimball lock and camera flip
	if (pitch_ > 89.0f)
		pitch_ = 89.0f;
	if (pitch_ < -89.0f)
		pitch_ = -89.0f;

	if (key_x_rotation_ || key_y_rotation_ /*|| mouse_x_rotation_ || mouse_y_rotation_*/ ||
		up_ || down_ || left_ || right_) {
		updateVectors();
	}

	//spdlog::get("console")->debug("Camera pos: x = {}, y = {}, z = {};", camera_position_.x, camera_position_.y, camera_position_.z);

	resetInput();
	calculateDynamicMatrices();
}

void VKE::Camera::mouseMode(Window *win, bool mouse_control) {
	//win->captureMouse(mouse_control);
}

glm::vec3 VKE::Camera::pos() {
	return camera_position_;
}

glm::mat4 VKE::Camera::getViewMatrix(glm::vec3 camera_position, glm::vec3 camera_front, glm::vec3 camera_up) {
	return glm::lookAt(camera_position, camera_position + camera_front, camera_up);
}

glm::mat4 VKE::Camera::getProjectionMatrix(float32 fov, float32 width, float32 height, float32 near_plane, float32 far_plane) {
	return glm::perspective(glm::radians(fov), width / height, near_plane, far_plane);
}

glm::mat4 VKE::Camera::getOrthoMatrix(float32 width, float32 height, float32 initial_x, float32 initial_y) {
	 glm::mat4 proj = {
			2.0f / (width - initial_x),   0.0f,         0.0f,   0.0f ,
			0.0f,         2.0f / (initial_y - height),   0.0f,   0.0f ,
			0.0f,         0.0f,        -1.0f,   0.0f ,
			(width + initial_x) / (initial_x - width),  (initial_y + height) / ( height - initial_y),  0.0f,   1.0f
	};

	 return proj;
}

glm::mat4 VKE::Camera::getViewProjectionMatrix(glm::vec3 camera_position, glm::vec3 camera_front, glm::vec3 camera_up, float32 fov, float32 width, float32 height, float32 near_plane, float32 far_plane) {
	return  glm::perspective(glm::radians(fov), width / height, near_plane, far_plane) * glm::lookAt(camera_position, camera_position + camera_front, camera_up);
}

void VKE::Camera::updateVectors() {

	glm::vec3 front;
	front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
	front.y = sin(glm::radians(pitch_));
	front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
	camera_front_ = glm::normalize(front);

	// Also re-calculate the Right and Up vector
	camera_right_ = glm::normalize(glm::cross(camera_front_, glm::vec3(0.0f, 1.0f, 0.0f)));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	camera_up_ = glm::normalize(glm::cross(camera_right_, camera_front_));
}

void VKE::Camera::calculateStaticMatrices(int fov, float window_width, float window_height, float near_plane, float far_plane) {
	
	projection_matrix_ = getProjectionMatrix(fov, window_width, window_height, near_plane, far_plane);
	projection_matrix_[1][1] *= -1.0f; // Reversal of Y axis, Vulkan axis

	//ortho_matrix_ = getOrthoMatrix(window_width, window_height, 0, 0);
  ortho_matrix_ = glm::ortho(0.0f, window_width, window_height, 0.0f);

  near_ = near_plane;
  far_ = far_plane;
}

void VKE::Camera::calculateDynamicMatrices() {
	view_matrix_ = getViewMatrix(camera_position_, camera_front_, camera_up_);
	view_projection_matrix_ = projection_matrix_ * view_matrix_;
}


glm::mat4 VKE::Camera::viewProjectionMatrix() {
	return view_projection_matrix_;
}
glm::mat4 VKE::Camera::viewMatrix() {
	return view_matrix_;
}
glm::mat4 VKE::Camera::projectionMatrix() {
	return projection_matrix_;
}
glm::mat4 VKE::Camera::orthoMatrix() {
	return ortho_matrix_;
}

