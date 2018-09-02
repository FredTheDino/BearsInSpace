#include "bear_shader_program.h"

namespace GFX
{
	struct ShaderInfo
	{
		uint32 type;
		string path;
	};

	ShaderProgram create_shader_program(Array<ShaderInfo> shader_info)
	{
		ShaderProgram program;

		program.id = glCreateProgram();

		uint32 num_shaders = (uint32) size(shader_info);
		Array<uint32> shader_ids = temp_array<uint32>(num_shaders);

		for (uint32 i = 0; i < num_shaders; i++)
		{
			ShaderInfo info = shader_info[i];
			
			// Create shader
			uint32 shader_id = glCreateShader(info.type);
			append(&shader_ids, shader_id);

			// Shader source
			OSFile file = plt.read_file(info.path, temp_push);
			int32 len_src = (uint32) file.size;
			glShaderSource(shader_id, 1, (string *) &file.data, &len_src);

			// Compile shader
			glCompileShader(shader_id);

			// Get compile status
			int32 shader_compile_status;
			glGetShaderiv(shader_id, GL_COMPILE_STATUS, &shader_compile_status);

			// Handle compile errors
			if (shader_compile_status != GL_TRUE)
			{
				// Info log length
				int32 len;
				glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &len);

				// Get info log
				Array<GLchar> log = temp_array<GLchar>(len);
				glGetShaderInfoLog(shader_id, len, &len, data_ptr(log));
				
				LOG("SHADER ERROR", "=======================");
				LOG("SHADER ERROR", "Failed to compile shader!");
				LOG("SHADER ERROR", data_ptr(log));
				LOG("SHADER ERROR", "=======================");

				ASSERT(false);
			}

			// Attach shader
			glAttachShader(program.id, shader_id);
		}

		// Link program
		glLinkProgram(program.id);

		// Get link status
		int32 program_link_status;
		glGetProgramiv(program.id, GL_LINK_STATUS, &program_link_status);

		// Handler link status
		if (program_link_status != GL_TRUE)
		{
			// Info log length
			int32 len;
			glGetProgramiv(program.id, GL_INFO_LOG_LENGTH, &len);

			// Get info log
			Array<GLchar> log = temp_array<GLchar>(len);
			glGetProgramInfoLog(program.id, len, &len, data_ptr(log));

			LOG("SHADER ERROR", "=======================");
			LOG("SHADER ERROR", "Failed to link program!");
			LOG("SHADER ERROR", data_ptr(log));
			LOG("SHADER ERROR", "=======================");
			

			ASSERT(false);
		}

		// Validate program
		glValidateProgram(program.id);

		// Check validation status
		int32 program_validation_status;
		glGetProgramiv(program.id, GL_VALIDATE_STATUS, &program_validation_status);

		// Handler validation failure
		if (program_validation_status != GL_TRUE)
		{
			// Info log length
			int32 len;
			glGetProgramiv(program.id, GL_INFO_LOG_LENGTH, &len);

			// Get info log
			Array<GLchar> log = temp_array<GLchar>(len);
			glGetProgramInfoLog(program.id, len, &len, data_ptr(log));

			LOG("SHADER ERROR", "=======================");
			LOG("SHADER ERROR", "Program validation failed!");
			LOG("SHADER ERROR", data_ptr(log));
			LOG("SHADER ERROR", "=======================");

			ASSERT(false);
		}

		// Detach all shaders
		for (uint64 i = 0; i < size(shader_ids); i++)
		{
			glDetachShader(program.id, shader_ids[i]);
			glDeleteShader(shader_ids[i]);
		}

		return program;
	}

	void delete_shader_program(ShaderProgram program)
	{
		glDeleteProgram(program.id);
	}

	void bind(ShaderProgram program)
	{
		glUseProgram(program.id);
	}

	int32 uniform_location(ShaderProgram program, string uniform_name)
	{
		return glGetUniformLocation(program.id, uniform_name);
	}

	void sendM4(ShaderProgram program, string uniform_name, Mat4f m)
	{
		glUniformMatrix4fv(uniform_location(program, uniform_name), 1, GL_TRUE, data_ptr(m));
	}

	void send3f(ShaderProgram program, string uniform_name, Vec3f v)
	{
		glUniform3f(uniform_location(program, uniform_name), v.x, v.y, v.z);
	}

	void send2f(ShaderProgram program, string uniform_name, Vec2f v)
	{
		glUniform2f(uniform_location(program, uniform_name), v.x, v.y);
	}

	void send1f(ShaderProgram program, string uniform_name, float32 f)
	{
		glUniform1f(uniform_location(program, uniform_name), f);
	}
}
