#pragma once

#include "bear_vertex_array.h"
#include "bear_shader_program.h"
//#include "transform.h"
//#include "texture.h"

namespace GFX
{
	struct MatrixProfile
	{
		string               uniform_name;
		uint8                uniform_length;
		//Transform           *transform;
	};

	Array<MatrixProfile> matrix_profiles;
	
	struct Renderable
	{
		VertexArray          vertex_array;
		ShaderProgram        program;
		uint32               num_vertices;
		//Texture             *texture;
		Array<MatrixProfile> matrix_profiles;
	};

	void draw(Renderable r, uint32 mode=GL_TRIANGLES,
			  bool use_global_matrix_profiles=true)
	{
		bind(r.program);

		/*if (use_global_matrix_profiles)
			for (uint64 i = 0; i < size(matrix_profiles); i++)
				r.program->sendM4(mp.first.c_str(), mp.second->matrix());

		if (r.texture != nullptr)
			r.texture->bind();

		r.program->sendM4(r.uniform.c_str(), r.transform->matrix());*/

		bind(r.vertex_array);

		if (r.vertex_array.has_indices)
			glDrawElements(mode, r.num_vertices, GL_UNSIGNED_INT, NULL);
		else
			glDrawArrays(mode, 0, r.num_vertices);
	}

	void add_profile(string uniform_name, uint8 uniform_length/*,
					 Transform *transform*/)
	{
		append(&matrix_profiles, MatrixProfile { uniform_name, uniform_length/*, transform*/ });
	}

	void remove_profile(string uniform_name)
	{
		uint64 _size = size(matrix_profiles);
		for (uint64 i = 0; i < _size; i++)
			if (matrix_profiles[i].uniform_name == uniform_name)
			{ remove(&matrix_profiles, i); return; }
	}
}
