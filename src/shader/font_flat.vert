#version 310 es

precision highp float;

layout(location=0) in vec2 in_pos;
layout(location=1) in vec2 in_tex_coords;

out vec2 pass_tex_coords;

void main()
{
	pass_tex_coords = in_tex_coords;
	
	gl_Position = vec4(in_pos, 0, 1);
}