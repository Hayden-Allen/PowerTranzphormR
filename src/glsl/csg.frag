#version 430 core
layout(location = 0) out vec4 o_col;

uniform sampler2D u_tex;
uniform vec3 u_cam_pos;
uniform mat4 u_m;

in vec2 v_tex;
in vec3 v_pos, v_N;
in float v_NdL;

void main()
{
	vec3 world_pos = vec3(u_m * vec4(v_pos, 1));
	vec3 V = normalize(world_pos - u_cam_pos);
	vec3 L = normalize(u_cam_pos);
	vec3 R = normalize(reflect(L, v_N));
	float RdV = pow(max(0, dot(V, R)), 16);

	vec4 spec = vec4(RdV * vec3(1), 1);
	vec4 diff = min(1, v_NdL + .33) * texture(u_tex, v_tex);
	o_col = clamp(diff + spec, vec4(vec3(0), 1), vec4(1));

	// o_col = texture(u_tex, v_tex);
	// o_col = vec4(abs(v_N), 1);
	// vec3 t = dFdx(world_pos);
	// vec3 bt = dFdy(world_pos);
	// vec3 normal = normalize(cross(t, bt));
	// o_col = vec4(abs(normal), 1);
}