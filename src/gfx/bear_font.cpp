#include "bear_font.h"

void init_font_rendering()
{
	// Shader programs
	font_program_flat = create_shader_program(temp_array<ShaderInfo>({
				{ GL_VERTEX_SHADER, "src/shader/font_flat.vert" },
				{ GL_FRAGMENT_SHADER, "src/shader/font_flat.frag" }
				}));

	// Vertex buffer

	// 4 * 4 <= 4 vertices per char glyph, 4 floats per vertex (x, y, u, v)
	font_vertex_buffer = create_vertex_buffer(NULL, 4 * 4 * GFX_MAXIMUM_TEXT_LENGTH, GL_DYNAMIC_DRAW);

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

// Assumes ascii and that all glyph lists look the same.
uint8 char_mapping_function(char c)
{
	if (c == '\0')
		return c;
	if (c == 10)
		return 1;
	return c - ' ' + 2;
}

void draw_surface_text(AssetID asset_id, float32 x, float32 y, string text, float32 scale, Vec3f color)
{
	ASSERT(get_type(asset_id) == BAT_FONT);
	if (!is_loaded(asset_id))
		return;

	uint32 length = str_len(text);
	ASSERT(length <= GFX_MAXIMUM_TEXT_LENGTH);

	AssetHeader header = get_asset_header(asset_id);
	uint64 num_glyphs = header.font.num_glyphs;
	Glyph *glyphs = header.font.glyphs;
	Texture texture = get_asset(asset_id).texture;
	uint64 num_kernings = header.font.num_kernings;
	Kerning *kernings = header.font.kernings;

	Array<float32> data = temp_array<float32>(4 * 4 * length);

	char last_c = 0;
	for (uint32 i = 0; i < length; i++)
	{
		char current_c = text[i];
		//
		// TODO: Kerning
		//

		for (uint64 i = 0; i < num_kernings; i++)
		{
			if (kernings[i].first != last_c)
				continue;
			if (kernings[i].second != current_c)
				continue;
			x += kernings[i].amount * scale * 0.5f;
			break;
		}

		last_c = current_c;
		uint64 mapped = char_mapping_function(current_c);
		ASSERT(mapped < num_glyphs);
		ASSERT(0 <= mapped);

		Glyph c = glyphs[mapped];
		float32 xpos = x + c.x * scale;
		float32 ypos = y - 0.0f * scale;
		float32 w = c.w * scale;
		float32 h = c.h * scale;

		Vec2f uv_min = {c.u      , c.v + c.h};
		Vec2f uv_max = {c.u + c.w, c.v      };

		append(&data, xpos / ASPECT_RATIO);
		append(&data, ypos + h);
		append(&data, uv_min.x);
		append(&data, uv_max.y);

		append(&data, (xpos + w) / ASPECT_RATIO);
		append(&data, ypos + h);
		append(&data, uv_max.x);
		append(&data, uv_max.y);

		append(&data, (xpos + w) / ASPECT_RATIO);
		append(&data, ypos);
		append(&data, uv_max.x);
		append(&data, uv_min.y);

		append(&data, xpos / ASPECT_RATIO);
		append(&data, ypos);
		append(&data, uv_min.x);
		append(&data, uv_min.y);

		x += c.x_advance * scale * 0.9f;
	}

	sub_data(font_vertex_buffer, 0, data);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float32 edge = (0.4f / (scale * scale));
	send1f(font_program_flat, "text_edge", edge);
	send3f(font_program_flat, "text_color", color);
	bind(font_program_flat);
	bind(font_vertex_array);
	bind(texture);
	glDrawElements(GL_TRIANGLES, length * 6, GL_UNSIGNED_INT, (void *) 0);
}
