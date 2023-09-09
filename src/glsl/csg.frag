#version 430 core
layout(location = 0) out vec4 o_col;

struct Light
{
	mat4 obj2world;
	vec4 ca, cd, cs;
	float sp;
	float pad0, pad1, pad2;
};

layout(std140, binding = 0) uniform LightBlock
{
	Light lights[128];
} u_light_block;
uniform uint u_num_lights;
uniform sampler2D u_tex0, u_tex1, u_tex2, u_tex3;
uniform vec3 u_cam_pos;
uniform mat4 u_m;
uniform float u_time;

in vec4 v_uv0, v_uv1, v_uv2, v_uv3;
in vec4 v_weights, v_rgba;
in vec3 v_pos, v_N;
in float v_NdL;

void main()
{
	vec3 N = normalize(v_N);

	vec4 multi_tex_res = texture(u_tex0, v_uv0.xy + v_uv0.zw * u_time) * v_weights[0] +
				texture(u_tex1, v_uv1.xy + v_uv1.zw * u_time) * v_weights[1] +
				texture(u_tex2, v_uv2.xy + v_uv2.zw * u_time) * v_weights[2] +
				texture(u_tex3, v_uv3.xy + v_uv3.zw * u_time) * v_weights[3];
	vec3 mixed_res = mix(multi_tex_res.rgb, v_rgba.rgb, v_rgba.a);

	vec3 world_pos = vec3(u_m * vec4(v_pos, 1));
	vec3 V = normalize(world_pos - u_cam_pos);
	vec3 total_light = vec3(0, 0, 0);
	vec3 total_diffuse = vec3(0.0);
	vec3 total_spec = vec3(0.0);
	vec3 total_amb = vec3(0.0);
	for(uint i = 0; i < u_num_lights; i++)
	{
		Light cur_light = u_light_block.lights[i];
		float cur_type = cur_light.obj2world[3][3];
		vec3 light_pos = vec3(cur_light.obj2world[3][0], cur_light.obj2world[3][1], cur_light.obj2world[3][2]);
		vec3 L = vec3(0, 0, 0);

		// directional light
		if(cur_type == 0)
		{
			L = normalize(light_pos);
		}
		// point light
		else if(cur_type == 1)
		{
			L = normalize(light_pos - v_pos);
		}

		vec3 R = normalize(reflect(L, N));
		float RdV = max(0, dot(V, R));
		float NdL = max(0, dot(N, L));

		vec4 amb  = cur_light.ca;
		vec4 diff = NdL * cur_light.cd;
		vec4 spec = pow(RdV, cur_light.sp) * cur_light.cs;
		total_diffuse += diff.rgb * diff.a;
		total_spec += spec.rgb * spec.a;
		total_amb += amb.rgb * amb.a;
	}
	
	o_col = vec4(clamp((total_amb + total_diffuse) * mixed_res + total_spec, vec3(0), vec3(1)), 1);
	// o_col = vec4(abs(N), 1);
}