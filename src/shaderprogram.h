#pragma once

#include <vector>
#include <iostream>
#include "gl.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "io.h"

namespace SK
{
  struct Shader
  {
    unsigned int type;
    const char* path;
  };
  
  class ShaderProgram
  {
  public:
    ShaderProgram(std::vector<Shader>);
    ~ShaderProgram();

    void bind();

    int location(const char*);
    
    void sendM4(const char*, glm::mat4);

  private:
    unsigned int _id;
  };
}
