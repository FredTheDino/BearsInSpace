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
		Array<ShaderInfo> line_shader_info = {
			{ GL_VERTEX_SHADER, "src/shader/debug.vert" },
			{ GL_FRAGMENT_SHADER, "src/shader/debug.frag" }
		};
		sp_lines = create_shader_program(line_shader_info);
		delete_array(&line_shader_info);
		
		// Line Vertex Buffer
		Array<float32> line_data = {
			.0f, .0f, .0f,
			.0f, .0f, .0f
		};
		vb_lines = create_vertex_buffer(line_data, GL_DYNAMIC_DRAW);
		delete_array(&line_data);

		// Line Vertex Array
		Array<VertexAttribute> line_vertex_attributes = {
			{ vb_lines, 0, 3, GL_FLOAT }
		};
		va_lines = create_vertex_array(line_vertex_attributes);
		delete_array(&line_vertex_attributes);

		// Line renderable
		r_lines = { va_lines, sp_lines, 2, {} };

		// Point renderable
		r_points = { va_lines, sp_lines, 1, {} };
	}

	void debug_draw_line(Vec3f pos1, Vec3f pos2, Vec3f color)
	{
		bind(sp_lines);
		send3f(sp_lines, "v_color", color);
		Array<float32> data = {
			pos1.x, pos1.y, pos1.z,
			pos2.x, pos2.y, pos2.z
		};
		
		sub_data(vb_lines, NULL, data);
		delete_array(&data);
		
		draw(r_lines, GL_LINES);
	}

	void debug_draw_point(Vec3f pos, Vec3f color)
	{
		bind(sp_lines);
		send3f(sp_lines, "v_color", color);

		Array<float32> data = {
			pos.x, pos.y, pos.z
		};
		sub_data(vb_lines, 0, data);
		delete_array(&data);
		
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
