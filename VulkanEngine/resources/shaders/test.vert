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

layout(location = 1) out vec2 FragUV;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec3 pos;
layout(location = 4) out vec4 light_pos;


const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );


void main() {
		FragUV = inUV;
		normal = normalize(mat3(transpose(ubm.model_inv)) * inNormal);
		pos = (ubm.model * vec4(inPosition, 1.0)).xyz;
    light_pos = (biasMat * uld.light_space * ubm.model) * vec4(inPosition, 1.0);

    gl_Position = ubm.proj * ubm.view * ubm.model * vec4(inPosition, 1.0);
}