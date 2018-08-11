#include "bear_asset.h"

void asset_loop(void *);

struct AssetQueue
{
	// TODO:
};

struct AssetManager
{
	bool running;
	Thread thread;
	AssetFileHeader header;
	Asset *assets;

} asset_manager;

void start_asset_loader()
{
	//asset_manager.thread = plt.create_thread(asset_loop, 0);
	asset_manager.running = true;

	plt.random_file_read("res/data.bear", &asset_manager.header, 0, sizeof(AssetFileHeader));
	ASSERT(asset_manager.header.file_code == 0x42454152); // BEAR in ASCII
	ASSERT(asset_manager.header.version == 1);

	uint32 assets_size = sizeof(Asset) * asset_manager.header.num_assets;
	asset_manager.assets = static_push_array(Asset, asset_manager.header.num_assets);
	plt.random_file_read("res/data.bear", asset_manager.assets, sizeof(AssetFileHeader), assets_size);
	// Read in header
	// Read in asset headers
}

void asset_loop(void *_) // Ignore this, it's just to it can be called.
{
	// 
}






