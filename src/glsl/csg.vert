#version 430 core
layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec2 i_tex;
uniform mat4 u_mvp;
out vec4 v_pos;
out vec2 v_tex;
void main()
{
	v_tex = i_tex;
	v_pos = gl_Position = u_mvp * vec4(i_pos, 1);
}
