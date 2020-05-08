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
		int debug_mode;
} uld;

layout(location = 1) in vec2 FragUV;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 pos;
layout(location = 4) in vec4 light_pos;

layout(location = 0) out vec4 outColor;

float IsFragmentShadowed(vec4 light_pos_){
	vec4 proj_coords = (light_pos_ / light_pos_.w);
	//proj_coords = (light_pos_.xyz * 0.5) + 0.5;
	
	float shadow = 1.0f;
	float bias = 0.01f;
	
	//if(proj_coords.z <= -1.0f || proj_coords.z >= 1.0f || proj_coords.w <= 0.0f) return shadow;
	float closest_depth = texture(shadowSampler, proj_coords.xy).x;
	float current_depth = proj_coords.z;

	//return (current_depth > closest_depth+ bias ? 1.0 : 0.0);
	shadow = floor(abs(step(current_depth, closest_depth + bias)));


	return shadow;

}

void main() {

		float gamma = 2.2;

    //vec4 albedo = vec4(pow(texture(texSampler, FragUV).rgb, vec3(gamma)), 1.0);
    vec4 albedo = texture(texSampler, FragUV);

		vec3 light_color = vec3(max(1.0, uld.color.x),
		max(1.0, uld.color.y),
		max(1.0, uld.color.z));

		vec3 ambient = light_color * uld.ambient;

		float light_intensity = max(0.0, dot(normal, uld.light_dir.xyz));
		vec3 diffuse = light_color * light_intensity;

		vec3 camera_dir = normalize(uld.camera_pos - pos);

		//vec3 reflect_dir = reflect(-uld.light_dir.xyz, normal);
		//float spec_factor = pow(max(dot(camera_dir, reflect_dir), 0.0),64.0);

		vec3 halfway_dir = normalize(uld.light_dir.xyz + camera_dir);
		float spec_factor = pow(max(dot(halfway_dir, normal), 0.0),32.0);

		vec3 specular_light = 0.75f * spec_factor * light_color;

		float is_shadowed = IsFragmentShadowed(light_pos);
		
		if (uld.debug_mode == 1)
				outColor = vec4(normal, 1.0);
		else if(uld.debug_mode == 2)
				outColor = vec4((light_intensity * is_shadowed).xxx,1.0);
		else if(uld.debug_mode == 3)
				outColor = vec4(specular_light,1.0);
		else
				outColor = vec4((ambient + (diffuse + specular_light) * is_shadowed) * albedo.xyz, 1.0);
    //outColor = vec4(normal, 1.0);
    //outColor = vec4(light_intensity.xxx,1.0);
}