#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 1) uniform samplerCube cubemap;
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


layout(location = 1) in vec3 vert_texcoord;
layout(location = 2) in mat4 proj;

layout(location = 0) out vec4 outColor;

void main() {
		
		float width = 720;
		float height = 640;
		vec3 sunBaseColor = uld.color * 16.0;

		vec3 out_color = texture(cubemap, normalize(vert_texcoord)).xyz * (uld.color + uld.ambient);

		float sunRadius = length(normalize(vert_texcoord) - normalize(uld.light_dir.xyz));
		float smoothRadius = 0.0f;

		float r_Sun = 0.15f;
		if(sunRadius < r_Sun)
		{
			sunRadius /= r_Sun;
			smoothRadius = smoothstep(0,1,0.1f/sunRadius-0.1f);
			out_color = mix(out_color, sunBaseColor, smoothRadius);
			
			//smoothRadius = smoothstep(0,1,0.18f/sunRadius-0.2f);
			//out_LightScattering = mix(vec3(0), sunBaseColor, smoothRadius);
		}



		if(uld.debug_mode == 1)
				outColor = vec4(vert_texcoord,1.0);
		else if(uld.debug_mode == 2 || uld.debug_mode == 3)
				outColor = vec4(smoothRadius.xxx,1.0);
		else
				outColor = vec4(out_color,1.0);


		//float inside_sun = step(0.999, dot(normalize(vert_texcoord), normalize(light_dir.xyz)));
		//if(inside_sun == 1.0)
		//		outColor = vec4(1.0f,1.0f,0.0f,1.0f);
		//else
		//		outColor = vec4(0.1,0.1,0.1,1.0f);

    //outColor = texture(cubemap, normalize(vert_texcoord));
}