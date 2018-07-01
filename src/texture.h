#pragma once

#include "gl.h"

namespace SK
{
  class Texture
  {
  public:
    Texture(unsigned int w, unsigned int h, int* pixels);
    ~Texture();

    void bind(unsigned int index = 0);
    
  private:
    unsigned int _id;
  };
}
