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

layout (location = 1) out vec2 TexCoords;

void main()
{
    TexCoords = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(TexCoords * 2.0f + -1.0f, 0.0f, 1.0f);

		//TexCoords = inUV;
		//gl_Position = vec4(inPosition.xy, 0.5f, 1.0f);
}