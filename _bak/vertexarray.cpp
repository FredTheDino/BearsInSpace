#include "vertexarray.h"

using namespace SK;

VertexArray::VertexArray(std::vector<VertexAttribute> attribs, IndexBuffer* indices)
{
  // Generate vao
  glGenVertexArrays(1, &_id);

  bind();

  if (indices != nullptr)
  {
    _has_indices = true;
    indices->bind();
  }
  
  for (const auto& attr : attribs)
  {
    attr.vbo->bind();
    glEnableVertexAttribArray(attr.location);
    glVertexAttribPointer(attr.location, attr.size, attr.type, attr.normalized ? GL_TRUE : GL_FALSE, attr.stride, attr.offset);
  }

  glBindVertexArray(0);
}

VertexArray::~VertexArray()
{
  glDeleteVertexArrays(1, &_id);
}

void VertexArray::bind()
{
  glBindVertexArray(_id);
}
