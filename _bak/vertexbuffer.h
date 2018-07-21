#pragma once

#include <iostream>
#include <vector>
#include "gl.h"

namespace SK
{
  class VertexBuffer
  {
  public:
    VertexBuffer(std::vector<float> data, unsigned int usage=GL_STATIC_DRAW);
    ~VertexBuffer();

    void bind();

    unsigned int getID() { return _id; }

  private:
    unsigned int _id = 0;
  };
}
