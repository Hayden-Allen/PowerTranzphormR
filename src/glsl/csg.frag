#version 430 core
layout(location = 0) out vec4 o_col;

uniform sampler2D u_tex0, u_tex1, u_tex2, u_tex3;
uniform vec3 u_cam_pos;
uniform mat4 u_m;

in vec2 v_uv0, v_uv1, v_uv2, v_uv3;
in float v_w0, v_w1, v_w2, v_w3;
in vec4 v_rgba;
in vec3 v_pos, v_N;
in float v_NdL;

void main()
{
	vec3 world_pos = vec3(u_m * vec4(v_pos, 1));
	vec3 V = normalize(world_pos - u_cam_pos);
	vec3 L = normalize(u_cam_pos);
	vec3 R = normalize(reflect(L, v_N));
	float RdV = pow(max(0, dot(V, R)), 16);

	vec4 multi_tex_res = texture(u_tex0, v_uv0) * v_w0 + texture(u_tex1, v_uv1) * v_w1 + texture(u_tex2, v_uv2) * v_w2 + texture(u_tex3, v_uv3) * v_w3;
	vec3 mixed_res = mix(multi_tex_res.rgb, v_rgba.rgb, v_rgba.a);

	vec3 spec = RdV * vec3(1);
	vec3 diff = min(1, v_NdL + .33) * mixed_res;
	o_col = clamp(vec4(diff + spec * 0.0, 1.0), vec4(vec3(0), 1), vec4(1));
}