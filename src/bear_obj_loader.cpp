
struct Mesh
{
	bool valid;
	Array<Vec3f> positions;
	Array<Vec3f> normals;
	Array<Vec2f> uvs;
	Array<int32> indicies;
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

float32 pow(float32 n, int32 exp)
{
	// TODO: This can be made faster
	float32 result = n;
	if (exp == 0) return 1.0f;
	while (exp != 0)
	{
		result *= n;
		exp--;
	}
	return result;
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
		f += (float32) decimals / (float32) pow(10.0f, (int32) (p - dot_pos) - 1);
	}
	*ptr = p;
	return negative ? -f : f;
}

Mesh load_mesh(OSFile file)
{
	Mesh mesh = {};
	if (file.timestamp == -1)
	{
		return mesh;
	}
	
	// Read in the positions to get an idea for the size.
	mesh.positions	= create_array<Vec3f>(100);
	
	char *ptr = (char *) file.data;
	char *end = &ptr[file.size - 1];
	for (;ptr != end; ptr++)
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
	mesh.normals	= create_array<Vec3f>(mesh.positions.size * 2);
	mesh.uvs		= create_array<Vec2f>(mesh.positions.size * 2);
	mesh.indicies	= create_array<int32>(mesh.positions.size * 2);
	
	// Reset the ptr.
	ptr = (char *) file.data;
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
			while (*ptr != '\n' && *ptr != '\0')
			{
				if (*ptr >= '0' && *ptr <= '9')
					append(&mesh.indicies, eat_int(&ptr));
				ptr++;
			}
		}
		while (*ptr != '\n' && *ptr != '\0') ptr++;
	}
	return mesh;
}

Mesh load_mesh(const char *file_name)
{
	OSFile file = world->plt.read_file(file_name);
	Mesh mesh = load_mesh(file);
	world->plt.free_file(file);
	return mesh;
}

void free_mesh(Mesh mesh)
{
	delete_array(&mesh.positions);
	delete_array(&mesh.normals);
	delete_array(&mesh.uvs);
	delete_array(&mesh.indicies);
}



