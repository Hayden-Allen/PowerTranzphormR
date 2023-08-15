#version 430 core
layout(location = 0) out vec4 o_col;
uniform sampler2D u_tex, u_heightmap;
in vec2 v_tex;
void main()
{
	o_col = vec4(texture(u_tex, v_tex).xyz, 1);
	// o_col = vec4(texture(u_heightmap, v_tex).xyz, 1);
}