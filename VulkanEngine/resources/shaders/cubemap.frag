#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 1) uniform samplerCube cubemap;

layout(location = 1) in vec3 vert_texcoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(cubemap, normalize(vert_texcoord));
}