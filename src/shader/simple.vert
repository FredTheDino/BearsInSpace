#version 310 es

precision highp float;

uniform mat4 m_view;
uniform mat4 m_model;

layout(location=0) in vec3 in_pos;
layout(location=1) in vec2 in_tex_coords;

out vec2 pass_tex_coords;

void main()
{
	pass_tex_coords = in_tex_coords;
	gl_Position = m_view * m_model * vec4(in_pos, 1);
}