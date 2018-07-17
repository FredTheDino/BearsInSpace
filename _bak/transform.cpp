#include "transform.h"

using namespace SK;

glm::mat4 Transform::matrix()
{
  glm::mat4 m_t  = glm::translate(glm::mat4(1.0), position);
  glm::mat4 m_rx = glm::rotate(m_t,  rotation.x, glm::vec3(1, 0, 0));
  glm::mat4 m_ry = glm::rotate(m_rx, rotation.y, glm::vec3(0, 1, 0));
  glm::mat4 m_rz = glm::rotate(m_ry, rotation.z, glm::vec3(0, 0, 1));
  return  glm::scale(m_rz, scale);
}
