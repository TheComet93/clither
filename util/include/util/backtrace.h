#ifndef UTIL_BACKTRACE_H
#define UTIL_BACKTRACE_H

#define BACKTRACE_SIZE 64

#include "util/pstdint.h"
#include "util/config.h"

C_HEADER_BEGIN

/*!
 * @brief Generates a backtrace.
 * @param[in] size The maximum number of frames to walk.
 * @return Returns an array of char* arrays.
 * @note The returned array must be freed manually with FREE(returned_array).
 */
UTIL_PUBLIC_API char**
get_backtrace(int* size);

C_HEADER_END

#endif /* UTIL_BACKTRACE_H */
