#include "game/event.h"
#include "game/game.h"
#include "game/log.h"
#include "util/hash.h"
#include "util/memory.h"
#include "util/string.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* ----------------------------------------------------------------------------
 * Static functions
 * ------------------------------------------------------------------------- */

/*!
 * @brief Frees an event object.
 * @note This does not remove it from the list.
 */
static void
event_free(struct event_t* event);

/* ----------------------------------------------------------------------------
 * Exported functions
 * ------------------------------------------------------------------------- */
char
event_system_create(struct game_t* game)
{
    assert(game);

    bsthv_init(&game->events);

    return 1;
}

/* ------------------------------------------------------------------------- */
void
event_system_destroy(struct game_t* game)
{
    assert(game);

    /* Free each event object, but don't modify the container while doing so */
    BSTHV_FOR_EACH(&game->events, struct event_t, name, event)
        event_free(event);
    BSTHV_END_EACH
    /* Clear and free container */
    bsthv_clear_free(&game->events);
}

/* ------------------------------------------------------------------------- */
struct event_t*
event_register(struct game_t* game, const char* name)
{
    struct event_t* event;

    assert(game);
    assert(name);

    /* allocate and initialise event object */
    if(!(event = (struct event_t*)MALLOC(sizeof(struct event_t), "event_create()")))
        goto malloc_event_failed;

    /* listener container */
    unordered_vector_init(&event->listeners, sizeof(struct event_listener_t));

    /* event has reference to game object */
    event->game = game;

    /* copy name */
    if((event->name = malloc_string(name)) == NULL)
        goto copy_event_name_failed;

    /* create node in game's event directory and add event */
    if(!bsthv_insert(&game->events, name, event))
        goto add_event_to_game_failed;

    /* success! */
    return event;

    add_event_to_game_failed : free_string(event->name);
    copy_event_name_failed   : FREE(event);
    malloc_event_failed      : return NULL;
}

/* ------------------------------------------------------------------------- */
void
event_unregister(struct event_t* event)
{
    assert(event);
    assert(event->game);

    /*
     * The game object maintains a structure of event objects (using ptree).
     * Find the node in the ptree that is this event and delete it. This will
     * automatically also free the event object, because during creation,
     * ptree_set_free_func was set to event_free().
     */
    if(bsthv_erase_element(&event->game->events, event) == NULL)
    {
        log_message(LOG_ERROR, event->game, "Attempted to destroy the event"
            " \"%s\", but the associated game object with name \"%s\" doesn't "
            "own it! The event will not be destroyed.",
             event->name, event->game->name);
        return;
    }

    event_free(event);
}

/* ------------------------------------------------------------------------- */
struct event_t*
event_get(const struct game_t* game, const char* name)
{
    assert(game);
    assert(name);

    return bsthv_find(&game->events, name);
}

/* ------------------------------------------------------------------------- */
char
event_register_listener(struct event_t* event,
                        event_callback_func callback)
{
    struct event_listener_t* new_listener;

    assert(event);
    assert(callback);

    /* make sure listener hasn't already registered to this event */
    UNORDERED_VECTOR_FOR_EACH(&event->listeners, struct event_listener_t, listener)
        if(listener->callback == callback)
        {
            log_message(LOG_WARNING, event->game, "Already registered as a listener"
                " to event \"%s\"", event->name);
            return 0;
        }
    UNORDERED_VECTOR_END_EACH

    /* create event listener object */
    new_listener = (struct event_listener_t*)unordered_vector_push_emplace(&event->listeners);
    new_listener->callback = callback;

    return 1;
}

/* ------------------------------------------------------------------------- */
char
event_unregister_listener(struct event_t* event,
                          event_callback_func callback)
{
    assert(event);
    assert(callback);

    UNORDERED_VECTOR_FOR_EACH(&event->listeners, struct event_listener_t, listener)
        if(listener->callback == callback)
        {
            unordered_vector_erase_element(&event->listeners, listener);
            return 1;
        }
    UNORDERED_VECTOR_END_EACH

    log_message(LOG_WARNING, event->game, "Tried to unregister from event \"%s\", but "
        "the listener was not found.", event->name);

    return 0;
}

/* ------------------------------------------------------------------------- */
void
event_unregister_all_listeners(struct event_t* event)
{
    unordered_vector_clear_free(&event->listeners);
}

/* ----------------------------------------------------------------------------
 * Static functions
 * ------------------------------------------------------------------------- */
static void
event_free(struct event_t* event)
{
    assert(event);
    assert(event->name);

    free_string(event->name);
    unordered_vector_clear_free(&event->listeners);

    FREE(event);
}
