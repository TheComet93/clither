#include <stdio.h>
#include "util/memory.h"
#include "clither/argv.h"
#include "game/game.h"

int
main(int argc, char** argv)
{
    struct arg_obj_t* args;
    struct game_t* game;

    memory_init();

    /* parse command line arguments */
    args = argv_parse(argc, argv);

    game = game_create("game");
    if(game)
        game_destroy(game);

    /* clean up */
    argv_free(args);
    memory_deinit();

    return 0;
}
