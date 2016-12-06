#include "util/config.h"
#include "util/unordered_vector.h"

C_HEADER_BEGIN

struct ring_buffer_t
{
    struct unordered_vector_t vector;
    uint32_t read;
    uint32_t write;
};

UTIL_PUBLIC_API struct ring_buffer_t*
ring_buffer_create(uint32_t element_size);

UTIL_PUBLIC_API void
ring_buffer_init(struct ring_buffer_t* rb, uint32_t element_size);

UTIL_PUBLIC_API void
ring_buffer_destroy(struct ring_buffer_t* rb);

UTIL_PUBLIC_API void
ring_buffer_clear(struct ring_buffer_t* rb);

UTIL_PUBLIC_API void
ring_buffer_clear_free(struct ring_buffer_t* rb);

UTIL_PUBLIC_API void
ring_buffer_queue(struct ring_buffer_t* rb, void* data);

UTIL_PUBLIC_API void*
ring_buffer_peak(struct ring_buffer_t* rb);

C_HEADER_END
