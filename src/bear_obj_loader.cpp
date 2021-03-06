
struct Mesh
{
	bool valid;
	Array<Vec3f> positions;
	Array<Vec3f> normals;
	Array<Vec2f> uvs;
	Array<uint32> indices;
	uint32 stride;
};

int32 eat_int(char **ptr)
{
	char *p = *ptr;
	bool negative = *p == '-';
	if (negative)
		p++;
	uint32 i = 0;
	while (true)
	{
		char c = *p;
		if (c < '0' || '9' < c)
			break;
		i *= 10;
		i += (c - '0');
		p++;
	}
	*ptr = p;
	return negative ? -i : i;
}

void eat_spaces(char **ptr)
{
	char *p = *ptr;
	while (*p == ' ') p++;
	*ptr = p;
}

float32 eat_float(char **ptr)
{
	char *p = *ptr;
	bool negative = *p == '-';
	if (negative)
		p++;
	float32 f = eat_int(&p);
	if (*p == '.')
	{
		char *dot_pos = ++p;
		int32 decimals = eat_int(&p);
		f += (float32) decimals / (float32) pow( (float32) 10.0f, (int32) (p - dot_pos));
	}
	*ptr = p;
	return negative ? -f : f;
}

Mesh load_mesh(char *path, void *(*alloc)(size_t))
{
	FILE *file = fopen(path, "r");
	fseek(file, 0, SEEK_END);
	uint64 size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *ptr = (char *) alloc(size + 1);
	char *start = ptr;
	fread(ptr, size, 1, file);
	fclose(file);

	Mesh mesh = {};
	// Read in the positions to get an idea for the size.
	mesh.positions	= create_array<Vec3f>(100);
	
	char *end = (char *) &ptr[size - 1];
	for (;(void *) ptr != (void *) end; ptr++)
	{
		if (*ptr == '\0') break;
		if (*ptr == 'v' && *(ptr + 1) == ' ')
		{
			ptr++;
			// v, we have a vertex
			Vec3f v;
			eat_spaces(&ptr);
			v.x = eat_float(&ptr);
			eat_spaces(&ptr);
			v.y = eat_float(&ptr);
			eat_spaces(&ptr);
			v.z = eat_float(&ptr);
			append(&mesh.positions, v);
		}
		while (*ptr != '\n' && *ptr != '\0') ptr++;
		continue;
	}
	// Now we kinda know the size of the other arrays.
	// Just randomly choosen 2
	uint64 num_elements = mesh.positions.size * 2;
	mesh.normals	= create_array<Vec3f> (num_elements);
	mesh.uvs		= create_array<Vec2f> (num_elements);
	mesh.indices	= create_array<uint32>(num_elements);
	
	// Reset the ptr.
	ptr = start;
	for (;ptr != end; ptr++)
	{
		if (*ptr == '\0') break;
		if (*ptr == 'v' && *(ptr + 1) == 'n')
		{
			ptr++;
			ptr++;
			// vn, we have a vertex
			Vec3f n;
			eat_spaces(&ptr);
			n.x = eat_float(&ptr);
			eat_spaces(&ptr);
			n.y = eat_float(&ptr);
			eat_spaces(&ptr);
			n.z = eat_float(&ptr);
			append(&mesh.normals, n);
		}
		else if (*ptr == 'v' && *(ptr + 1) == 't')
		{
			ptr++;
			ptr++;
			// vt, we have a vertex
			Vec2f t;
			eat_spaces(&ptr);
			t.x = eat_float(&ptr);
			eat_spaces(&ptr);
			t.y = eat_float(&ptr);
			append(&mesh.uvs, t);
		}
		else if (*ptr == 'f' && *(ptr + 1) == ' ')
		{
			ptr++;
			ptr++;
			// f, we're reading faces.
			uint32 sum_data = 0;
			do
			{
				if (*ptr >= '0' && *ptr <= '9')
				{
					append(&mesh.indices, (uint32) eat_int(&ptr));
					sum_data++;
				}
				else
				{
					ptr++;
				}
			} while (*ptr != '\n' && *ptr != '\0');
			ASSERT(sum_data % 3 == 0);
			mesh.stride = sum_data / 3;
		}
		while (*ptr != '\n' && *ptr != '\0') ptr++;
	}
	return mesh;
}

void free_mesh(Mesh mesh)
{
	delete_array(&mesh.positions);
	delete_array(&mesh.normals);
	delete_array(&mesh.uvs);
	delete_array(&mesh.indices);
}



