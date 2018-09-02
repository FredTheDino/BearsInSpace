
#define HALT_AND_CATCH_FIRE() ((int *)(void *)0)[0] = 1
#define ASSERT(expr) if (!(expr)) { HALT_AND_CATCH_FIRE(); }

#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "bear_types.h"
#include "bear_array.h"
#include "bear_array_plt.cpp"

//#include "gfx/bear_font.h"

#include "bear_wav_loader.cpp"
#include "bear_obj_loader.cpp"

#include "bear_asset.h"

Array<FileData> files;
Array<FileData> endings;

char __line[256];
bool get_line(char **line, uint64 *length, FILE *file)
{
	*line = (char *) __line;
	uint64 l = 0;
	char c;
	do 
	{
		c = fgetc(file);
		if (feof(file))
			break;
		__line[l] = c;
		l++;
	} while (c != '\n' && c != '\r' && c != '\0');
	if (l == 0)
		return false;

	__line[l] = '\0';
	return true;
}

int64 str_len(const char *str)
{
	int64 i = 0;
	while (str[i++] != '\0');
	return i;
}

uint64 str_eq(const char *a, const char *b, int64 len=-1)
{
	if (len == -1)
	{
		uint64 a_len = str_len(a);
		uint64 b_len = str_len(b);
		len = a_len < b_len ? a_len : b_len;
		len--;
	}

	for (uint64 i = 0; i < len; i++)
	{
		if (b[i] != a[i])
		{
			return false;
		}
	}
	return true;
}

const char *go_to_after(const char *source, const char *look_for)
{
	int64 look_for_length = str_len(look_for) - 1;
	const char *ptr = source;
	while (*ptr)
	{
		if (str_eq(ptr, look_for, look_for_length))
		{
			return ptr + look_for_length;
		}
		ptr++;
	}
	return nullptr;
}

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
	FILE_ENDING(BAT_FONT, "fnt");
	return endings;
}


void keep_writing(FILE *file, void *data, uint64 len)
{
	int32 ret = fwrite(data, 1, len, file);
	ASSERT(ret != 0);
}

void find_files_in_folder(const char *);
int main(int arg_len, char **args)
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
					printf("%s (image), %dx%dx%d, size: %lld\n", file_path, 
							header.image.width, header.image.height, 
							header.image.color_depth, header.data_size);
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
					for (uint64 i = 0; i < size(mesh.indices);)
					{
						int32 p_i = -1;
						int32 u_i = -1;
						int32 n_i = -1;
						ASSERT(mesh.stride > 0);
						ASSERT(mesh.stride < 4);
						// These has to be in this order. Since this is how they will be packed. I know
						// It's kinda B.
						ASSERT(size(mesh.positions));
						p_i = get(mesh.indices, i++) - 1;
						if (size(mesh.uvs))
							u_i = get(mesh.indices, i++) - 1;
						if (size(mesh.normals))
							n_i = get(mesh.indices, i++) - 1;

						ASSERT(-1 <= p_i);
						ASSERT(-1 <= n_i);
						ASSERT(-1 <= u_i);
						Indici full_indici = {p_i, n_i, u_i};
						int32 vertex_index = find(full, full_indici);
						if (vertex_index == -1)
						{
							// Didn't find it.
							append(&full, full_indici);
							Vec3f pos, nor;
							Vec2f uv;
							pos = get(mesh.positions, p_i);
							if (0 <= u_i)
								uv = get(mesh.uvs, u_i);
							else
								uv = {};
							if (0 <= n_i)
								nor = normalize(get(mesh.normals, n_i));
							else
								nor = {};

							Vertex vertex; 
							vertex.x = pos.x;
							vertex.y = pos.y;
							vertex.z = pos.z;

							vertex.nx = nor.x;
							vertex.ny = nor.y;
							vertex.nz = nor.z;

							vertex.u = uv.x;
							vertex.v = uv.y;

							append(&out_indicies, (uint32) size(out_verticies));
							append(&out_verticies, vertex); 
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
					header.mesh.indices = out_indicies.data;

					break;
				}
			case (BAT_FONT):
				{
					uint64 path_length = str_len(file_path);
					file_path[path_length - 3] = 't';
					file_path[path_length - 2] = 'x';
					header.font.image = (int8 *) stbi_load(file_path, 
							&header.font.width, &header.font.height, 
							&header.font.color_depth, 0);
					ASSERT(header.font.width == header.font.height);
					uint32 image_size = header.font.width * header.font.height * header.font.color_depth;

					file_path[path_length - 3] = 'n';
					file_path[path_length - 2] = 't';
					FILE *handle = fopen(file_path, "r");
					ASSERT(handle);
					
					char *line;
					size_t length;

					uint32 current_glyph = 0;
					uint32 current_kerning = 0;
					while (get_line(&line, &length, handle))
					{
						if (str_eq(line, "chars "))
						{
							header.font.num_glyphs = atoi(go_to_after(line, "count="));
							header.font.glyphs = (Glyph *) malloc(header.font.num_glyphs * sizeof(Glyph));
						}
						else if (str_eq(line, "char "))
						{
							Glyph g;
							g.id = atoi(go_to_after(line, "id="));
							g.u = ((float32) atoi(go_to_after(line, "x="))) / (float32) header.font.width;
							g.v = ((float32) atoi(go_to_after(line, "y="))) / (float32) header.font.width;
							g.w = ((float32) atoi(go_to_after(line, "width="))) / (float32) header.font.width;
							g.h = ((float32) atoi(go_to_after(line, "height="))) / (float32) header.font.width;
							g.x = ((float32) atoi(go_to_after(line, "xoffset="))) / (float32) header.font.width;
							g.y = ((float32) atoi(go_to_after(line, "yoffset="))) / (float32) header.font.width;
							g.x_advance = ((float32) atoi(go_to_after(line, "xadvance="))) / (float32) header.font.width;
							header.font.glyphs[current_glyph++] = g;
						}
						else if (str_eq(line, "kernings "))
						{
							header.font.num_kernings = atoi(go_to_after(line, "count="));
							header.font.kernings = (Kerning *) malloc(header.font.num_kernings * sizeof(Kerning));
						}
						else if (str_eq(line, "kerning "))
						{
							Kerning k;
							k.first = atoi(go_to_after(line, "first="));
							k.second = atoi(go_to_after(line, "second="));
							k.amount = ((float32) atoi(go_to_after(line, "amount="))) / (float32) header.font.width;
							header.font.kernings[current_kerning++] = k;
						}
					}

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
			uint64 indi_size = sizeof(*asset.mesh.indices) * asset.mesh.num_indicies;
			keep_writing(disk, asset.mesh.verticies, vert_size);
			keep_writing(disk, asset.mesh.indices, indi_size);
			asset.data_size = vert_size + indi_size;
			free(asset.mesh.verticies);
			free(asset.mesh.indices);
		}
		else if (asset.type == BAT_FONT)
		{
			uint64 image_size = sizeof(int8) * asset.font.width * asset.font.height * asset.font.color_depth;
			uint64 glyph_size = sizeof(Glyph) * asset.font.num_glyphs;
			uint64 kerning_size = sizeof(Kerning) * asset.font.num_kernings;
			keep_writing(disk, asset.font.image, image_size);
			keep_writing(disk, asset.font.glyphs, glyph_size);
			keep_writing(disk, asset.font.kernings, kerning_size);
			asset.data_size = image_size + kerning_size + glyph_size;
			free(asset.font.image);
			free(asset.font.glyphs);
			free(asset.font.kernings);
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
#include <dirent.h>

void find_files_in_folder(const char *_dir)
{
	struct dirent *de;
	DIR *dir = opendir(_dir);
	if (dir == 0)
	{
		printf("Failed to open dir '%s'\n", _dir);
		return;
	}

	while ((de = readdir(dir)) != 0)
	{
		char *file_name = de->d_name;
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

	closedir(dir);
}

#endif

