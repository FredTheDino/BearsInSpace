#include "indexbuffer.h"

using namespace SK;

IndexBuffer::IndexBuffer(std::vector<unsigned int> data, unsigned int usage)
{
  glGenBuffers(1, &_id);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), &data[0], usage);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer()
{
  glDeleteBuffers(1, &_id);
}

void IndexBuffer::bind()
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);
}
