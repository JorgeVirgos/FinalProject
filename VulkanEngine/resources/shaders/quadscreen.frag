#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 1) uniform sampler2D texSampler;
layout(binding = 1, set = 1) uniform sampler2D shadowSampler;

layout(binding = 0, set = 2) uniform UniformLightData {
		mat4 light_space;
		vec4 light_dir;

		vec3 camera_pos;
		float shininess;

		vec3 color;
		float ambient;

		float shineexp;
		float exposure;
		vec2 empty;

		int debug_mode;
} uld;


layout(location = 1) in vec2 TexCoords;

layout(location = 0) out vec4 FragColor;

float LinearizeDepth(in vec2 uv)
{
    float zNear = 0.1;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 1000.0; // TODO: Replace by the zFar  of your perspective projection
    float depth = texture(shadowSampler, TexCoords).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main(){
	//float c = LinearizeDepth(TexCoords);
	//FragColor = vec4(c,c,c,1.0);

  //const float gamma = 2.2;
  //vec3 hdrColor = texture( renderedTexture, TexCoords).rgb;
  //hdrColor = pow(hdrColor, vec3(1.0 / gamma)); 
	//FragColor = vec4(hdrColor,1.0);

  vec4 color = texture(texSampler, TexCoords);

  float luminance = 0.2126*color.r + 0.7152*color.g + 0.0722*color.b;
  vec3 hdr_mapped = vec3(1.0, 1.0, 1.0) - exp(-color.xyz * uld.exposure);
  

	//FragColor = color;
	FragColor = vec4(hdr_mapped, 1.0);
	//FragColor = vec4(LinearizeDepth(TexCoords).xxx, 1.0);
}