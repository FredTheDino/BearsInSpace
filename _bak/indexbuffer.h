#pragma once

#include <iostream>
#include <vector>
#include "gl.h"

namespace SK
{
  class IndexBuffer
  {
  public:
    IndexBuffer(std::vector<unsigned int>, unsigned int usage=GL_STATIC_DRAW);
    ~IndexBuffer();

    void bind();

    unsigned int getID() { return _id; }

  private:
    unsigned int _id = 0;
  };
}
