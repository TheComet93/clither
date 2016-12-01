#include "game/config.h"
#include "util/bst_hashed_vector.h"

C_HEADER_BEGIN

struct event_system_t;

struct game_t
{
    char* name;
    struct bsthv_t events;
};

GAME_PUBLIC_API struct game_t*
game_create(const char* game_name);

GAME_PUBLIC_API void
game_destroy(struct game_t* game);

C_HEADER_END
