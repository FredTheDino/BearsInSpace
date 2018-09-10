#pragma once

#define GFX_MAXIMUM_TEXT_LENGTH 64

ShaderProgram font_program_flat;
VertexBuffer font_vertex_buffer;
VertexArray font_vertex_array;
IndexBuffer font_index_buffer;

void init_font_rendering();

void destroy_font_rendering();

void draw_surface_text(AssetID asset_id, 
		float32 x, float32 y, string text, 
		float32 scale, Vec3f color={ 1.0f, 1.0f, 1.0f });

// 
// NOTE:
// _OLD_ SDF Font RENDERING, there are major bugs in the distance calculations since 
// the circle generated doesn't hit all pixels. But it is a good blueprint
// for how to use FreeType.
// 

#if 0
#include <cfloat>
#include <ft2build.h>
#include FT_FREETYPE_H

#define FREETYPE_ERROR_LOG(...) ERROR_LOG("FREETYPE", __VA_ARGS__)

#define FONT_PIXEL_SIZES 96
#define FONT_BORDER_SIZE 8

struct Glyph
{
	uint32  width;
	uint32  height;
	int32   bearing_x;
	int32   bearing_y;
	int64   advance;
	Vec2f   uv_min;
	Vec2f   uv_max;
};

bool is_white_space(char c)
{
	return c <= 32;
}

void add_font(string name, GFX::Texture texture, Array<Glyph> glyphs)
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
	int32 search_radius = width;

	for (int32 center_y = 0; 
			center_y < height + border * 2; 
			center_y++)
	{
		for (int32 center_x = 0; center_x < width + border * 2; center_x++)
		{
			bool in = is_inside_font(src, center_x, center_y, width, height, border);

			Vec2f offset = {};

			for (int32 r = 1; r <= search_radius; r++)
			{
				for (int32 angle = -r * r; angle < r * r; angle++)
				{
					// NOTE(Ed): This can probably be made faster or smarter. Because we will hitt doubles.
					int32 x = center_x + r * cos((angle * PI) / (r * r));
					int32 y = center_y + r * sin((angle * PI) / (r * r));
					bool s = is_inside_font(src, x, y, width, height, border);

					if (in ^ s)
					{
						offset = { (float32) x - center_x, (float32) y - center_y };
						break;
					}
				}

				if (offset.x != 0)
					break;
			}

			float32 distance = length_squared(offset);
			uint32 current_pixel = center_y * (width + 2 * border) + center_x;
			if (in)
			{
				int16 d;
				if (distance == 0)
					d = 0xFF;
				else
					d = 0x7F * (1.0f + minimum(1.0f, sqrt(distance) / FONT_PIXEL_SIZES * 2));

				ASSERT(d >= 0x7F && d <= 0xFF);

				dst[current_pixel] = (uint8) d;
			}
			else
			{
				int16 d;
				if (distance == 0)
					d = 0x00;
				else
					d = 0x7F * (1.0f - minimum(1.0f, sqrt(distance) / FONT_PIXEL_SIZES * 2));

				ASSERT(d >= 0x00 && d <= 0x7F);

				dst[current_pixel] = (uint8) d;
			}
		}
	}
}

void scale_down(uint8 *dst, uint8 *src, int32 width, int32 height)
{
	for (int32 i = 0; i < height / 2; i++)
	{
		for (int32 j = 0; j < width / 2; j++)
		{
			int32 src_x = j * 2 + 1;
			int32 src_y = i * 2 + 1;

			uint32 total = 0;
			total += src[(src_y + 0) * (width) + (src_x + 0)];

			total += src[(src_y + 1) * (width) + (src_x + 0)] / 2;
			total += src[(src_y - 1) * (width) + (src_x + 0)] / 2;
			total += src[(src_y + 0) * (width) + (src_x + 1)] / 2;
			total += src[(src_y + 0) * (width) + (src_x - 1)] / 2;

			total += src[(src_y + 1) * (width) + (src_x + 1)] / 4;
			total += src[(src_y + 1) * (width) + (src_x - 1)] / 4;
			total += src[(src_y - 1) * (width) + (src_x + 1)] / 4;
			total += src[(src_y - 1) * (width) + (src_x - 1)] / 4;

			dst[i * (width / 2) + j] = total / 4;
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

	Array<Glyph> glyphs = static_array<Glyph>(128);

	struct GlyphTexture
	{
		uint8 glyph;
		uint8 *data;
		uint32 x, y;
		uint32 width, height;
	};

	Array<GlyphTexture> textures = static_array<GlyphTexture>(128);

	// Retrieve FreeType data
	for (uint16 i = 0; i < 128; i++)
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
			append(&textures, { (uint8) i, data, 0, 0, w, h });
		else
		{
			for (uint8 j = size(textures); j > 0; j--)
			{
				GlyphTexture gt = textures[j - 1];
				if (gt.width * gt.height > w * h)
				{
					insert(&textures, j, { (uint8) i, data, 0, 0, w, h });
					break;
				}
				else if (j == 1)
				{
					prepend(&textures, { (uint8) i, data, 0, 0, w, h });
					break;
				}
			}
		}

		Glyph c =
		{
			face->glyph->bitmap.width + 2 * FONT_BORDER_SIZE,
			face->glyph->bitmap.rows + 2 * FONT_BORDER_SIZE,
			face->glyph->bitmap_left - FONT_BORDER_SIZE,
			face->glyph->bitmap_top - FONT_BORDER_SIZE,
			face->glyph->advance.x + (FONT_BORDER_SIZE << 6)
		};

		append(&glyphs, c);

		LOG("BMP->SDF", "Status: %d/%d (%f%%)", i, 128, (i * 100.0f) / 128);
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

			Glyph *c = get_ptr(glyphs, gt.glyph);
			c->uv_min = { ((float32) gt.x + uv_margin) / total_width, 1.0f - ((float32) gt.height + uv_margin) / total_height };
			c->uv_max = { ((float32) (gt.x + gt.width) - uv_margin) / total_width, 1.0f };
		}
	}

	uint8 *scaled_data1 = (uint8 *) static_push((total_width / 2) * (total_height / 2));

	scale_down(scaled_data1, data, total_width, total_height);

	static_pop((void *) data);

	uint8 *scaled_data2 = (uint8 *) static_push((total_width / 4) * (total_height / 4));

	scale_down(scaled_data2, scaled_data1, total_width / 2, total_height / 2);

	static_pop((void *) scaled_data1);

	add_font(name, GFX::create_texture((int32) total_width / 4, (int32) total_height / 4, 1, scaled_data2, GL_RED, GL_RED), glyphs);

	delete_array(&textures);

	static_pop((void *) scaled_data2);

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
#endif
