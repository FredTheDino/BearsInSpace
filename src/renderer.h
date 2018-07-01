#pragma once

#include <unordered_map>
#include <string>

#include "vertexarray.h"
#include "shaderprogram.h"
#include "transform.h"
#include "texture.h"

namespace SK
{
  struct Renderable
  {
    VertexArray* vertex_array;
    ShaderProgram* program;
    unsigned int num_vertices;
    Texture* texture;
    Transform* transform;
    std::string uniform;
  };
  
  class Renderer
  {
  public:
    static void draw(Renderable r, unsigned int mode=GL_TRIANGLES, bool use_profiles=true)
    {
      r.program->bind();
      
      if (use_profiles)
	for (auto mp : _matrix_profiles)
	  r.program->sendM4(mp.first.c_str(), mp.second->matrix());
      
      if (r.texture != nullptr)
	r.texture->bind();
      
      r.program->sendM4(r.uniform.c_str(), r.transform->matrix());
      
      r.vertex_array->bind();
      
      if (r.vertex_array->hasIndices())
	glDrawElements(mode, r.num_vertices, GL_UNSIGNED_INT, NULL);
      else
	glDrawArrays(mode, 0, r.num_vertices);
    }

    static void matrixProfile(std::string uniform, Transform* transform)
    {
      _matrix_profiles.emplace(uniform, transform);
    }

  private:
    static std::unordered_map<std::string, Transform*> _matrix_profiles;
  };
}
