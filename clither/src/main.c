#include <stdio.h>
#include "util/memory.h"
#include "clither/argv.h"
#include "game/renderer.h"

int
main(int argc, char** argv)
{
    struct arg_obj_t* args;
    struct renderer_t* renderer;

    memory_init();
    renderer = renderer_create();

    /* parse command line arguments */
    args = argv_parse(argc, argv);

    /* clean up */
    argv_free(args);
    renderer_destroy(renderer);
    memory_deinit();

    return 0;
}
