#pragma once

#include "bear_vertex_array.cpp"
#include "bear_shader_program.cpp"
#include "bear_texture.cpp"
#include "bear_frame_buffer.cpp"

#define GLOBAL_MATRIX_PROFILES 16

struct MatrixProfile
{
	string               uniform_name;

	// Note(Ed): This can be a union and an enum. 
	// And it will take less space.
	Mat4f               *matrix;
	Transform           *transform;
	Camera              *camera;
};

void send_profile(ShaderProgram program, MatrixProfile profile)
{
	if (profile.matrix != nullptr)
		sendM4(program, profile.uniform_name, *profile.matrix);
	else if (profile.transform != nullptr)
		sendM4(program, profile.uniform_name, toMat4f(*profile.transform));
	else if (profile.camera != nullptr)
		sendM4(program, profile.uniform_name, toMat4f(*profile.camera));
}

struct Renderable
{
	VertexArray          vertex_array;
	ShaderProgram        program;
	uint32               num_vertices;
	//Texture             *texture;
	Array<MatrixProfile> matrix_profiles;
};

void draw(FrameBuffer target, Renderable r, uint32 mode=GL_TRIANGLES)
{
	bind(target);

	bind(r.program);

	for (uint64 i = 0; i < size(r.matrix_profiles); i++)
		send_profile(r.program, r.matrix_profiles[i]);

	bind(r.vertex_array);

	if (r.vertex_array.has_indices)
		glDrawElements(mode, r.num_vertices, GL_UNSIGNED_INT, NULL);
	else
		glDrawArrays(mode, 0, r.num_vertices);
}

void draw_to_screen()
{	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glDisable(GL_DEPTH_TEST);

	bind(world->output_program);

	bind(world->output_quad);

	bind(world->output_buffer.textures[0]);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glEnable(GL_DEPTH_TEST);
}
