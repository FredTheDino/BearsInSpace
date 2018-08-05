#pragma once

#define DBG_POINT_SIZE 5.0f
#define DBG_LINE_WIDTH 3.0f

namespace GFX
{
	ShaderProgram sp_lines;
	VertexBuffer vb_lines;
	VertexArray va_lines;
	Renderable r_lines;
	Renderable r_points;
	
	void init_debug()
	{
		// GL init
		glPointSize(DBG_POINT_SIZE);
		glLineWidth(DBG_LINE_WIDTH);
		
		// Line Shader Program
		Array<ShaderInfo> line_shader_info = temp_array<ShaderInfo>({
			{ GL_VERTEX_SHADER, "src/shader/debug.vert" },
			{ GL_FRAGMENT_SHADER, "src/shader/debug.frag" }
		});
		sp_lines = create_shader_program(line_shader_info);
		
		// Line Vertex Buffer
		Array<float32> line_data = temp_array<float32>({
			.0f, .0f, .0f,
			.0f, .0f, .0f
		});
		vb_lines = create_vertex_buffer(line_data, GL_DYNAMIC_DRAW);

		// Line Vertex Array
		Array<VertexAttribute> line_vertex_attributes = temp_array<VertexAttribute>({
			{ vb_lines, 0, 3, GL_FLOAT }
		});
		va_lines = create_vertex_array(line_vertex_attributes);

		// Line renderable
		r_lines = { va_lines, sp_lines, 2, {} };

		// Point renderable
		r_points = { va_lines, sp_lines, 1, {} };
	}

	void debug_draw_line(Vec3f pos1, Vec3f pos2, Vec3f color)
	{
		bind(sp_lines);
		send3f(sp_lines, "v_color", color);
		Array<float32> data = temp_array<float32>({
			pos1.x, pos1.y, pos1.z,
			pos2.x, pos2.y, pos2.z
		});
		sub_data(vb_lines, 0, data);
		
		draw(r_lines, GL_LINES);
	}

	void debug_draw_point(Vec3f pos, Vec3f color)
	{
		bind(sp_lines);
		send3f(sp_lines, "v_color", color);

		// Question: Since we know a point will allways have
		// 3 floats in it. And they will be sequential. You
		// can treat it as an array from the start and
		// just send that to the buffer. I think you're
		// making this more complicated than it has to be
		// by adding in the stretchy-buffers. Since they
		// do require some extra setup.
		Array<float32> data = temp_array<float32>({
			pos.x, pos.y, pos.z
		});
		sub_data(vb_lines, 0, data);
		
		draw(r_points, GL_POINTS);
	}

	void destroy_debug()
	{
		// Lines
		delete_vertex_array(va_lines);
		delete_vertex_buffer(vb_lines);
		delete_shader_program(sp_lines);
	}
}
