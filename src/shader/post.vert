#version 310 es

precision highp float;

layout(location=0) in vec2 in_pos;

out vec2 pass_tex_coords;

void main()
{
	pass_tex_coords = vec2(in_pos.x / 2.0f + .5f, in_pos.y / 2.0f + .5f);
	
	gl_Position = vec4(in_pos, 0, 1);
}