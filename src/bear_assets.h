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
	uint32 AssetID;
	// Data
	union
	{
		Mesh mesh;
		Sound sound;
		Texture texture;
	};
};

