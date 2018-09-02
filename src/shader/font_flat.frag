#version 310 es

precision highp float;

uniform sampler2D t_sampler;

in vec2 pass_tex_coords;

layout(location=0) out vec4 out_color;

uniform float text_edge;
uniform vec3 text_color;

void main()
{
	float dist = texture(t_sampler, pass_tex_coords).a;
	float alpha_val = smoothstep(0.49 - text_edge, 0.50, dist);
	
	if (alpha_val == 0.0)
		discard;

	out_color = vec4(text_color, alpha_val);
}
