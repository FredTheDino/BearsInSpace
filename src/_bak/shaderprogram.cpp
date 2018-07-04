#include "shaderprogram.h"

using namespace SK;

ShaderProgram::ShaderProgram(std::vector<Shader> shaders)
{

  _id = glCreateProgram();
  
  std::vector<unsigned int> shader_ids;
  
  for (auto const& sh : shaders)
  {
    // Create shader
    shader_ids.push_back(glCreateShader(sh.type));

    // Shader source
    std::string src = SK::IO::read(sh.path);
    const char* src_raw = src.c_str();
    int len_src = src.length();
    glShaderSource(shader_ids.back(), 1, &src_raw, &len_src);

    // Compile shader
    glCompileShader(shader_ids.back());

    // Get compile status
    int sh_status = false;
    glGetShaderiv(shader_ids.back(), GL_COMPILE_STATUS, &sh_status);

    // Handle errors
    if(sh_status != GL_TRUE)
    {
      // Info log length
      int len = 0;
      glGetShaderiv(shader_ids.back(), GL_INFO_LOG_LENGTH, &len);

      // Get actual info log
      std::vector<char> log(len);
      glGetShaderInfoLog(shader_ids.back(), len, &len, &log[0]);

      std::cout << "======================" << std::endl;
      std::cout << "Failed to compile shader: \"" << sh.path << "\"" << std::endl;
      std::cout << std::string(&log[0]) << std::endl;
      std::cout << "======================" << std::endl;
    }

    // Attach shader
    glAttachShader(_id, shader_ids.back());
  }

  // Link program
  glLinkProgram(_id);

  // Check link status
  int pl_status = 0;
  glGetProgramiv(_id, GL_LINK_STATUS, &pl_status);

  // Handle errors
  if(pl_status != GL_TRUE)
  {
    // Info log length
    int len = 0;
    glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &len);
    
    // Get actual info log
    std::vector<char> log(len);
    glGetProgramInfoLog(_id, len, &len, &log[0]);

    std::cout << "======================" << std::endl;
    std::cout << "Failed to link program!" << std::endl;
    std::cout << std::string(&log[0]) << std::endl;
    std::cout << "======================" << std::endl;
  }

  // Validate program
  glValidateProgram(_id);

  // Check validation status
  int pv_status = 0;
  glGetProgramiv(_id, GL_VALIDATE_STATUS, &pv_status);

  // Handle errors
  if(pv_status != GL_TRUE)
  {
    // Info log length
    int len = 0;
    glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &len);
    
    // Get actual info log
    std::vector<char> log(len);
    glGetProgramInfoLog(_id, len, &len, &log[0]);

    std::cout << "======================" << std::endl;
    std::cout << "Program validation failed!" << std::endl;
    std::cout << std::string(&log[0]) << std::endl;
    std::cout << "======================" << std::endl;
  }
  
  // Detach shaders
  for (auto const& id : shader_ids)
    glDetachShader(_id, id);

  // Delete shaders
  for (auto const& id : shader_ids)
    glDeleteShader(id);
}

ShaderProgram::~ShaderProgram()
{
  glDeleteProgram(_id);
}

void ShaderProgram::bind()
{
  glUseProgram(_id);
}

int ShaderProgram::location(const char* name)
{
  return glGetUniformLocation(_id, name);
}

void ShaderProgram::sendM4(const char* u_name, glm::mat4 m4)
{
  return glUniformMatrix4fv(location(u_name), 1, GL_FALSE, glm::value_ptr(m4));
}
