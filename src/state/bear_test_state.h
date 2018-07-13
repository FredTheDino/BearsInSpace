#pragma once

namespace GSTEST
{

	GFX::Renderable renderable;
	GFX::VertexBuffer vertex_buffer;
	GFX::VertexArray vertex_array;
	
	void enter()
	{
		// Vertex buffer
		Array<float32> data = {
			.0f, .5f,
			.5f, -.5f,
			-.5f, -.5f
		};

		for (int i = 0; i < 6; i++)
			PRINT("%d: %f\n", i, data[i]);
		
		vertex_buffer = GFX::create_vertex_buffer(data);
		
		delete_array(&data);
		
		// Vertex array
		Array<GFX::VertexAttribute> attributes = { {vertex_buffer, 0, 2, GL_FLOAT} };
		vertex_array = GFX::create_vertex_array(attributes);
		delete_array(&attributes);

		PRINT("Has indices: %d\n", (int) vertex_array.has_indices);

		// Renderable
		renderable.vertex_array = vertex_array;
		renderable.num_vertices = 3;
	}

	void update(float32 delta)
	{

	}

	void draw()
	{
		GFX::draw(renderable);
	}

	void exit()
	{
		GFX::delete_vertex_array(vertex_array);
		GFX::delete_vertex_buffer(vertex_buffer);
	}
}

GameState test_state = {
	GSTEST::enter,
	GSTEST::update,
	GSTEST::draw,
	GSTEST::exit
};
