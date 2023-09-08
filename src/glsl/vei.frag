#version 330 core
layout(location = 0) out vec4 o_color;

uniform vec3 u_cam_pos, u_cam_dir;
uniform float u_time;

in vec3 v_pos, v_norm, v_pos_ndc;

vec3 hsl2rgb(in vec3 c)
{
    vec3 rgb = clamp(abs(mod(c.x * 6 + vec3(0, 4, 2), 6) - 3) - 1, 0, 1);

    return c.z + c.y * (rgb - .5) * (1 - abs(2 * c.z - 1));
}

void main()
{
	vec3 N = normalize(cross(dFdx(v_pos), dFdy(v_pos)));
	vec3 L = normalize(v_pos - u_cam_pos);
	float NdL = max(0, dot(N, -L));

	vec3 V = normalize(v_pos - u_cam_pos);
	vec3 R = normalize(reflect(L, normalize(v_norm)));
	float spec = pow(max(0, dot(V, R)), 16);

	// vec3 color = vec3(.1, .3, .9);
	vec3 color = hsl2rgb(vec3(cos(-u_time * .5 + (cos(u_time * .25) * v_pos_ndc.x + sin(u_time * .25) * v_pos_ndc.y) * 3), .85, .6));
	// vec3 spec_color = vec3(.5, .7, 1);
	vec3 spec_color = hsl2rgb(vec3(sin(u_time), .75, .7));
	o_color = vec4(clamp((NdL + .2) * color + spec * spec_color, vec3(0), vec3(1)), min(1, .6 + spec));
}