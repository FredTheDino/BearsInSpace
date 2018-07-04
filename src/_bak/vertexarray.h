#pragma once

#include "vertexbuffer.h"
#include "indexbuffer.h"

namespace SK
{
  struct VertexAttribute
  {
    VertexBuffer* vbo;
    unsigned int location;
    unsigned int size;
    unsigned int type;
    bool normalized    = false;
    int stride         = 0;
    const void* offset = NULL;
  };
  
  class VertexArray
  {
  public:
    VertexArray(std::vector<VertexAttribute>, IndexBuffer* indices=nullptr);
    ~VertexArray();
    
    void bind();

    unsigned int getID() { return _id; }

    bool hasIndices() { return _has_indices; }
    
  private:
    unsigned int _id;
    bool _has_indices = false;
  };
}
