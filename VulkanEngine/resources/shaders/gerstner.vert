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

		float shineexp;
		float exposure;
		float time;
		float empty;

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

const float amplitudes[] = {
	1.5, 0.5, 0.35, 0.025
};

const float wavelengths[] = {
	50.0, 20.0, 16.0, 0.8
};

const float speeds[] = {
	1.0, 3.5, 4.0, 3.0
};

const vec2 wave_dirs[] = {
	{1.0, 1.0}, {0.25, -0.75}, {-0.6, 0.4}, {1.0, 0.0},
};

const float q_factor[] = {
	0.75, 0.75, 0.75, 0.75
};

const int wave_num = 4;

vec3 CalculateWavePos(){

	vec3 final_wave_displacement = vec3(0.0,0.0,0.0);

	for(uint i = 0; i < wave_num; ++i){

		float amplitude = amplitudes[i];
		float frequency = (2*3.1416) / wavelengths[i];
		float phase = speeds[i] * frequency;
		float q_constant = q_factor[i] / (frequency * amplitude * wave_num);
		vec2 wave_dir = normalize(wave_dirs[i]);

		final_wave_displacement.x += q_constant * amplitude * wave_dir.x * cos(frequency * dot(wave_dir, inPosition.xz) + phase * uld.time);
		final_wave_displacement.y += amplitude * sin(frequency * dot(wave_dir, inPosition.xz) + phase * uld.time);
		final_wave_displacement.z += q_constant * amplitude * wave_dir.y * cos(frequency * dot(wave_dir, inPosition.xz) + phase * uld.time);
	}

	return vec3(inPosition.x, 0.0, inPosition.z) + final_wave_displacement;

}

vec3 CalculateWaveNormal(vec3 pos){

	vec3 final_normal_displacement = vec3(0.0,0.0,0.0);

	for(uint i = 0; i < wave_num-1; ++i){

		float amplitude = amplitudes[i];
		float frequency = (2*3.1416) / wavelengths[i];
		float phase = speeds[i] * frequency;
		float q_constant = q_factor[i] / (frequency * amplitude * wave_num);
		vec3 wave_dir = normalize(vec3(wave_dirs[i].x, 0.0, wave_dirs[i].y));

		float wa = frequency * amplitude;
		float sinus   = sin(frequency * dot(wave_dir, pos) + phase * uld.time);
		float cosinus = cos(frequency * dot(wave_dir, pos) + phase * uld.time);

		final_normal_displacement.x += wave_dir.x * wa * cosinus;
		final_normal_displacement.y += q_constant * wa * sinus;
		final_normal_displacement.z += wave_dir.z * wa * cosinus;
	}

	return vec3(-final_normal_displacement.x, 1.0 - final_normal_displacement.y, -final_normal_displacement.z);
}


void main() {

		// float amplitude = amplitudes[0];
		// float frequency = (2*3.1416) / wavelengths[0];
		// float phase = speeds[0] * frequency;
		// float q_constant = 0.75 / (frequency * amplitude);
		// float time = uld.time;
		// vec2 wave_dir = wave_dirs[0];
		// vec3 final_pos = inPosition;

		// final_pos.x = final_pos.x + (q_constant * amplitude * wave_dir.x * cos(frequency * dot(wave_dir, final_pos.xz) + phase * time));
		// final_pos.y = amplitude * sin(frequency * dot(wave_dir, final_pos.xz) + phase * time);
		// final_pos.z = final_pos.z + (q_constant * amplitude * wave_dir.y * cos(frequency * dot(wave_dir, final_pos.xz) + phase * time));

		vec3 final_pos, final_normal;
		
		final_pos = CalculateWavePos();
		final_normal = CalculateWaveNormal(final_pos);

		FragUV = inUV;
		normal = normalize(mat3(transpose(ubm.model_inv)) * final_normal);
		pos = (ubm.model * vec4(final_pos, 1.0)).xyz;
    light_pos = (biasMat * uld.light_space * ubm.model) * vec4(final_pos.x, inPosition.y, final_pos.z, 1.0);

    gl_Position = ubm.proj * ubm.view * ubm.model * vec4(final_pos, 1.0);
}