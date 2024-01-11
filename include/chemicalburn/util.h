#pragma once
#ifndef CHEMICALBURN_UTIL_H
#define CHEMICALBURN_UTIL_H

#include <assert.h>

#define CB_ASSERT(x) assert(x)

#ifndef NDEBUG
#include <stdio.h>
#define CB_DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define CB_DEBUG_LOG(...) ((void)0)
#endif

#define CB_COUNTOF(array) (sizeof(array) / sizeof(array[0]))

#define CB_MIN(a, b) ((a) < (b) ? (a) : (b))

#define CB_MAX(a, b) ((a) > (b) ? (a) : (b))

#ifdef HAVE___ATTRIBUTE__UNUSED
#define CB_UNUSED __attribute__((unused))
#elif defined(HAVE___PRAGMA_WARNING_SUPPRESS)
#define CB_UNUSED __pragma(warning(suppress:4100))
#else
#define CB_UNUSED
#endif

float cb_random_float_between(float min, float max);

int cb_random_int_between(int min, int max);

#endif
