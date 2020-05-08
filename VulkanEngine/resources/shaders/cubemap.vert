#version 450
//#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform UniformBufferMatrices {
    mat4 model;
		mat4 model_inv;
    mat4 view;
    mat4 proj;
} ubm;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 1) out vec3 vert_texcoord;
layout(location = 2) out mat4 proj;


void main() {
		vert_texcoord = inPosition;
		proj = ubm.proj;

		//vec4 pos = vec4(inPosition, 0.0f);
		//pos = pos * ubm.view;
		//pos.w = 1.0f;
		//gl_Position = pos * ubm.proj;

		//mat4 expanded_v_matrix = mat4(mat3(ubm.view));
    //gl_Position =  (ubm.proj * expanded_v_matrix) * vec4(inPosition, 0.000001f);

		vec3 position = (mat3(ubm.view)) * inPosition;
		gl_Position = (ubm.proj * vec4(position, 1.0f)).xyzz;
}