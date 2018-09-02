#version 310 es

precision highp float;

uniform sampler2D t_sampler;

in vec2 pass_tex_coords;

out vec4 out_color;

void main()
{
	float dist = .0f;
	//float dist = 0.005f;
	
	vec4 color = vec4(.0f, .0f, .0f, .0f);
	color += texture(t_sampler, pass_tex_coords) * 0.1964825501511404f;
	color += texture(t_sampler, pass_tex_coords + vec2(dist, .0f)) * 0.2969069646728344f / 2.0f;
	color += texture(t_sampler, pass_tex_coords - vec2(dist, .0f)) * 0.2969069646728344f / 2.0f;
	color += texture(t_sampler, pass_tex_coords + vec2(.0f, dist)) * 0.2969069646728344f / 2.0f;
	color += texture(t_sampler, pass_tex_coords - vec2(.0f, dist)) * 0.2969069646728344f / 2.0f;
	color += texture(t_sampler, pass_tex_coords + vec2(dist, dist)) * 0.09447039785044732f / 2.0f;
	color += texture(t_sampler, pass_tex_coords - vec2(-dist, dist)) * 0.09447039785044732f / 2.0f;
	color += texture(t_sampler, pass_tex_coords + vec2(dist, dist)) * 0.09447039785044732f / 2.0f;
	color += texture(t_sampler, pass_tex_coords - vec2(-dist, dist)) * 0.09447039785044732f / 2.0f;
	color += texture(t_sampler, pass_tex_coords + vec2(2.0f * dist, .0f)) * 0.010381362401148057f / 2.0f;
	color += texture(t_sampler, pass_tex_coords - vec2(2.0f * dist, .0f)) * 0.010381362401148057f / 2.0f;
	color += texture(t_sampler, pass_tex_coords + vec2(.0f, 2.0f * dist)) * 0.010381362401148057f / 2.0f;
	color += texture(t_sampler, pass_tex_coords - vec2(.0f, 2.0f * dist)) * 0.010381362401148057f / 2.0f;

	out_color = color;
}