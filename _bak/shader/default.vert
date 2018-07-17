#version 310 es

precision highp float;

uniform mat4 m_view;
uniform mat4 m_model;

layout(location=0) in vec3 in_pos;
layout(location=1) in vec2 in_uv;
layout(location=2) in vec3 in_normal;

out vec3 pass_normal;
out vec2 pass_uv;

void main()
{
  pass_normal = in_normal;
  pass_uv = in_uv;
  gl_Position = m_view * m_model * vec4(in_pos, 1);
}