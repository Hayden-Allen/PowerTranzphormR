#version 430 core
layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec2 i_tex;
uniform mat4 u_mvp;
uniform sampler2D u_heightmap;
out vec2 v_tex;
void main()
{
	v_tex = i_tex;
	vec3 pos = i_pos;
	pos.y *= texture(u_heightmap, i_tex).r;
	gl_Position = u_mvp * vec4(pos, 1);
	// gl_Position = u_mvp * vec4(i_pos, 1);
}
