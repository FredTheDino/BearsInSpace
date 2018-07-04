#pragma once

#include <fstream>
#include <sstream>

namespace SK
{
  class IO
  {
  public:
    static std::string read(const char* path)
    {
      std::ifstream t(path);
      std::stringstream buffer;
      buffer << t.rdbuf();

      return buffer.str();
    }
  };
}
