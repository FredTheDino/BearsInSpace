#version 310 es

precision highp float;

uniform sampler2D t_sampler;

in vec2 pass_tex_coords;

layout(location=0) out vec4 out_color;

void main()
{
	float alpha_val = smoothstep(0.47, 0.50, texture(t_sampler, pass_tex_coords).r);
	
	out_color = vec4(1.0, 1.0, 1.0, alpha_val);
	//out_color = vec4(1.0, 1.0, 1.0, texture(t_sampler, pass_tex_coords).r);
}