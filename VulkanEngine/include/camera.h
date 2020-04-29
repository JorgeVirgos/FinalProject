// ----------------------------------------------------------------------------
// Copyright (C) Rize Engine / Mario Borrajo Megoya / Jorge Virgos Castejon
// Copyright (C) 2019 Jorge Virgos Castejon
// Window Management Class.
// ----------------------------------------------------------------------------

#ifndef CAMERA_H_
#define CAMERA_H_ 1

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "platform_types.h"

namespace VKE {

	class Window;

  /// \brief General purpose camera, storing the rendering matrices and responding to input.
  class Camera {
  public:
    Camera();
    ~Camera();

    /// \brief Returns a view matrix with the given position in world space, looking towards the camera_front vector respective to the camera_up.
		static glm::mat4 getViewMatrix(glm::vec3 camera_position, glm::vec3 camera_front, glm::vec3 camera_up);
    /// \brief Returns a Projection matrix with the given fov, width and height, and near and far planes.
		static glm::mat4 getProjectionMatrix(float32 fov, float32 width, float32 height, float32 near, float32 far);
    /// \brief Returns a multiplication (P*V) of the view and projection matrices resulting from the inputs.
    static glm::mat4 getViewProjectionMatrix(glm::vec3 camera_position, glm::vec3 camera_front, glm::vec3 camera_up, float32 fov, float32 width, float32 height, float32 near, float32 far);
    /// \brief Returns an orthogonal projection matrix, with the given width and height, and the specified near and far planes.
    static glm::mat4 getOrthoMatrix(float32 width, float32 height, float32 near_plane, float32 far_plane);

    /// \brief Initial parameters for the camera: position and up vectors, and yaw and pitch rotation values.
    void init(glm::vec3 camera_position, glm::vec3 camera_up, float32 yaw, float32 pitch);
    /// \brief Method setting the three speeds of the camera: moving speed, rotational speed and mouse sensitivity.
    void changeSpeed(float32 speed, float32 rotation_speed, float32 mouse_s);
    /// \brief Method setting the key toggling the rest of the camera input.
    void setCameraControlKey(uint64 key);

    /// \brief Method controlling the input from the keyboard affecting the camera. Must be called every frame after updating input.
    void keyboardInput(Window *win);
    /// \brief Method controlling the input from the mouse affecting the camera. Must be called every frame after updating input.
    void mouseInput(Window *win, bool reset_mouse);
    /// \brief Resets all the input variables of the camera.
    void resetInput();
    /// \brief Calculates logic of the camera.
    void logic(float32 delta_time);

    /// \brief Toggles whether the camera rotating respons to mouse control.
    void mouseMode(Window *win, bool mouse_control);

    /// \brief Calculates the static matrices, which DO NOT need to be calculated each frame.
    void calculateStaticMatrices(int fov, float window_width, float window_height, float near_plane, float far_plane);
    /// \brief Calculates the dynamic matrices, which NEED to be called each frame.
    void calculateDynamicMatrices();

		glm::vec3 pos();
		glm::mat4 viewProjectionMatrix();
		glm::mat4 viewMatrix();
		glm::mat4 projectionMatrix();
		glm::mat4 orthoMatrix();

  
    float near_; ///< Visibility variable of the near projection plane. Unused, kept for external references
    float far_; ///< Visibility variable of the far projection plane. Unused, kept for external references

  private:

    /// \brief Internal method called when any input is detected. Recalculates vectors to be used in matrix calcutation.
    void updateVectors();

    bool up_;     ///< Up arrow input variable.
    bool down_;   ///< Down arrow input variable.
    bool right_;  ///< Right arrow input variable.
    bool left_;   ///< Left arrow input variable.

    int32 key_x_rotation_; ///< Keyboard x rotation input
    int32 key_y_rotation_; ///< Keyboard y rotation input

    //float64 mouse_x_rotation_; ///< Mouse x rotation input
    //float64 mouse_y_rotation_; ///< Mouse y rotation input

    float32 speed_;       ///< Moving sspeed of the camera
    float32 rot_speed_;   ///< Rotational speed of the camera
    float32 sensitivity_; ///< Mouse rotational speed of the camera
    float32 yaw_;         ///< Internally stored rotational component
    float32 pitch_;       ///< Internally stored rotational component

    glm::vec3 camera_position_; ///< Camera position vector, updated after new input.
    glm::vec3 camera_up_;       ///< Camera up vector, updated after new input.
    glm::vec3 camera_front_;    ///< Camera front or forward vector, updated after new input.
    glm::vec3 camera_right_;    ///< Camera right vector, updated after new input.

		glm::mat4 view_projection_matrix_;  
		glm::mat4 projection_matrix_;       
		glm::mat4 view_matrix_;             
		glm::mat4 ortho_matrix_;            

    //float64 previous_mouse_x_position_; 
    //float64 previous_mouse_y_position_; 

    uint64 camera_control_key_;   ///< Enum variable describing the key that toggles the camera input.
    bool mouse_controlling_camera_; ///< Whether the input of the camera is activated or not.
  };

}

#endif // CAMERA_H_