#version 310 es

precision highp float;

uniform sampler2D t_sampler;

layout(location=0) out vec4 out_color;

in vec2 pass_tex_coords;

void main()
{
	vec4 temp = texture(t_sampler, pass_tex_coords);

	out_color = temp;
}
