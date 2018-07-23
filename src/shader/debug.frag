#version 310 es

precision highp float;

uniform vec3 v_color;

layout(location=0) out vec4 out_color;

void main()
{
	out_color = vec4(v_color, 1);
}
