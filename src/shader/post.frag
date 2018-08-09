#version 310 es

precision highp float;

uniform sampler2D t_sampler;

in vec2 pass_tex_coords;

out vec4 out_color;

void main()
{
	vec4 color = texture(t_sampler, pass_tex_coords);
	
	out_color = vec4(color.z, color.x, color.y, 1.0f);
}