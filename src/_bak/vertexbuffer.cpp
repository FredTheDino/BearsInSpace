#include "vertexbuffer.h"

using namespace SK;

VertexBuffer::VertexBuffer(std::vector<float> data, unsigned int usage)
{
  glGenBuffers(1, &_id);
  
  glBindBuffer(GL_ARRAY_BUFFER, _id);

  glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], usage);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::~VertexBuffer()
{
  glDeleteBuffers(1, &_id);
}

void VertexBuffer::bind()
{
  glBindBuffer(GL_ARRAY_BUFFER, _id);
}
