#version 430 core
layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec2 i_tex;
uniform mat4 u_mvp, u_m;
uniform sampler2D u_heightmap;
out vec3 v_world_pos;
out vec2 v_tex;
void main()
{
	v_tex = i_tex;
	vec4 pos = vec4(i_pos.x, (1 - texture(u_heightmap, i_tex).r) * 10, i_pos.z, 1);
	gl_Position = u_mvp * pos;
	v_world_pos = vec3(u_m * pos);
}
