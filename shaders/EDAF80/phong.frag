#version 410

uniform sampler2D normal_map;
uniform sampler2D diffuse_map;

uniform vec3 light_position;
uniform vec3 camera_position;
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;

uniform float shininess;

uniform mat4 normal_model_to_world;

uniform bool use_normal_mapping;

in VS_OUT {
	vec3 vertex;
	vec3 tangent;
	vec3 normal;
	vec3 binormal;
	vec2 texcoord;
} fs_in;

out vec4 frag_color;

void main()
{

	vec3 Diffuse;
	vec3 Specular;

	vec3 L = normalize(light_position - fs_in.vertex);
	vec3 V = normalize(camera_position - fs_in.vertex);

	if (use_normal_mapping){

		mat3 TBN = mat3(normalize(fs_in.tangent), normalize(fs_in.binormal), normalize(fs_in.normal));

		vec3 normal = texture(normal_map, fs_in.texcoord).xyz * 2 - 1;
		vec4 normal_world = normal_model_to_world * vec4(TBN * normal, 0.0f);
		normal = normalize(normal_world.xyz);

		Diffuse = texture(diffuse_map, fs_in.texcoord).rgb * max(dot(normal, L), 0.0f);
		Specular = specular * pow(max(dot(reflect(-L, normal), V), 0), shininess);

	} else {

		vec4 normal_world = normal_model_to_world * vec4(fs_in.normal, 0.0f);
		Diffuse = texture(diffuse_map, fs_in.texcoord).rgb * max(dot(normalize(normal_world.xyz), L), 0.0f);
		Specular = specular * pow( max(dot(reflect(-L, normalize(normal_world.xyz)), V), 0), shininess);
	}

	frag_color = vec4(ambient + Diffuse + Specular, 1.0);
}
