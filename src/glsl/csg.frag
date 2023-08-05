#version 430 core
layout(location = 0) out vec4 o_col;
uniform vec3 u_col;
uniform sampler2D u_tex;
in vec4 v_pos;
in vec2 v_tex;
void main()
{
	// gl_FragDepth = -v_pos.z;
	o_col = vec4(u_col * texture(u_tex, v_tex).xyz, 1);
	// o_col = vec4(1, 0, 0, 1);
}