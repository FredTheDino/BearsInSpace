World *world;
#define DEBUG_LOG(message)  world->plt.log(__FILE__, __LINE__, "DEBUG", message)
#define ERROR_LOG(message)  world->plt.log(__FILE__, __LINE__, "ERROR", message)
#define LOG(type, message)	world->plt.log(__FILE__, __LINE__, type, message)


#define ASSERT(check) ((check) ? (void)0 : assert_(__FILE__, __LINE__, #check))
void assert_(const char *file, uint32 line, const char *check)
{
	world->plt.log(file, line, "ASSERT", check);
	HALT_AND_CATCH_FIRE();
}

#define MALLOC2(type, num) \
	(type *) world->plt.malloc(__FILE__, __LINE__, sizeof(type) * num)
#define MALLOC1(type) \
	(type *) world->plt.malloc(__FILE__, __LINE__, sizeof(type))

#define GET_MACRO(_2, _1, NAME, ...) NAME
#define MALLOC(...) \
	GET_MACRO(__VA_ARGS__, MALLOC2, MALLOC1) (__VA_ARGS__)

#define FREE(ptr) \
	world->plt.free((void *)ptr)

#define REALLOC(ptr, size) \
	world->plt.realloc(__FILE__, __LINE__, (void *) ptr, size)
