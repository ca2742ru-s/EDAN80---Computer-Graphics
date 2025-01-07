#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoord;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform float ellapsed_time_s;


out VS_OUT {
	vec3 vertex;
	vec3 normal_water;

	vec2 normalCoord0;
	vec2 normalCoord1; 
	vec2 normalCoord2; 

} vs_out;

uniform vec2 amplitude = vec2(1.0, 0.5);
uniform vec2 freq = vec2(0.2, 0.4);
uniform vec2 phase = vec2(0.5, 1.3);
uniform float sharpness = 1.0;
uniform vec2 direction1 = vec2(-1.0, 0.0);
uniform vec2 direction2 = vec2(-0.7, 0.7);


float wave(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time){
	return amplitude * pow(sin((position.x * direction.x + position.y * direction.y) * frequency + time * phase) * 0.5 + 0.5, sharpness);
}

float dwave(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time){
	return 0.5 * sharpness * frequency * amplitude * pow(sin((direction.x * position.x + direction.y * position.y) * frequency + time * phase) * 0.5 + 0.5, sharpness - 1) * cos((direction.x * position.x + direction.y * position.y) * frequency + time * phase);
}


void main() {

	vec2 texScale = vec2(8, 4);
	float normalTime = mod(ellapsed_time_s, 100.0f);
	vec2 normalSpeed = vec2(-0.05, 0);

	vs_out.normalCoord0.xy = vec2(texcoord.xy * texScale + normalTime * normalSpeed);
	vs_out.normalCoord1.xy = vec2(texcoord.xy * texScale * 2 + normalTime * normalSpeed * 4);
	vs_out.normalCoord2.xy = vec2(texcoord.xy * texScale * 4 + normalTime * normalSpeed * 8);

	vec3 displaced_vertex = vertex;

	float dHx;
	float dHz;

	displaced_vertex.y += wave(vertex.xz, direction1, amplitude.x, freq.x, phase.x, sharpness, ellapsed_time_s) + wave(vertex.xz, direction2, amplitude.y, freq.y, phase.y, sharpness, ellapsed_time_s); //Translate into sphere dimentions

	dHx = dwave(vertex.xz, direction1, amplitude.x, freq.x, phase.x, sharpness, ellapsed_time_s)*direction1.x + dwave(vertex.xz, direction2, amplitude.y, freq.y, phase.y, sharpness, ellapsed_time_s)*direction2.x;
	dHz = dwave(vertex.xz, direction1, amplitude.x, freq.x, phase.x, sharpness, ellapsed_time_s)*direction1.y + dwave(vertex.xz, direction2, amplitude.y, freq.y, phase.y, sharpness, ellapsed_time_s)*direction2.y;

	vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0)); //Applying the displaced vertex to vertex and moving it to world space
	vs_out.normal_water = vec3(-dHx, 1, -dHz); //creating the wave normal vector

	gl_Position = vertex_world_to_clip * vec4(vs_out.vertex, 1);

}
