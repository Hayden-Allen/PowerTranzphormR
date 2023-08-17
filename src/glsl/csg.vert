#version 430 core
layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec2 i_tex;
layout (location = 2) in vec3 i_norm;

uniform mat4 u_mvp, u_m, u_normal;
uniform vec3 u_cam_pos;

out vec2 v_tex;
out vec3 v_N, v_pos;
out float v_NdL;

void main()
{
	gl_Position = u_mvp * vec4(i_pos, 1);
	
	vec3 L = normalize(vec3(1, 1, 1));
	v_NdL = max(0, dot(i_norm, L));

	v_tex = i_tex;
	v_N = i_norm;
	v_pos = i_pos;
}
