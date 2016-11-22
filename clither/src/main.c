#include <stdio.h>
#include "clither/argv.h"
#include "util/memory.h"

int
main(int argc, char** argv)
{
    struct arg_obj_t* args;

    memory_init();

    /* parse command line arguments */
    args = argv_parse(argc, argv);

    /* clean up */
    argv_free(args);
    memory_deinit();

    return 0;
}
