#pragma once

#define CARVE_VERSION "2.1.0"
#define CARVE_VERSION_MAJOR 2
#define CARVE_VERSION_MINOR 1
#define CARVE_VERSION_PATCH 0

/* #undef CARVE_STATIC */
/* #undef CARVE_USE_TIMINGS */
/* #undef CARVE_NODEBUG */
/* #undef CARVE_DEBUG */
/* #undef CARVE_DEBUG_WRITE_PLY_DATA */

#define CARVE_USE_EXACT_PREDICATES

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif
