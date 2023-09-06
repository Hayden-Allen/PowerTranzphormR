#version 330 core
layout(location = 0) out vec4 o_color;

uniform vec3 u_cam_pos, u_cam_dir;

in vec3 v_pos;

void main()
{
	vec3 N = normalize(cross(dFdx(v_pos), dFdy(v_pos)));
	vec3 L = normalize(vec3(1, 1, 1));
	float NdL = max(0, dot(N, L));

	vec3 R = reflect(L, N);
	vec3 V = -normalize(u_cam_dir);
	float VdR = max(0, dot(V, R));
	float spec = pow(VdR, 1);

	vec3 color = vec3(.1, .3, .8);
	o_color = vec4(clamp((NdL + .2) * color + vec3(spec), vec3(0), vec3(1)), .75);
}