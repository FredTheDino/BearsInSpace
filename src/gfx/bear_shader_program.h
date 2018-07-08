#pragma once

namespace GFX
{
	struct ShaderInfo
	{
		uint32 type;
		string path;
	};

	struct ShaderProgram
	{
		uint32 id;
	};

	ShaderProgram create_shader_program(Array<ShaderInfo> shader_info)
	{
		ShaderProgram program;

		program.id = glCreateProgram();

		uint32 num_shaders = (uint32) size(shader_info);
		Array<uint32> shader_ids = create_array<uint32>(num_shaders);

		for (uint32 i = 0; i < num_shaders; i++)
		{
			ShaderInfo info = shader_info[i];
			
			// Create shader
			uint32 shader_id = glCreateShader(info.type);
			append(&shader_ids, shader_id);

			// Shader source
			OSFile file = world->plt.read_file(info.path);
			int32 len_src = (uint32) file.size;
			glShaderSource(shader_id, 1, (string *) &file.data, &len_src);
			world->plt.free_file(file);

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
				Array<int8> log = create_array<int8>(len);
				glGetShaderInfoLog(shader_id, len, &len, data_ptr(log));
				
				ERROR_LOG("=======================");
				ERROR_LOG("Failed to compile shader!");
				ERROR_LOG(data_ptr(log));
				ERROR_LOG("=======================");
			
				delete_array(&log);

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
			Array<int8> log = create_array<int8>(len);
			glGetProgramInfoLog(program.id, len, &len, data_ptr(log));

			ERROR_LOG("=======================");
			ERROR_LOG("Failed to link program!");
			ERROR_LOG(data_ptr(log));
			ERROR_LOG("=======================");
			
			delete_array(&log);

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
			Array<int8> log = create_array<int8>(len);
			glGetProgramInfoLog(program.id, len, &len, data_ptr(log));

			ERROR_LOG("=======================");
			ERROR_LOG("Program validation failed!");
			ERROR_LOG(data_ptr(log));
			ERROR_LOG("=======================");
			
			delete_array(&log);

			ASSERT(false);
		}

		// Detach all shaders
		for (uint64 i = 0; i < size(shader_ids); i++)
			glDetachShader(program.id, shader_ids[i]);

		// Delete all shaders
		for (uint64 i = 0; i < size(shader_ids); i++)
			glDeleteShader(shader_ids[i]);

		delete_array(&shader_ids);

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
		glUniformMatrix4fv(uniform_location(program, uniform_name), 1, GL_FALSE, data_ptr(m));
	}
}
