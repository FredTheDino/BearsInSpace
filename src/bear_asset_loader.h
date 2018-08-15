#include "bear_asset.h"

string data_dir = "res/data.bear";

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

void _read_in_asset(void *_arg)
{
	AssetLoadCommand *cmd = (AssetLoadCommand *) _arg;
	plt.random_file_read(data_dir, cmd->addrs, cmd->start, cmd->size);
	am.loaded_states[cmd->asset_id] = BAS_REDIN;
}

void load_asset(int32 asset_id)
{
	ASSERT(asset_id >= 0);
	AssetState *state = am.loaded_states + asset_id;
	
	if (*state == BAS_UNLOADED)
	{
		*state = BAS_LOADING;

		AssetHeader *header = &am.headers[asset_id];
		uint64 alloc_size = header->data_size + sizeof(AssetLoadCommand);
		uint8 *space = (uint8 *) static_push(alloc_size);
		header->data = (void *) space;

		AssetLoadCommand *cmd = (AssetLoadCommand *)(space + header->data_size);
		*cmd = {(void *) space, asset_id, header->data_offset, header->data_size};
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
		static_pop(am.headers[asset_id].data); // This can potentially be moved.
		// Do some more stuff.
	}

	
}

// Returns the first, -1 if not found.
AssetID get_asset_id(AssetType type, const char *upper=0, const char *lower=0)
{
	// Maybe we should use a hash later.
	for (uint32 i = 0; i < am.file_header.num_assets; i++)
	{
		AssetHeader *header = &am.headers[i];
		if (header->type != type)
			continue;
		if (upper && !(str_eq(header->tag.upper, upper)))
			continue;
		if (lower && !(str_eq(header->tag.lower, lower)))
			continue;
		return i;
	}
	return -1;
}

Asset get_asset(int32 asset_id)
{
	ASSERT(asset_id >= 0);
	ASSERT(asset_id < (int32) am.file_header.num_assets);

	if (am.loaded_states[asset_id] == BAS_LOADED)
	{
		Asset asset = am.assets[asset_id];
		return asset;
	}
	else
	{
		// We should return a default asset here. So we can see we're using something
		// That isn't yet loaded, and tell us to start loading it.
		load_asset(asset_id);
		AssetHeader header = am.headers[asset_id];
		Asset asset;
		switch (header.type)
		{
			case(BAT_MESH):
				asset = default_mesh;
				break;
			case(BAT_IMAGE):
				asset = default_image;
				break;
			case(BAT_SOUND):
				asset = {false, 0};
				break;
			default:
				HALT_AND_CATCH_FIRE();
				break;
		}
		return asset;
	}
}

Asset get_asset(AssetType type, const char *upper=0, const char *lower=0)
{
	return get_asset(get_asset_id(type, upper, lower));
}

void update_assets() // Create the assets here since this is called on the main thread.
{
	// TODO: Make this better, currently we're looping through each asset, this is 
	// dumb, we should have a queue here too. But it's a minor penalty for ease of writing.
	for (uint32 asset_id = 0; asset_id < am.file_header.num_assets; asset_id++)
	{
		if (am.loaded_states[asset_id] == BAS_REDIN)
		{
			// Process each asset type so it's set up correctly.
			am.loaded_states[asset_id] = BAS_LOADED;

			AssetHeader *header = &am.headers[asset_id];
			uint8 *data = (uint8 *) header->data;
			Asset asset;
			switch (header->type)
			{
				case(BAT_MESH):
					{
						header->mesh.verticies = (Vertex *) header->data;
						header->mesh.indicies = (uint32 *) (data + (sizeof(Vertex) * header->mesh.num_verticies));
						GFX::VertexBuffer buffer = GFX::create_vertex_buffer(
							(float32 *) header->mesh.verticies,
							header->mesh.num_verticies * 8);

						GFX::IndexBuffer index = GFX::create_index_buffer(
							(uint32 *) header->mesh.indicies, 
							header->mesh.num_indicies);
						static_pop(data);

						GFX::VertexAttribute attributes[3];
						const uint32 v_size = sizeof(Vertex);
						const uint32 f_size = sizeof(float32);
						// Vert: 
						// x, y, z
						// nx, ny, nz
						// u, v
						attributes[0] = {buffer, 0, 3, GL_FLOAT, false, v_size, (void *) (0 * f_size)};
						attributes[1] = {buffer, 2, 3, GL_FLOAT, false, v_size, (void *) (3 * f_size)};
						attributes[2] = {buffer, 1, 2, GL_FLOAT, false, v_size, (void *) (6 * f_size)};
						asset.mesh_vao = GFX::create_vertex_array(attributes, 3, index);
						break;
					}
				case(BAT_IMAGE):
					{
						Image img = {header->image.width, header->image.height, header->image.color_depth, data};
						asset.texture = GFX::create_texture(img);
						static_pop(data);
						break;
					}
				case(BAT_SOUND):
					break;
				default:
					HALT_AND_CATCH_FIRE();
					break;
			}
			asset.valid = true;
			am.assets[asset_id] = asset;
		}
	}
}

void start_asset_loader()
{
	plt.random_file_read(data_dir, &am.file_header, 0, sizeof(AssetFileHeader));
	ASSERT(am.file_header.file_code == 0x42454152); // BEAR in ASCII
	ASSERT(am.file_header.version == 1);

	uint32 assets_size = sizeof(AssetHeader) * am.file_header.num_assets;
	am.headers = static_push_array(AssetHeader, am.file_header.num_assets);
	plt.random_file_read(data_dir, am.headers, sizeof(AssetFileHeader), assets_size);

	am.loaded_states = static_push_array(AssetState, am.file_header.num_assets);
	for (uint32 i = 0; i < am.file_header.num_assets; i++)
	{
		PRINT("LOADED: %s.%s\n", am.headers[i].tag.lower, am.headers[i].tag.upper);
		am.loaded_states[i] = BAS_UNLOADED;
	}

	am.assets = static_push_array(Asset, am.file_header.num_assets);

	PRINT("Size of HEADER: %d\n", sizeof(AssetHeader));

	// Default assets:
	AssetID mesh_id = get_asset_id(BAT_MESH, "default", "mesh");
	AssetID img_id = get_asset_id(BAT_IMAGE, "default", "image");
	load_asset(mesh_id);
	load_asset(img_id);

	while (am.loaded_states[img_id] != BAS_REDIN);

	update_assets();

	default_mesh = get_asset(mesh_id);
	default_mesh.valid = false;
	default_image = get_asset(img_id);
	default_image.valid = false;
}	

