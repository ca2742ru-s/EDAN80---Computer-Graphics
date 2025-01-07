#version 410

uniform vec3 camera_position;
uniform vec3 light_position;
uniform samplerCube skybox;
uniform sampler2D normal_map;
uniform mat4 normal_model_to_world;

uniform float ellapsed_time_s;

in VS_OUT {
	vec3 vertex;
	vec3 normal_water;

	vec2 normalCoord0;
	vec2 normalCoord1; 
	vec2 normalCoord2;
} fs_in;

out vec4 frag_color;

void main()
{


	
	vec3 V = normalize(camera_position - fs_in.vertex); //View vector
	//vec3 L = normalize(light_position - fs_in.vertex); //Light vector

	vec3 t = normalize(vec3(1, fs_in.normal_water.x, 0));
	vec3 b = normalize(vec3(0, fs_in.normal_water.z, 1));
	vec3 n = normalize(fs_in.normal_water);

	mat3 TBN = mat3(t, b, n);

	//Fast fresnell
	float R0 = 0.02037;
	float fresnel = R0 + (1-R0)*pow((1 - dot(V,n)), 5);


	//Animated normal mapping:
	vec3 n0 = texture(normal_map, fs_in.normalCoord0).xyz * 2 - 1;
	vec3 n1 = texture(normal_map, fs_in.normalCoord1).xyz * 2 - 1;
	vec3 n2 = texture(normal_map, fs_in.normalCoord2).xyz * 2 - 1;

	vec3 nbump = normalize(n0 + n1 + n2);
	nbump = TBN * nbump;
	n = normalize(vec3(normal_model_to_world * vec4(nbump, 0)));

	
	//Reflection:
	vec3 R = reflect(-V, n);
	vec4 colorReflection = texture(skybox, R);

	//Refactor:
	float nfact = 1/1.33;
	vec4 refraction = texture(skybox, refract(-V, n, nfact));



	//Color:
	float facing = 1 - max(dot(V, n), 0);
	vec4 colorDeep = vec4(0.0, 0.0, 0.1, 1.0);
	vec4 colorShallow = vec4(0.0, 0.5, 0.5, 1.0);

	vec4 colorWater = mix(colorDeep, colorShallow, facing);

	frag_color = colorWater + colorReflection*fresnel + refraction*(1-fresnel);

	if(!gl_FrontFacing){
		n = -n;
	}


}
