
#if 0
#define LOG(type, message) debug_log_(__FILE__, __LINE__, type, message)
#define ERROR_LOG(message) debug_log_(__FILE__, __LINE__, "ERROR", message)

#define DEBUG_LOG(message) debug_log_(__FILE__, __LINE__, "DEBUG", message)
void debug_log_(const char *file_name, const int line_number, const char *type, const char *message)
{
	// Replace this.
#ifdef WIN32
	win_printf("[%s:%d] %s: %s\n", file_name, line_number, type, message);
#else
	printf("[%s:%d] %s: %s\n", file_name, line_number, type, message);
#endif
}

#define ASSERT(check) ((check) ? (void)0 : ASSERT_(__FILE__, __LINE__, #check))
void inline ASSERT_(const char *file_name, const int line_number, const char *check)
{
	debug_log_(file_name, line_number, "ASSERT", check);
	HALT_AND_CATCH_FIRE();
}

#define MALLOC2(type, num) (type *) malloc_(__FILE__, __LINE__, sizeof(type) * num)
#define MALLOC1(type) (type *) malloc_(__FILE__, __LINE__, sizeof(type))

#define MALLOC_GET(_1, NAME, ...) NAME
#define MALLOC(...) MALLOC_GET(__VA_ARGS__, MALLOC1, MALLOC2)(__VA_ARGS__)

#define FREE(ptr) free_((void *)ptr)

#define REALLOC(ptr, size) realloc_(__FILE__, __LINE__, (void *) ptr, size)
#endif

