#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform UniformBufferMatrices {
    mat4 model;
		mat4 model_inv;
    mat4 view;
    mat4 proj;
} ubm;

layout(binding = 0, set = 2) uniform UniformLightData {
		mat4 light_space;
		vec4 light_dir;
		vec3 camera_pos;
		float shininess;
		vec3 color;
		float ambient;
		int debug_mode;
} uld;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

void main() {
    gl_Position = uld.light_space * ubm.model *vec4(inPosition, 1.0);
}