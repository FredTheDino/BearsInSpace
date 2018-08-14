
#define HALT_AND_CATCH_FIRE() ((int *)(void *)0)[0] = 1
#define ASSERT(expr) if (!(expr)) { HALT_AND_CATCH_FIRE(); }

#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "bear_types.h"
#include "bear_array.h"
#include "bear_array_plt.cpp"

#include "bear_wav_loader.cpp"
#include "bear_obj_loader.cpp"

#include "bear_asset.h"

Array<FileData> files;
Array<FileData> endings;

char _strings[2048];
char *write_head = (char *) _strings;
char *copy_string_to_storage(char *str)
{
	char *out = write_head;
	while (*str)
	{	
		*write_head++ = *str++;
	}
	write_head++;
	return out;
}

void copy_until(char *to, char *from, char until='\0')
{
	while (*from != until)
	{
		*to++ = *from++;
	}
	*to = '\0';
}

bool ends_with(const char *src, const char *ending)
{
	char *s = (char *) src;
	while (*s)
	{
		char *e = (char *) ending;
		char *s_copy = s;
		while (*s_copy == *e)
		{
			if (*s_copy == '\0')
				return true;
			s_copy++;
			e++;
		}
		s++;
	}
	return false;
}

Array<FileData> initalize_endings()
{
	auto endings = create_array<FileData>(50);
#define FILE_ENDING(type, ending) append(&endings, {(char *) (ending), type})
	FILE_ENDING(BAT_IMAGE, "png");
	FILE_ENDING(BAT_SOUND, "wav");
	FILE_ENDING(BAT_MESH, "obj");
	FILE_ENDING(BAT_FONT, "ttf");
	return endings;
}


void keep_writing(FILE *file, void *data, uint64 len)
{
	uint8 *read_head = (uint8 *) data;
	uint64 left_to_write = len;
	const uint64 buffer_size = 1000000;
	while (left_to_write)
	{
		uint64 write_pass = left_to_write > buffer_size ? buffer_size: left_to_write;
		auto ret = fwrite(read_head, 1, left_to_write, file);
		ASSERT(ret != 0);
		left_to_write -= ret;
		read_head += ret;
	}
}

void find_files_in_folder(const char *);
int main(int arg_len, char *args)
{
	endings = initalize_endings();
	files = create_array<FileData>(50);
	const char *resource_dir = "res"; // Pass in this value.
	find_files_in_folder(resource_dir);

	auto assets = create_array<AssetHeader>(50);
	printf("files found:\n");
	for (uint32 i = 0; i < size(files); i++)
	{
		FileData data = get(files, i);
		printf(" (%d)  %s\n", data.type, data.file_name);

		char file_path[100];
		sprintf(file_path, "%s/%s", resource_dir, data.file_name);

		AssetHeader header;
		header.type = data.type;

		char *file_path_ptr = file_path;
		while (*file_path_ptr != '/') file_path_ptr++;
		file_path_ptr++;
		copy_until(header.tag.upper, file_path_ptr, '.');
		while (*file_path_ptr != '.') file_path_ptr++;
		file_path_ptr++;
		copy_until(header.tag.lower, file_path_ptr, '.');

		switch (data.type) 
		{
			case (BAT_IMAGE):
				{
					header.data = (void *) stbi_load(file_path, 
							&header.image.width, &header.image.height, 
							&header.image.color_depth, 0);
					header.data_size = header.image.width * header.image.height * header.image.color_depth;
					printf("%s (image), %dx%dx%d, size: %d\n", file_path, 
							header.image.width, header.image.height, header.image.color_depth, header.data_size);
					break;
				}
			case (BAT_SOUND):
				{
					AudioBuffer buffer = load_wav(file_path, malloc);
					header.data = (void *) buffer.data;
					header.sound.channels = buffer.channels;
					header.sound.bitdepth = buffer.bitdepth;
					header.sound.sample_rate = buffer.sample_rate;
					header.sound.num_samples = buffer.num_samples;
					header.data_size = header.sound.num_samples * header.sound.bitdepth / 8;
					printf("%s (sound) chl %d, bit %d, rate %d, num %d\n", 
						file_path, header.sound.channels, header.sound.bitdepth, header.sound.sample_rate, 
						header.sound.num_samples);
					break;
				}
			case (BAT_MESH):
				{
					struct Indici
					{
						int32 p, n, u;
						bool operator== (Indici other) const 
						{
							return 
								p == other.p && 
								n == other.n && 
								u == other.u;
						}
					};

					Mesh mesh = load_mesh(file_path, malloc);
					Array<Vertex> out_verticies = create_array<Vertex>(100);
					Array<uint32> out_indicies = create_array<uint32>(100);
					Array<Indici> full  = create_array<Indici>(100);
					for (uint64 i = 0; i < size(mesh.indicies); i += mesh.stride)
					{
						int32 p_i;
						int32 u_i;
						int32 n_i;
						ASSERT(mesh.stride > 0);
						ASSERT(mesh.stride < 4);
						if (mesh.stride == 3)
						{
							p_i = get(mesh.indicies, i + 0) - 1;
							u_i = get(mesh.indicies, i + 1) - 1;
							n_i = get(mesh.indicies, i + 2) - 1;
						}
						else if (mesh.stride == 2)
						{
							p_i = get(mesh.indicies, i + 0) - 1;
							n_i = get(mesh.indicies, i + 1) - 1;
						}
						else if (mesh.stride == 1)
						{
							p_i = get(mesh.indicies, i + 0) - 1;
						}
						ASSERT(p_i >= 0);
						ASSERT(n_i >= 0);
						ASSERT(u_i >= 0);
						Indici full_indici = {p_i, n_i, u_i};
						int32 vertex_index = find(full, full_indici);
						if (vertex_index == -1)
						{
							// Didn't find it.
							append(&full, full_indici);
							Vec3f pos, nor;
							Vec2f uv;
							ASSERT(size(mesh.positions));
							pos = get(mesh.positions, p_i);
							if (size(mesh.normals))
								nor = normalize(get(mesh.normals, n_i));
							else
								nor = {};
							if (size(mesh.uvs))
								uv = get(mesh.uvs, u_i);
							else
								uv = {};

							Vertex vertex = 
							{
								pos.x, pos.y, pos.z,
								nor.x, nor.y, nor.z,
								uv.x, uv.y
							};
							append(&out_verticies, vertex); 
							append(&out_indicies, (uint32) sizeof(out_verticies));
						}
						else
						{
							append(&out_indicies, (uint32) vertex_index);
						}
					}
					free_mesh(mesh);
					header.mesh.num_verticies = size(out_verticies);
					header.mesh.verticies = out_verticies.data;
					header.mesh.num_indicies = size(out_indicies);
					header.mesh.indicies = out_indicies.data;
					break;
				}
			default:
				printf("ERROR Unsupported type %d\n", data.type);
		}
		append(&assets, header);
	}

	AssetFileHeader file_header;
	file_header.file_code = 0x42454152;
	file_header.version = 1;
	file_header.num_assets = size(assets);

	remove("res/data.bear");
	FILE *disk = fopen("res/data.bear", "wb");
	keep_writing(disk, &file_header, sizeof(file_header));

	// Skipp this part of the header.
	fseek(disk, sizeof(AssetHeader) * file_header.num_assets, SEEK_CUR);
	for (uint32 i = 0; i < size(assets); i++)
	{
		AssetHeader asset = get(assets, i);
		int64 offset = ftell(disk);
		ASSERT(offset > 0);
		if (asset.type == BAT_MESH)
		{
			uint64 vert_size = sizeof(*asset.mesh.verticies) * asset.mesh.num_verticies;
			uint64 indi_size = sizeof(*asset.mesh.indicies) * asset.mesh.num_indicies;
			keep_writing(disk, asset.mesh.verticies, vert_size);
			keep_writing(disk, asset.mesh.indicies, indi_size);
			asset.data_size = vert_size + indi_size;
			free(asset.mesh.verticies);
			free(asset.mesh.indicies);
		}
		else
		{
			keep_writing(disk, asset.data, asset.data_size);
			free(asset.data);
		}
		asset.data_offset = offset;
		set(assets, i, asset);
	}
	fseek(disk, sizeof(AssetFileHeader), SEEK_SET);
	ASSERT(fwrite(assets.data, sizeof(AssetHeader), file_header.num_assets, disk));

	fclose(disk);

	// Write dummy header
	// Write dummy headers
	// Write contents

	delete_array(&assets);
	delete_array(&endings);
	delete_array(&files);

	return 0;
}

#ifdef _WIN32
#include <windows.h>

void find_files_in_folder(const char *_dir)
{
	char dir[50];
	sprintf(dir, "%s/*", _dir);

	WIN32_FIND_DATA ffd; // Maybe we need As here on the types.
	HANDLE handle = FindFirstFile(dir, &ffd);
	if (handle == INVALID_HANDLE_VALUE)
	{
		printf("Failed to load directory\n");
		return;
	}

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// This is a directory.
			if (*ffd.cFileName == '.')
				continue;
			find_files_in_folder(ffd.cFileName);
		}
		else
		{
			// We assume it's a file.
			char *file_name = ffd.cFileName;
			for (uint32 i = 0; i < size(endings); i++)
			{
				FileData ending = get(endings, i);
				if (ends_with(file_name, ending.file_ending))
				{
					char *ptr = copy_string_to_storage(file_name);
					FileData data = {ptr, ending.type};
					append(&files, data);
				}
			}
		}
	}
	while (FindNextFile(handle, &ffd) != 0);
}

#else // Linux.

#error // Not yet implemented.

#endif

