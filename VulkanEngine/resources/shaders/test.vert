#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform UniformBufferMatrices {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubm;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 1) out vec2 FragUV;


void main() {
    gl_Position = ubm.proj * ubm.view * ubm.model *vec4(inPosition, 1.0);
		FragUV = inUV;
}