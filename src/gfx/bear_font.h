#pragma once

#include <cfloat>
#include <ft2build.h>
#include FT_FREETYPE_H

#define FREETYPE_ERROR_LOG(...) ERROR_LOG("FREETYPE", __VA_ARGS__)

#define FONT_PIXEL_SIZES 96
#define FONT_BORDER_SIZE 8

struct FTChar
{
	uint32  width;
	uint32  height;
	int32   bearing_x;
	int32   bearing_y;
	int64   advance;
	Vec2f   uv_min;
	Vec2f   uv_max;
};

struct FontEntry
{
	FontEntry *next;
	string name;
	GFX::Texture texture;
	Array<FTChar> glyphs;
} font_map[256];

uint8 font_map_hash(string name)
{
	return str_len(name) > 1 ?
		(((((uint8) *name) - 'a') & 0x0F) << 4) | ((((uint8) name[1]) - 'a') & 0x0F)
		: (uint8) *name;
}

FontEntry get_font(string name)
{
	FontEntry *entry = &font_map[font_map_hash(name)];

	// No match with name
	if (size(entry->glyphs) == 0)
		ASSERT(false);

	while (entry != nullptr && !str_eq(entry->name, name))
	{
		entry = entry->next;
	}

	// No match with name
	if (entry == nullptr)
		ASSERT(false);

	return *entry;
}

void add_font(string name, GFX::Texture texture, Array<FTChar> glyphs)
{
	FontEntry *entry = &font_map[font_map_hash(name)];
	
	if (size(entry->glyphs) == 0)
	{
		entry->texture = texture;
		entry->glyphs = glyphs;
	}
	else
	{
		while (!str_eq(entry->name, name))
		{
			if (entry->next == nullptr)
			{
				entry->next = new FontEntry;
				*entry->next = {};
				entry->next->texture = texture;
				entry->next->glyphs = glyphs;
				entry = entry->next;
				break;
			}
			entry = entry->next;
		}
	}
	entry->name = name;
}

bool is_inside_font(uint8 *src, int32 x, int32 y, int32 width, int32 height, int32 border)
{
	return !(y < border || y >= height + border || x < border || x >= width + border) && src[(y - border) * width + x - border] > 0;
}

void bitmap_to_sdf(uint8 *dst, uint8 *src, int32 width, int32 height, int32 border)
{
	int32 search_radius = width / 2;
	
	for (int32 i = 0; i < height + border * 2; i++)
	{
		for (int32 j = 0; j < width + border * 2; j++)
		{
			bool in = is_inside_font(src, j, i, width, height, border);
			Array<Vec2f> dists = temp_array<Vec2f>(8);
			append(&dists, { FLT_MAX, FLT_MAX });
			
			for (int32 k = -search_radius; k < search_radius; k++)
			{
				for (int32 l = -search_radius; l < search_radius; l++)
				{
					if (i + k < 0 || j + l < 0 || i + k >= height + 2 * border || j + l >= width + 2 * border)
						continue;
					
					bool s = is_inside_font(src, j + l, i + k, width, height, border);
					
					if (in ^ s)
					{
						if (length_squared(dists[0]) > k * k + l * l)
						{
							clear(&dists);
							append(&dists, { (float32) l, (float32) k });
						}
						else if (length_squared(dists[0]) == k * k + l * l)
						{
							bool should_append = false;
							for (uint8 m = 0; m < size(dists); m++)
								if (absolute(dists[m].x - l) <= 2.0f && absolute(dists[m].y - k) <= 2.0f)
									should_append = true;

							if (should_append)
								append(&dists, { (float32) l, (float32) k });
						}
					}
				}
			}

			Vec2f dist = {};
			
			for (uint8 k = 0; k < size(dists); k++)
			{
				dist.x += dists[k].x;
				dist.y += dists[k].y;
			}

			dist /= size(dists);
			
			if (in)
				dst[i * (width + 2 * border) + j] = minimum((uint8) (0x7F * (1.0f + minimum(length_squared(dist) / width, 1.0f))), (uint8) 0xFF) ;
			else
				dst[i * (width + 2 * border) + j] = maximum((uint8) (0x7F * (1.0f - minimum(length_squared(dist) / width, 1.0f))), (uint8) 0x00);
		}
	}
}

void load_font(string name, string path, float32 uv_margin=.5f)
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		FREETYPE_ERROR_LOG("Failed to initialize freetype!");
		ASSERT(false);
	}

	FT_Face face;
	if (FT_New_Face(ft, path, 0, &face))
	{
		FREETYPE_ERROR_LOG("Failed to load font: %s", path);
		ASSERT(false);
	}

	FT_Set_Pixel_Sizes(face, 0, FONT_PIXEL_SIZES);

	// Allow all texture dimensions
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	Array<FTChar> glyphs = static_array<FTChar>(128);

	struct GlyphTexture
	{
		uint8 glyph;
		uint8 *data;
		uint32 x, y;
		uint32 width, height;
	};
	
	Array<GlyphTexture> textures = static_array<GlyphTexture>(128);
	
	// Retrieve FreeType data
	for (uint8 i = 0; i < 128; i++)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			FREETYPE_ERROR_LOG("Failed to load glyph: %c", i);
			continue;
		}

		uint32 w = face->glyph->bitmap.width + 2 * FONT_BORDER_SIZE;
		uint32 h = face->glyph->bitmap.rows + 2 * FONT_BORDER_SIZE;

		// Duplicate data, since the source will be deleted upon loading next glyph
		uint8 *data = (uint8 *) static_push(w * h);
		//memcpy((void *) data, face->glyph->bitmap.buffer, w * h);
		bitmap_to_sdf(data, face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, FONT_BORDER_SIZE);

		// Make sure the glyph textures are sorted from largest to smallest
		if (size(textures) == 0)
			append(&textures, { i, data, 0, 0, w, h });
		else
		{
			for (uint8 j = size(textures); j > 0; j--)
			{
				GlyphTexture gt = textures[j - 1];
				if (gt.width * gt.height > w * h)
				{
					insert(&textures, j, { i, data, 0, 0, w, h });
					break;
				}
				else if (j == 1)
				{
					prepend(&textures, { i, data, 0, 0, w, h });
					break;
				}
			}
		}

		FTChar c =
		{
			face->glyph->bitmap.width + 2 * FONT_BORDER_SIZE,
			face->glyph->bitmap.rows + 2 * FONT_BORDER_SIZE,
			face->glyph->bitmap_left - FONT_BORDER_SIZE,
			face->glyph->bitmap_top - FONT_BORDER_SIZE,
			face->glyph->advance.x + (FONT_BORDER_SIZE << 6)
		};

		append(&glyphs, c);
	}

	// Pack textures (TODO: IMPROVE THIS A LOT)
	uint32 total_width = 0, total_height = 0;
	
	uint32 current_x = 0;
	for (uint8 i = 0; i < 128; i++)
	{
		GlyphTexture *gt = get_ptr(textures, i);
		if (gt->width == 0 || gt->height == 0)
			break;

		gt->x = current_x;
		gt->y = 0;
		
		current_x += gt->width;
		
		if (gt->height > total_height)
			total_height = gt->height;
	}

	total_width = current_x;

	LOG("GLYPH", "TW: %d, TH: %d", (int32) total_width, (int32) total_height);

	// Combine textures
	uint8 *data = (uint8 *) static_push(total_width * total_height);
	
	for (uint8 i = 0; i < 128; i++)
	{
		GlyphTexture gt = textures[i];
		
		if (gt.width > 0 && gt.height > 0)
		{
			for (uint32 j = 0; j < gt.height; j++)
			{
				for (uint32 k = 0; k < gt.width; k++)
				{
					data[(total_width * total_height - (j + gt.y + 1) * total_width) + (k + gt.x)] = gt.data[j * gt.width + k];
				}
			}
			
			static_pop((void *) gt.data);
			
			FTChar *c = get_ptr(glyphs, gt.glyph);
			c->uv_min = { ((float32) gt.x + uv_margin) / total_width, 1.0f - ((float32) gt.height + uv_margin) / total_height };
			c->uv_max = { ((float32) (gt.x + gt.width) - uv_margin) / total_width, 1.0f };
		}
	}

	add_font(name, GFX::create_texture({ (int32) total_width, (int32) total_height, 0, data }, GL_RED, GL_RED), glyphs);

	static_pop((void *) data);

	delete_array(&textures);

	// Only allow texture dimensions of 4^n (DEFAULT)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void destroy_fonts()
{
	for (uint16 i = 0; i < 256; i++)
	{
		if (size(font_map[i].glyphs) > 0)
		{
			GFX::delete_texture(font_map[i].texture);
			delete_array(&font_map[i].glyphs);
		}

		FontEntry *e = font_map[i].next;
		while (e != nullptr)
		{
			GFX::delete_texture(e->texture);
			delete_array(&e->glyphs);
			
			FontEntry *e_old = e;
			e = e->next;
			delete e_old;
		}

		font_map[i] = {};
	}
}

#undef FREETYPE_ERROR_LOG

namespace GFX
{

	#define GFX_MAXIMUM_TEXT_LENGTH 64
	
	ShaderProgram font_program_flat;
	VertexBuffer font_vertex_buffer;
	VertexArray font_vertex_array;
	IndexBuffer font_index_buffer;
	
	void init_font_rendering()
	{
		// Shader programs
		font_program_flat = create_shader_program(temp_array<ShaderInfo>({
					{ GL_VERTEX_SHADER, "src/shader/font_flat.vert" },
					{ GL_FRAGMENT_SHADER, "src/shader/font_flat.frag" }
				}));

		// Vertex buffer
		
		// 4 * 4 <= 4 vertices per char glyph, 4 floats per vertex (x, y, u, v)
		font_vertex_buffer = create_vertex_buffer(4 * 4 * GFX_MAXIMUM_TEXT_LENGTH, GL_DYNAMIC_DRAW);

		// Index buffer
		Array<uint32> indices = temp_array<uint32>(6 * GFX_MAXIMUM_TEXT_LENGTH);

		for (uint32 i = 0; i < GFX_MAXIMUM_TEXT_LENGTH; i++)
		{
			append(&indices, i * 4 + 0);
			append(&indices, i * 4 + 1);
			append(&indices, i * 4 + 3);
			append(&indices, i * 4 + 1);
			append(&indices, i * 4 + 2);
			append(&indices, i * 4 + 3);
		}
		
		font_index_buffer = create_index_buffer(indices);

		// Vertex array
		
		font_vertex_array = create_vertex_array(temp_array<VertexAttribute>({
					{ font_vertex_buffer, 0, 2, GL_FLOAT, false, 4 * sizeof(float32), (void *) 0 },
					{ font_vertex_buffer, 1, 2, GL_FLOAT, false, 4 * sizeof(float32), (void *) (2 * sizeof(float32)) }
				}), font_index_buffer);
	}

	void destroy_font_rendering()
	{
		delete_vertex_array(font_vertex_array);
		delete_index_buffer(font_index_buffer);
		delete_vertex_buffer(font_vertex_buffer);
		delete_shader_program(font_program_flat);
	}
	
	void draw_surface_text(string font_name, float32 x, float32 y, string text, float32 scale, Vec3f color={ 1.0f, 1.0f, 1.0f })
	{
		uint32 length = strlen(text);

		ASSERT(length <= GFX_MAXIMUM_TEXT_LENGTH);

		FontEntry font = get_font(font_name);

		Array<float32> data = temp_array<float32>(4 * 4 * length);
		
		for (uint32 i = 0; i < length; i++)
		{
			FTChar c = font.glyphs[text[i]];

			float32 xpos = x + c.bearing_x * scale;
			float32 ypos = y - (c.height - c.bearing_y) * scale;
			float32 w = c.width * scale;
			float32 h = c.height * scale;

			append(&data, xpos / ASPECT_RATIO);
			append(&data, ypos + h);
			append(&data, c.uv_min.x);
			append(&data, c.uv_max.y);

			append(&data, (xpos + w) / ASPECT_RATIO);
			append(&data, ypos + h);
			append(&data, c.uv_max.x);
			append(&data, c.uv_max.y);

			append(&data, (xpos + w) / ASPECT_RATIO);
			append(&data, ypos);
			append(&data, c.uv_max.x);
			append(&data, c.uv_min.y);

			append(&data, xpos / ASPECT_RATIO);
			append(&data, ypos);
			append(&data, c.uv_min.x);
			append(&data, c.uv_min.y);
			
			x += (c.advance >> 6) * scale;
		}

		sub_data(font_vertex_buffer, 0, data);
		
		bind(font_program_flat);
		bind(font_vertex_array);
		bind(font.texture);
		glDrawElements(GL_TRIANGLES, length * 6, GL_UNSIGNED_INT, (void *) 0);
	}
}
