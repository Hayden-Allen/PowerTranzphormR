#version 430 core
layout(location = 0) out vec4 o_col;

struct Light
{
	mat4 o2w, w2o;
	vec4 ca, cd, cs;
	float sp, rmax;
	float cos_tmin, cos_tmax;
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

	vec4 multi_tex_res =	texture(u_tex0, v_uv0.xy - v_uv0.zw * u_time) * v_weights[0] +
				texture(u_tex1, v_uv1.xy - v_uv1.zw * u_time) * v_weights[1] +
				texture(u_tex2, v_uv2.xy - v_uv2.zw * u_time) * v_weights[2] +
				texture(u_tex3, v_uv3.xy - v_uv3.zw * u_time) * v_weights[3];
	vec3 mixed_res = mix(multi_tex_res.rgb, v_rgba.rgb, v_rgba.a);

	vec3 world_pos = vec3(u_m * vec4(v_pos, 1));
	vec3 V = normalize(world_pos - u_cam_pos);
	vec3 total_light = vec3(0, 0, 0);
	vec3 total_diff = vec3(0.0);
	vec3 total_spec = vec3(0.0);
	vec3 total_amb = vec3(0.0);
	for(uint i = 0; i < u_num_lights; i++)
	{
		Light cur_light = u_light_block.lights[i];
		float cur_type = cur_light.o2w[3][3];
		vec3 light_pos = vec3(cur_light.o2w[3][0], cur_light.o2w[3][1], cur_light.o2w[3][2]);
		vec3 L = vec3(0, 0, 0);
		float atten = 1.f, amb_atten = 1.f;

		// directional light
		if(cur_type == 0)
		{
			L = normalize(light_pos);
		}
		// point light
		else if(cur_type == 1)
		{
			vec3 d_pos = light_pos - v_pos;
			float d_pos_length2 = dot(d_pos, d_pos);
			float rmax2 = cur_light.rmax * cur_light.rmax;
			// current fragment beyond lights AOE
			if(d_pos_length2 >= rmax2)
				continue;
			float d_pos_length = length(d_pos);
			// (r^2) / (rmax^2) * (2r / rmax - 3) + 1
			atten = (d_pos_length2 / rmax2) * (2 * d_pos_length / cur_light.rmax - 3) + 1;

			L = normalize(light_pos - v_pos);
			amb_atten = float(dot(N, L) > 0);
		}
		// area light
		else if(cur_type == 2)
		{
			vec3 d_pos = light_pos - v_pos;
			float d_pos_length2 = dot(d_pos, d_pos);
			float rmax2 = cur_light.rmax * cur_light.rmax;
			// current fragment beyond lights AOE
			if(d_pos_length2 >= rmax2)
				continue;
		}
		// spotlight
		else if(cur_type == 3)
		{
			vec3 d_pos = light_pos - v_pos;
			float d_pos_length2 = dot(d_pos, d_pos);
			float rmax2 = cur_light.rmax * cur_light.rmax;
			// current fragment beyond lights AOE
			if(d_pos_length2 >= rmax2)
				continue;

			vec3 light_v_pos = (cur_light.w2o * vec4(v_pos, 1)).xyz;
			// current fragment is behind light
			if(light_v_pos.z <= 0)
				continue;
			float cos_theta = light_v_pos.z / length(light_v_pos);
			if(cos_theta <= cur_light.cos_tmax)
				continue;

			float d_pos_length = length(d_pos);
			// (r^2) / (rmax^2) * (2r / rmax - 3) + 1
			atten = (d_pos_length2 / rmax2) * (2 * d_pos_length / cur_light.rmax - 3) + 1;

			L = normalize(light_pos - v_pos);
			amb_atten = float(dot(N, L) > 0);

			float angular_atten = clamp((cos_theta - cur_light.cos_tmax) / (cur_light.cos_tmin - cur_light.cos_tmax), 0, 1);
			atten *= angular_atten;
		}

		vec3 R = normalize(reflect(L, N));
		float RdV = max(0, dot(V, R));
		float NdL = max(0, dot(N, L));

		vec4 amb  = atten * amb_atten * cur_light.ca;
		vec4 diff = atten * NdL * cur_light.cd;
		vec4 spec = atten * pow(RdV, cur_light.sp) * cur_light.cs;
		total_amb += amb.rgb * amb.a;
		total_diff += diff.rgb * diff.a;
		total_spec += spec.rgb * spec.a;
	}
	
	o_col = vec4(clamp((total_amb + total_diff) * mixed_res + total_spec, vec3(0), vec3(1)), 1);
	// o_col = vec4(abs(N), 1);
}