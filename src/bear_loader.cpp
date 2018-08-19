#include "bear_loader.h"

const string data_dir = "res/data.bear";

// Helper function for threads
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
	AssetState *state = &am.loaded_states[asset_id];
	AssetHeader *header = &am.headers[asset_id];
	
	if (*state == BAS_LOADED)
	{
		*state = BAS_UNLOADED;
		switch (header->type)
		{
			case (BAT_SOUND):
				{
					static_pop(am.headers[asset_id].data);
				}
			case (BAT_MESH):
				{
					LOG("LOADER ERROR", "Don't know how to free meshes... Sorry...\n");
					break;
				}	
			case (BAT_IMAGE):
				{
					LOG("LOADER ERROR", "Don't know how to free textures... Sorry...\n");
					break;

				}
			default:
				PRINT("Trying to free unrecognized Asset Type (%d)\n", header->type);
		}
		*state = BAS_UNLOADED;
	}
	if (*state == BAS_REDIN)
	{
		// Here we've only read it in. So we just free and we're done.
		static_pop(header->data);
		*state = BAS_UNLOADED;
	}
}

void unload_all_assets()
{
	for (uint32 i = 0; i < am.file_header.num_assets; i++)
	{
		unload_asset(i);
	}
}

// Returns the first, -1 if not found.
AssetID get_asset_id(uint32 type, const char *upper, const char *lower)
{
	// Maybe we should use a hash later.
	for (uint32 i = 0; i < am.file_header.num_assets; i++)
	{
		AssetHeader *header = &am.headers[i];
		if (type && !(header->type & type))
			continue;
		if (upper && !(str_eq(header->tag.upper, upper)))
			continue;
		if (lower && !(str_eq(header->tag.lower, lower)))
			continue;
		return i;
	}
	return -1;
}

// Returns ALL the matching IDs.
Array<AssetID> get_asset_ids(uint32 type, const char *upper, const char *lower)
{
	Array<AssetID> result = temp_array<AssetID>(20);
	for (AssetID i = 0; i < (AssetID) am.file_header.num_assets; i++)
	{
		AssetHeader *header = &am.headers[i];
		if (type && !(header->type & type))
			continue;
		if (upper && !(str_eq(header->tag.upper, upper)))
			continue;
		if (lower && !(str_eq(header->tag.lower, lower)))
			continue;
		append(&result, i);
	}
	return result;
}

AssetType get_type(AssetID id)
{
	return am.headers[id].type;
}

Asset get_asset(AssetID asset_id)
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

// Returns the first match.
Asset get_asset(AssetType type, const char *upper=0, const char *lower=0)
{
	return get_asset(get_asset_id(type, upper, lower));
}

bool is_loading()
{
	for (uint32 asset_id = 0; asset_id < am.file_header.num_assets; asset_id++)
	{
		AssetState state = am.loaded_states[asset_id];
		if (state == BAS_LOADING)
		{
			return true;	
		}
	}
	return false;
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
						attributes[0] = {buffer, 0, 3, GL_FLOAT, false, v_size, (void *) (0 * f_size)};
						attributes[1] = {buffer, 1, 2, GL_FLOAT, false, v_size, (void *) (3 * f_size)};
						attributes[2] = {buffer, 2, 3, GL_FLOAT, false, v_size, (void *) (5 * f_size)};
						asset.vao = GFX::create_vertex_array(attributes, 3, index);
						asset.draw_length = header->mesh.num_indicies;
						break;
					}
				case(BAT_IMAGE):
					{
						asset.texture = GFX::create_texture(
								header->image.width, header->image.height, 
								header->image.color_depth, data);
						static_pop(data);
						break;
					}
				case(BAT_SOUND):
					{
						AudioBuffer buffer;
						buffer.channels = header->sound.channels;
						buffer.bitdepth = header->sound.bitdepth;
						buffer.sample_rate = header->sound.sample_rate;
						buffer.num_samples = header->sound.num_samples;
						buffer.data = header->data;
						// TODO: This is dumb...
						add_buffer(&world->audio, buffer);
					}
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

void start_loader()
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
		am.loaded_states[i] = BAS_UNLOADED;
	}
	am.assets = static_push_array(Asset, am.file_header.num_assets);

	// Load default assets
	AssetID mesh_id = get_asset_id(BAT_MESH, "default", "mesh");
	AssetID img_id = get_asset_id(BAT_IMAGE, "default", "image");
	load_asset(mesh_id);
	load_asset(img_id);

	AssetState state;
	while (is_loading());

	update_assets();

	default_mesh = get_asset(mesh_id);
	default_mesh.valid = false;
	default_image = get_asset(img_id);
	default_image.valid = false;
}	

void stop_loader()
{
	// We have to wait for all the loading OPS to end.
	while (is_loading());
	unload_all_assets();

	static_pop(am.headers);
	static_pop(am.loaded_states);
	static_pop(am.assets);
}

