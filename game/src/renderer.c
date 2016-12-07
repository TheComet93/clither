#include "game/renderer_impl.h"
#include "game/renderer_vulkan.h"
#include "game/log.h"
#include "util/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
struct renderer_t*
renderer_create(struct game_t* game)
{
    struct renderer_t* renderer;
    if(!(renderer = (struct renderer_t*)MALLOC(sizeof *renderer, "renderer_create()")))
        return NULL;

    if(!renderer_init(renderer, game))
    {
        FREE(renderer);
        return NULL;
    }

    return renderer;
}

/* ------------------------------------------------------------------------- */
char
renderer_init(struct renderer_t* renderer, struct game_t* game)
{
    assert(renderer);

    /* most functions rely on unused pointers to be NULL */
    memset(renderer, 0, sizeof *renderer);

    renderer->game = game;
    renderer->width = 800;
    renderer->height = 600;

    if(!vulkan_load_library(renderer))
        goto load_vulkan_library_failed;

    if(!vulkan_create_instance(renderer))
        goto create_vulkan_instance_failed;

    return 1;

    create_vulkan_instance_failed : vulkan_unload_library(renderer);
    load_vulkan_library_failed    : return 0;
}

/* ------------------------------------------------------------------------- */
void
renderer_destroy(struct renderer_t* renderer)
{
    assert(renderer);

    vulkan_destroy_instance(renderer);
    vulkan_unload_library(renderer);

    FREE(renderer);
}
