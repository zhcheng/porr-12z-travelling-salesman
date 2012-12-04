#include <float.h> // FLT_MAX
#include <limits.h> // INT_MAX
#include <stdlib.h> // rand
#include <time.h>

int best_index = 0;
float best_value = FLT_MAX;
int global_benchmark = INT_MAX;

// TODO: Make these functions inline
long clock_ms()
{
#ifdef _WIN32
	return (long)(clock() * 1000 / CLOCKS_PER_SEC);
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return (long)(ts.tv_sec * 1000.0f + ts.tv_nsec / 1000.0f);
#endif
}

int rand_my(unsigned int* seed) {
#ifdef _WIN32
	return rand();
#else
	return rand_r(seed);
#endif
}
