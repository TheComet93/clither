#include "game/config.h"

C_HEADER_BEGIN

struct renderer_t;

struct renderer_t*
renderer_create(void);

char
renderer_init(struct renderer_t* renderer);

void
renderer_destroy(struct renderer_t* renderer);

C_HEADER_END
