#version 330 core
layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec3 i_norm;

uniform mat4 u_mvp, u_model, u_normal;
uniform float u_time, u_switch_time;

out vec3 v_pos, v_norm, v_pos_ndc;

void main()
{
	float dt = u_time - u_switch_time;
	float scale_xz = (exp(-dt * 5) * sin(dt * 30)) * .5 + 1;
	float scale_y =  (exp(-dt * 5) * cos(dt * 30)) * .6 + 1;
	vec4 pos = vec4(vec3(i_pos.x * scale_xz, i_pos.y * scale_y, i_pos.z * scale_xz), 1);
	gl_Position = u_mvp * pos;
	v_pos_ndc = gl_Position.xyz;
	v_pos = (u_model * pos).xyz;
	v_norm = (u_normal * vec4(i_norm, 0)).xyz;
}