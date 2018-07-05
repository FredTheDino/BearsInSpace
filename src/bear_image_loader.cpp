#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
// TODO: This has some stdlib stuff. Like assert and malloc.
#include "stb_image.h"

struct Image
{
	int32 width, height;
	int32 channels;
	uint8 *data;
};

Image load_image(const char *path)
{
	OSFile file = world->plt.read_file(path);
	Image img;
	img.data = stbi_load_from_memory((uint8 *) file.data, file.size, &img.width, &img.height, &img.channels, 4);
	world->plt.free_file(file);
	return img;
}

void free_image(Image img)
{
	stbi_image_free(img.data);
}

