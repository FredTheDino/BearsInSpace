#pragma pack( push, 1 ) // If we don't have this, compilers can padd things like they want.
enum AssetType
{
	// Bear Asset Type. I'm a genious.
	BAT_MESH	= 0b000001,
	BAT_SOUND	= 0b000010,
	BAT_IMAGE	= 0b000100,
	BAT_FONT	= 0b001000,
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

// Vertex 

struct Vertex
{
	float32 x, y, z;
	float32 u, v;
	float32 nx, ny, nz;
};

// Font stuff
// TODO: Mapping function, since the .fnt file should have the same set of chars in it, we can 
// do a simple transform to change from chars to indicies in the list. Saving us space.
struct Glyph
{
	char id;
	float32 u, v;
	float32 x, y;
	float32 w, h;
	float32 x_advance;
};

struct Kerning
{
	char first;
	char second;
	float32 amount;
};

// List of these is after the file header
struct AssetHeader
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
			uint32 *indices;
		} mesh;
		struct 
		{
			int32 width, height;
			int32 color_depth;
			int8 *image;

			uint64 max_glyph;
			uint64 num_kernings;
			Kerning *kernings;
			uint64 num_glyphs;
			Glyph *glyphs;
		} font;
	};
};

#pragma pack( pop )

struct FileData
{
	union
	{
		char *file_ending;
		char *file_name;
	};
	AssetType type;
};

