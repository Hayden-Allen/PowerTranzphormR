#version 430 core
layout(location = 0) out vec4 o_col;
uniform sampler2D u_tex;
uniform vec3 u_cam_pos;
uniform mat4 u_m;
in vec2 v_tex;
in float v_NdL;
in vec3 v_pos, v_N;
void main()
{
	vec3 world_pos = vec3(u_m * vec4(v_pos, 1));
	vec3 V = normalize(world_pos - u_cam_pos);
	vec3 L = normalize(vec3(1, 1, 1));
	vec3 R = normalize(reflect(L, v_N));
	float RdV = pow(max(0, dot(V, R)), 16);
	
	vec3 diff = v_NdL * texture(u_tex, v_tex).xyz;
	vec3 spec = vec3(RdV);
	o_col = vec4(clamp(diff + spec, vec3(0), vec3(1)), 1);
}