#include "game/config.h"

C_HEADER_BEGIN

struct game_t;
struct renderer_t;

struct renderer_t*
renderer_create(struct game_t* game);

char
renderer_init(struct renderer_t* renderer, struct game_t* game);

void
renderer_destroy(struct renderer_t* renderer);

C_HEADER_END
