#include "game/game.h"
#include "game/event.h"
#include "util/memory.h"
#include <assert.h>

/* ------------------------------------------------------------------------- */
struct game_t*
game_create(const char* game_name)
{
    struct game_t* game;
    if((game = (struct game_t*)MALLOC(sizeof *game, "game_create()")) == NULL)
        goto malloc_game_failed;

    if(!event_system_create(game))
        goto create_event_system_failed;

    create_event_system_failed : FREE(game);
    malloc_game_failed         : return NULL;
}

/* ------------------------------------------------------------------------- */
void
game_destroy(struct game_t* game)
{
    assert(game);

    event_system_destroy(game);

    FREE(game);
}
