#pragma once

#include "transform.h"

namespace SK
{
  class Camera : public Transform
  {
  public:
    
    void perspective(float fov, float aspect_ratio, float z_near, float z_far)
    {
      projection(glm::perspective(fov, aspect_ratio, z_near, z_far));
    }

    void ortho(float left, float right, float bottom, float top)
    {
      projection(glm::ortho(left, right, bottom, top));
    }
    
    void projection(glm::mat4 projection)
    {
      _projection = projection;
    } 
    
    glm::mat4 matrix()
    {
      return _projection * glm::inverse(Transform::matrix());
    }

  private:
    glm::mat4 _projection = glm::mat4(1.0);
  };
}
