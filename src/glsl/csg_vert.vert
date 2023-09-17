#version 430 core
layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec3 i_norm;
layout (location = 2) in vec4 i_uv0;
layout (location = 3) in vec4 i_uv1;
layout (location = 4) in vec4 i_uv2;
layout (location = 5) in vec4 i_uv3;
layout (location = 6) in vec4 i_weights;
layout (location = 7) in vec4 i_rgba;

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

uniform mat4 u_mvp, u_m, u_normal;
uniform vec3 u_cam_pos;
uniform vec4 u_uv_offset;

out vec4 v_uv0, v_uv1, v_uv2, v_uv3;
out vec4 v_weights, v_rgba;
// out vec3 v_N, v_pos;
// out float v_NdL;
out vec3 v_light_ambient, v_light_diffuse, v_light_specular;

void main()
{
	gl_Position = u_mvp * vec4(i_pos, 1);

	v_uv0 = vec4(i_uv0.x * u_uv_offset.x, i_uv0.y * u_uv_offset.y, i_uv0.z + u_uv_offset.z, i_uv0.w + u_uv_offset.w);
	v_uv1 = vec4(i_uv1.x * u_uv_offset.x, i_uv1.y * u_uv_offset.y, i_uv1.z + u_uv_offset.z, i_uv1.w + u_uv_offset.w);
	v_uv2 = vec4(i_uv2.x * u_uv_offset.x, i_uv2.y * u_uv_offset.y, i_uv2.z + u_uv_offset.z, i_uv2.w + u_uv_offset.w);
	v_uv3 = vec4(i_uv3.x * u_uv_offset.x, i_uv3.y * u_uv_offset.y, i_uv3.z + u_uv_offset.z, i_uv3.w + u_uv_offset.w);
	v_weights = i_weights;
	v_rgba = i_rgba;

	vec3 world_pos = vec3(u_m * vec4(i_pos, 1));
	vec3 N = normalize((u_normal * vec4(i_norm, 0)).xyz);
	vec3 V = normalize(world_pos - u_cam_pos);
	vec3 total_diff = vec3(0);
	vec3 total_spec = vec3(0);
	vec3 total_amb = vec3(0);
	for(uint i = 0; i < u_num_lights; i++)
	{
		Light cur_light = u_light_block.lights[i];
		float cur_type = cur_light.o2w[3][3];
		vec3 light_pos = vec3(cur_light.o2w[3][0], cur_light.o2w[3][1], cur_light.o2w[3][2]);
		vec3 L = vec3(0);
		float atten = 1, amb_atten = 1;

		// directional light
		if(cur_type == 0)
		{
			L = normalize(light_pos);
		}
		// point light
		else if(cur_type == 1)
		{
			vec3 d_pos = light_pos - world_pos;
			float d_pos_length2 = dot(d_pos, d_pos);
			float rmax2 = cur_light.rmax * cur_light.rmax;
			// current fragment beyond lights AOE
			if(d_pos_length2 >= rmax2)
				continue;
			float d_pos_length = length(d_pos);
			// (r^2) / (rmax^2) * (2r / rmax - 3) + 1
			atten = (d_pos_length2 / rmax2) * (2 * d_pos_length / cur_light.rmax - 3) + 1;

			L = normalize(light_pos - world_pos);
			amb_atten = float(dot(N, L) > 0);
		}
		// area light
		else if(cur_type == 2)
		{
			vec3 d_pos = light_pos - world_pos;
			float d_pos_length2 = dot(d_pos, d_pos);
			float rmax2 = cur_light.rmax * cur_light.rmax;
			// current fragment beyond lights AOE
			if(d_pos_length2 >= rmax2)
				continue;
		}
		// spotlight
		else if(cur_type == 3)
		{
			vec3 d_pos = light_pos - world_pos;
			float d_pos_length2 = dot(d_pos, d_pos);
			float rmax2 = cur_light.rmax * cur_light.rmax;
			// current fragment beyond lights AOE
			if(d_pos_length2 >= rmax2)
				continue;

			vec3 light_i_pos = (cur_light.w2o * vec4(world_pos, 1)).xyz;
			// current fragment is behind light
			if(light_i_pos.z >= 0)
				continue;
			float cos_theta = -light_i_pos.z / length(light_i_pos);
			if(cos_theta <= cur_light.cos_tmax)
				continue;

			float d_pos_length = length(d_pos);
			// (r^2) / (rmax^2) * (2r / rmax - 3) + 1
			atten = (d_pos_length2 / rmax2) * (2 * d_pos_length / cur_light.rmax - 3) + 1;

			L = normalize(light_pos - world_pos);
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
	v_light_ambient = total_amb;
	v_light_diffuse = total_diff;
	v_light_specular = total_spec;
}
