#version 430 core
layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec3 i_norm;
layout (location = 2) in vec2 i_uv0;
layout (location = 3) in vec2 i_uv1;
layout (location = 4) in vec2 i_uv2;
layout (location = 5) in vec2 i_uv3;
layout (location = 6) in vec4 i_weights;
layout (location = 7) in vec4 i_rgba;

uniform mat4 u_mvp, u_m, u_normal;
uniform vec3 u_cam_pos;

out vec2 v_uv0, v_uv1, v_uv2, v_uv3;
out vec4 v_weights, v_rgba;
out vec3 v_N, v_pos;
out float v_NdL;

void main()
{
	gl_Position = u_mvp * vec4(i_pos, 1);
	
	// vec3 L = normalize(u_cam_pos);
	vec3 world_pos = vec3(u_m * vec4(i_pos, 1));
	vec3 L = normalize(u_cam_pos - world_pos);
	v_NdL = max(0, dot(i_norm, L));

	v_uv0 = i_uv0;
	v_uv1 = i_uv1;
	v_uv2 = i_uv2;
	v_uv3 = i_uv3;
	v_weights = i_weights;
	v_rgba = i_rgba;
	v_N = i_norm;
	v_pos = i_pos;
}
