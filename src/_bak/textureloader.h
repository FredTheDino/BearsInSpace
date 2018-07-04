#pragma once

#include <string>
//#include <FreeImage.h>

#include "texture.h"

/*namespace SK
{
  class TextureLoader
  {
  public:

    static Texture* load(const char* path)
    {
      FREE_IMAGE_FORMAT format = FreeImage_GetFileType(path, 0);
      FIBITMAP* image = FreeImage_Load(format, path);

      FIBITMAP* temp = image;
      image = FreeImage_ConvertTo32Bits(image);
      FreeImage_Unload(temp);

      int w = FreeImage_GetWidth(image);
      int h = FreeImage_GetHeight(image);

      int* pixels = (int*) FreeImage_GetBits(image);

      Texture* tex = new Texture(w, h, pixels);

      FreeImage_Unload(image);

      return tex;
    }
  };
}*/
