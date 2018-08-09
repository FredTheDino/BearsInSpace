enum AssetTypes
{
	// Bear Asset Type. I'm a genious.
	BAT_OBJ,
	BAT_WAV,
	BAT_PNG,
};

struct Asset
{
	AssetType type;
	uint32 id;
	// Data
	union
	{
		Mesh mesh;
		Sound sound;
		Texture texture;
	};
};

void initalize_endings()
{
	supported_endings = create_array<FileData>(50);
#define FILE_ENDING(type, ending) append(&supported_endings, {(char *) (ending), type})
	FILE_ENDING(FT_IMAGE, "png");
	FILE_ENDING(FT_SOUND, "wav");
	FILE_ENDING(FT_MESH, "obj");
	FILE_ENDING(FT_FONT, "ttf");
}


