#version 310 es

precision highp float;

uniform sampler2D sampler;

in vec3 pass_normal;
in vec2 pass_uv;

layout(location=0) out vec4 out_color;

void main()
{
  vec3 light = normalize(vec3(.5f, -1, -.5f));
  out_color = max(dot(pass_normal, -light), .0f) * texture(sampler, pass_uv);
}