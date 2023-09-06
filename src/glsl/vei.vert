#version 330 core
layout(location = 0) in vec3 i_pos;

uniform mat4 u_mvp, u_model;

out vec3 v_pos;

void main()
{
	vec4 pos = vec4(i_pos, 1);
	gl_Position = u_mvp * pos;
	v_pos = (u_model * pos).xyz;
}