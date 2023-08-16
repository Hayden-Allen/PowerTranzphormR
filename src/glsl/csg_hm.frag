#version 430 core
layout(location = 0) out vec4 o_col;
uniform sampler2D u_tex, u_heightmap;
in vec3 v_world_pos;
in vec2 v_tex;
void main()
{
	vec3 L = normalize(vec3(1, 1, 1));
	vec3 N = normalize(cross(normalize(dFdx(v_world_pos)), normalize(dFdy(v_world_pos))));
	float NdL = dot(N, L);
	o_col = vec4(NdL * texture(u_tex, v_tex).xyz, 1);
	// o_col = vec4(texture(u_heightmap, v_tex).xyz, 1);
}