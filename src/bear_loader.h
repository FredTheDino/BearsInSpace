#pragma once
#include "bear_asset.h"

enum AssetState
{
	BAS_UNLOADED = 0,
	BAS_LOADING,
	BAS_REDIN,
	BAS_LOADED
};

typedef int32 AssetID;

struct Asset
{
	bool valid;
	union
	{
		GFX::VertexArray mesh_vao;
		GFX::Texture texture;
		AudioID buffer_id;
	};
};
Asset default_mesh, default_image;

struct AssetManager
{
	AssetFileHeader file_header;
	AssetState *loaded_states;
	AssetHeader *headers;
	Asset *assets;
} am;

struct AssetLoadCommand 
{
	void *addrs;
	AssetID asset_id;
	uint64 start;
	uint64 size;
};

void load_asset(int32 asset_id);
void unload_asset(uint32 asset_id);
void unload_all_assets();

bool is_loading();

//
// For all get_* functions the upper and lower tags
// can be set to NULL, which will match anything.
//
// Returns the first, -1 if not found.
AssetID get_asset_id(AssetType type, const char *upper=0, const char *lower=0);
// Returns ALL the matching IDs.
Array<AssetID> get_asset_ids(AssetType type, const char *upper=0, const char *lower=0);
// Returns the actual asset the game can use.
Asset get_asset(int32 asset_id);

