#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 1) uniform sampler2D texSampler;

layout(location = 1) in vec2 TexCoords;

layout(location = 0) out vec4 FragColor;

float LinearizeDepth(in vec2 uv)
{
    float zNear = 0.1;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 100; // TODO: Replace by the zFar  of your perspective projection
    float depth = texture(texSampler, TexCoords).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main(){
	//float c = LinearizeDepth(TexCoords);
	//FragColor = vec4(c,c,c,1.0);

  //const float gamma = 2.2;
  //vec3 hdrColor = texture( renderedTexture, TexCoords).rgb;
  //hdrColor = pow(hdrColor, vec3(1.0 / gamma)); 
	//FragColor = vec4(hdrColor,1.0);

	FragColor = texture(texSampler, TexCoords);
}