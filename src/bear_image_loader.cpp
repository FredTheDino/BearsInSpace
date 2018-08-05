#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#define STBI_NO_STDIO
// TODO: This has some stdlib stuff. Like assert and malloc.
#include "stb_image.h"

struct Image
{
	int32 width, height;
	int32 channels;
	uint8 *data;
};

Image load_image(string path)
{
	OSFile file = plt.read_file(path, temp_push);
	Image img;
	img.data = stbi_load_from_memory((uint8 *) file.data, file.size, 
			&img.width, &img.height, &img.channels, 4);
	return img;
}

void free_image(Image *img)
{
	stbi_image_free(img->data);

	img->width = 0;
	img->height = 0;
	img->channels = 0;
	img->data = nullptr;
}

