#ifndef UTIL_MEMORY_H
#define UTIL_MEMORY_H

#include "util/pstdint.h"
#include "util/config.h"

C_HEADER_BEGIN
#ifdef ENABLE_MEMORY_DEBUGGING
#   define MALLOC(size, where) malloc_wrapper_debug(size, "malloc() failed in " where " - not enough memory")
#   define FREE free_wrapper_debug
#else
#   define MALLOC(size, where) malloc_wrapper(size, "malloc() failed in " where " - not enough memory")
#   define FREE free free
#endif

/*!
 * @brief Initialises the memory system.
 *
 * In release mode this does nothing. In debug mode it will initialise
 * memory reports and backtraces, if enabled.
 */
UTIL_PUBLIC_API void
memory_init(void);

/*!
 * @brief De-initialises the memory system.
 *
 * In release mode this does nothing. In debug mode this will output the memory
 * report and print backtraces, if enabled.
 * @return Returns the number of memory leaks.
 */
UTIL_PUBLIC_API uintptr_t
memory_deinit(void);

#ifdef ENABLE_MEMORY_DEBUGGING
/*!
 * @brief Does the same thing as a normal call to malloc(), but does some
 * additional work monitor and track down memory leaks.
 */
UTIL_PUBLIC_API void*
malloc_wrapper_debug(uintptr_t size, const char* msg);

UTIL_PUBLIC_API void*
malloc_wrapper(uintptr_t size, const char* msg);

/*!
 * @brief Does the same thing as a normal call to fee(), but does some
 * additional work monitor and track down memory leaks.
 */
UTIL_PUBLIC_API void
free_wrapper_debug(void* ptr);

#   ifdef ENABLE_MEMORY_EXPLICIT_MALLOC_FAILURES
/*!
 * @brief Causes the next call to MALLOC() to fail.
 * @note Because unit tests are executed in parallel, this function will
 * acquire a mutex, which will be released again when force_malloc_fail_off()
 * is called.
 */
UTIL_PUBLIC_API void
force_malloc_fail_on(void);

/*!
 * @brief Causes all calls to malloc() after the nth call to fail.
 * @note Because unit tests are executed in parallel, this function will
 * acquire a mutex, which will be released again when force_malloc_fail_off()
 * is called.
 * @param num_allocations After how many allocations malloc() should begin to
 * fail. 1 means instantly, 2 means after one call, etc.
 */
UTIL_PUBLIC_API void
force_malloc_fail_after(int num_allocations);

/*!
 * @brief Allows the next call to MALLOC() to function normally again.
 * @note Because unit tests are executed in parallel, this function releases
 * the mutex previously acquired in force_malloc_fail_on().
 */
UTIL_PUBLIC_API void
force_malloc_fail_off(void);
#   endif /* ENABLE_MEMORY_EXPLICIT_MALLOC_FAILURES */


#endif /* ENABLE_MEMORY_DEBUGGING */

UTIL_PUBLIC_API void
mutated_string_and_hex_dump(void* data, intptr_t size_in_bytes);

C_HEADER_END

#endif /* UTIL_MEMORY_H */
