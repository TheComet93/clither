#include "util/ring_buffer.h"
#include "util/memory.h"
#include <assert.h>

/* ------------------------------------------------------------------------- */
struct ring_buffer_t*
ring_buffer_create(uint32_t element_size)
{
    struct ring_buffer_t* rb;
    assert(element_size > 0);

    if((rb = (struct ring_buffer_t*)MALLOC(sizeof *rb, "ring_buffer_create()")) == NULL)
        return NULL;

    ring_buffer_init(rb, element_size);

    return rb;
}

/* ------------------------------------------------------------------------- */
void
ring_buffer_init(struct ring_buffer_t* rb, uint32_t element_size)
{
    unordered_vector_init(&rb->vector, element_size);
    rb->read = 0;
    rb->write = 1;
}

/* ------------------------------------------------------------------------- */
void
ring_buffer_destroy(struct ring_buffer_t* rb)
{
    assert(rb);
    ring_buffer_clear_free(rb);
    FREE(rb);
}

/* ------------------------------------------------------------------------- */
void
ring_buffer_clear(struct ring_buffer_t* rb)
{
    unordered_vector_clear(&rb->vector);
    rb->read = 0;
    rb->write = 1;
}

/* ------------------------------------------------------------------------- */
void
ring_buffer_clear_free(struct ring_buffer_t* rb)
{
    unordered_vector_clear_free(&rb->vector);
}
