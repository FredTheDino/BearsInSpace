bool should_run_tests = true;
int passed_tests = 0;
int total_tests = 0;

#define TEST_HEADER(text) test_header(#text)
void test_header(const char *header)
{
	world->plt.print("\n           \x1b[1m\x1b[4m %s \x1b[0m\n", header);
}

#define TEST(exp) test_(exp, #exp, __LINE__)
void test_(bool check, const char *expression, uint32 line_number)
{
	total_tests++;
	if (check)
		passed_tests++;
	const char *state = check ? 
		"\x1b[32mPASSED\x1b[0m": 
		"\x1b[31mFAILED\x1b[0m";
	world->plt.print(" (%04d) \x1b[1m%s : \x1b[1m%s\x1b[0m\n", line_number, state, expression);
}

void test_summary()
{
	world->plt.print("    \x1b[4m %04d/%04d TESTS PASSED \x1b[0m\n", passed_tests, total_tests);
	
}

void run_tests()
{
	should_run_tests = false;

	TEST_HEADER(Array);
	auto arr = create_array<int32>(100);
	TEST(arr.data != 0);
	for (uint32 i = 0; i < arr.limit; i++)
	{
		append(&arr, (int32) i);
	}
	TEST(arr.size == 100);
	bool all_values_checkout = true;
	remove(&arr, 50);
	set(arr, 70, 0);
	for (uint32 i = 0; i < arr.size; i++)
	{
		int32 actual = (i < 50) ? i : i + 1;
		if (i == 70)
			actual = 0;
		if (get(arr, i) != actual)
		{
			all_values_checkout = false;
			break;
		}
	}
	TEST(all_values_checkout);

	relimit(&arr, 10);
	TEST(arr.limit != 10);

	for (uint32 i = 0; i < 10; i++)
	{
		remove(&arr, 0);
	}
	TEST(arr.size == 0);
	relimit(&arr, 1);
	for (uint32 i = 0; i < 900; i++)
	{
		append(&arr, 0);
	}
	TEST(arr.size != 0);

	delete_array(&arr);
	TEST(arr.data == 0);

	TEST_HEADER(Vec2);
	{
		Vec2f a = {};
		Vec2f b = {0.0f, 0.0f};
		Vec2f c = {1.0f, 1.5f};

		TEST(a == b);
		a = {0.0f, 1.0f};
		b = {1.0f, 0.5f};
		TEST(a + b == c);
		TEST(c - b == a);
		TEST(dot(b, a) == 0.5f);
		TEST(b * 0.0f == a * 0.0f);
		c = {2.0f, 1.0f};
		TEST(b * 2.0f == c);
		a = {1.0f, 1.0f};
		TEST(length_squared(a) == 2.0f);
		a = {5.2f, 0.8f};
		b = {1.3f, 0.2f};
		TEST(a / 2.0f == b * 2.0f);
	}

	TEST_HEADER(Vec3);
	{
		Vec3f a = {};
		Vec3f b = {0.0f, 0.0f, 0.0f};
		Vec3f c = {1.0f, 1.5f, 0.4f};

		TEST(a == b);
		a = {0.0f, 1.0f, 0.1f};
		b = {1.0f, 0.5f, 0.3f};
		TEST(a + b == c);
		TEST(c - b == a);
		TEST(dot(b, a) == (0.5f + 0.1f * 0.3f));
		TEST(b * 0.0f == a * 0.0f);
		c = {2.0f, 1.0f, 0.6f};
		TEST(b * 2.0f == c);
		a = {1.0f, 1.0f, 1.0f};
		TEST(length_squared(a) == 3.0f);
		a = {5.2f, 0.8f, 2.0f};
		b = {1.3f, 0.2f, 0.5f};
		TEST(a / 2.0f == b * 2.0f);
	}

	TEST_HEADER(Vec4);
	{
		Vec4f a = {};
		Vec4f b = {0.0f, 0.0f, 0.0f, 0.0f};
		Vec4f c = {1.0f, 1.5f, 0.4f, 0.0f};

		TEST(a == b);
		a = {0.0f, 1.0f, 0.1f, -0.1f};
		b = {1.0f, 0.5f, 0.3f,  0.1f};
		TEST(a + b == c);
		TEST(c - b == a);
		TEST(dot(b, a) == (0.5f + 0.1f * 0.3f - 0.01f));
		TEST(b * 0.0f == a * 0.0f);
		c = {2.0f, 1.0f, 0.6f, 0.2f};
		TEST(b * 2.0f == c);
		a = {1.0f, 1.0f, 1.0f, 1.0f};
		TEST(length_squared(a) == 4.0f);
		a = {5.2f, 0.8f, 2.0f, 0.4f};
		b = {1.3f, 0.2f, 0.5f, 0.1f};
		TEST(a / 2.0f == b * 2.0f);
	}

	test_summary();
}

