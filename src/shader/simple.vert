#version 310 es

precision highp float;

uniform mat4 m_view;
uniform mat4 m_model;

layout(location=0) in vec2 in_pos;

void main()
{
  gl_Position = m_view * m_model * vec4(in_pos, 0, 1);
}