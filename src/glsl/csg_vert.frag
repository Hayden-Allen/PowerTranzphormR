#version 430 core
layout(location = 0) out vec4 o_col;

/*
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
*/

uniform sampler2D u_tex0, u_tex1, u_tex2, u_tex3;
// uniform vec3 u_cam_pos;
// uniform mat4 u_m;
uniform float u_time;
uniform bool u_enable_lighting;

in vec4 v_uv0, v_uv1, v_uv2, v_uv3;
in vec4 v_weights, v_rgba;
// in vec3 v_pos, v_N;
// in float v_NdL;
in vec3 v_light_ambient, v_light_diffuse, v_light_specular;

void main()
{
	// vec3 N = normalize(v_N);

	vec4 multi_tex_res =	texture(u_tex0, v_uv0.xy - v_uv0.zw * u_time) * v_weights[0] +
				texture(u_tex1, v_uv1.xy - v_uv1.zw * u_time) * v_weights[1] +
				texture(u_tex2, v_uv2.xy - v_uv2.zw * u_time) * v_weights[2] +
				texture(u_tex3, v_uv3.xy - v_uv3.zw * u_time) * v_weights[3];
	vec3 mixed_res = mix(multi_tex_res.rgb, v_rgba.rgb, v_rgba.a);

	if (!u_enable_lighting)
	{
		o_col = vec4(clamp(mixed_res, vec3(0), vec3(1)), 1.0);
		return;
	}

	o_col = vec4(clamp((v_light_ambient + v_light_diffuse) * mixed_res + v_light_specular, vec3(0), vec3(1)), 1);
}