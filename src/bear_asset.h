enum AssetType
{
	// Bear Asset Type. I'm a genious.
	BAT_MESH,
	BAT_SOUND,
	BAT_IMAGE,
	BAT_FONT,
};

struct AssetTag
{
	char upper[32];
	char lower[32];
};

// First in file
struct AssetFileHeader
{
	// TODO: Maybe store the time in here? 
	// TODO: Maybe a hash
	// Should allways be "BEAR", or 0x42454152 
	// if viewed as an int.
	uint32 file_code; 
	// The current version of the file format. So we 
	// don't load something we can't support
	uint32 version; 
	uint32 num_assets;
};

struct Vertex
{
	float32 x, y, z;
	float32 nx, ny, nz;
	float32 u, v;
};

// List of these is after the file header
struct Asset
{
	AssetType type;
	// TODO: Tag goes here.
	AssetTag tag;
	uint64 data_size;
	uint64 data_offset;
	void *data;
	union
	{
		struct 
		{
			int32 width, height;
			int32 color_depth;
		} image;
		struct
		{
			int32 channels, bitdepth;
			int32 sample_rate;
			int32 num_samples;
		} sound;
		struct
		{
			int32 num_verticies;
			Vertex *verticies;
			int32 num_indicies;
			uint32 *indicies;
		} mesh;
		// TODO Font.
	};
};

struct FileData
{
	union
	{
		char *file_ending;
		char *file_name;
	};
	AssetType type;
};


