#include "game/game.h"
#include "game/event.h"
#include "game/renderer.h"
#include "util/memory.h"
#include "util/string.h"
#include <assert.h>

/* ------------------------------------------------------------------------- */
struct game_t*
game_create(const char* game_name)
{
    struct game_t* game;
    if((game = (struct game_t*)MALLOC(sizeof *game, "game_create()")) == NULL)
        goto malloc_game_failed;

    if((game->name = malloc_string(game_name)) == NULL)
        goto copy_game_name_failed;

    if(!event_system_create(game))
        goto create_event_system_failed;

    if((game->renderer = renderer_create(game)) == NULL)
        goto create_renderer_failed;

    return game;

    create_renderer_failed     : event_system_destroy(game);
    create_event_system_failed : free_string(game->name);
    copy_game_name_failed      : FREE(game);
    malloc_game_failed         : return NULL;
}

/* ------------------------------------------------------------------------- */
void
game_destroy(struct game_t* game)
{
    assert(game);

    renderer_destroy(game->renderer);
    event_system_destroy(game);
    free_string(game->name);

    FREE(game);
}
