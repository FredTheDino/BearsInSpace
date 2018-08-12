#include "bear_asset.h"

string data_dir = "res/data.bear";

enum AssetState
{
	BAS_UNLOADED = 0,
	BAS_LOADING,
	BAS_REDIN,
	BAS_LOADED
};

struct AssetLoadCommand
{
	void *addrs;
	uint64 asset_id;
	uint64 start;
	uint64 size;
};

struct AssetManager
{
	AssetFileHeader header;
	AssetState *loaded_states;
	Asset *assets;
} am;

void start_asset_loader()
{
	plt.random_file_read(data_dir, &am.header, 0, sizeof(AssetFileHeader));
	ASSERT(am.header.file_code == 0x42454152); // BEAR in ASCII
	ASSERT(am.header.version == 1);

	uint32 assets_size = sizeof(Asset) * am.header.num_assets;
	am.assets = static_push_array(Asset, am.header.num_assets);
	plt.random_file_read(data_dir, am.assets, sizeof(AssetFileHeader), assets_size);

	am.loaded_states = static_push_array(AssetState, am.header.num_assets);
	for (uint32 i = 0; i < am.header.num_assets; i++)
	{
		am.loaded_states[i] = BAS_UNLOADED;
	}
}

void _read_in_asset(void *_arg)
{
	AssetLoadCommand *cmd = (AssetLoadCommand *) _arg;
	plt.random_file_read(data_dir, cmd->addrs, cmd->start, cmd->size);
	am.loaded_states[cmd->asset_id] = BAS_REDIN;
}

// 
// TODO: Write the platform threadpool with SDL.
//

void load_asset(uint32 asset_id)
{
	AssetState *state = am.loaded_states + asset_id;
	
	if (*state == BAS_UNLOADED)
	{
		*state = BAS_LOADED;

		Asset *asset = &am.assets[asset_id];
		uint64 alloc_size = asset->data_size + sizeof(AssetLoadCommand);
		uint8 *space = (uint8 *) static_push(alloc_size);
		asset->data = (void *) space;

		AssetLoadCommand *cmd = (AssetLoadCommand *)(space + asset->data_size);
		*cmd = {(void *) space, asset_id, asset->data_offset, asset->data_size};
		Work work = {_read_in_asset, cmd};

		plt.submit_work(work);
	}
}

void unload_asset(uint32 asset_id)
{
	AssetState *state = am.loaded_states + asset_id;
	
	if (*state == BAS_LOADED)
	{
		*state = BAS_UNLOADED;
		static_pop(am.assets[asset_id].data);
	}
}

void update_assets() // Create the assets here since this is called on the main thread.
{
	// TODO: Make this better, currently we're looping through each asset, this is 
	// dumb, we should have a queue here too. But it's a minor penalty for ease of writing.
	for (uint32 asset_id = 0; asset_id < am.header.num_assets; asset_id++)
	{
		if (am.loaded_states[asset_id] == BAS_REDIN)
		{
			// Process each asset type so it's set up correctly.
			am.loaded_states[asset_id] = BAS_LOADED;
		}
	}
}




