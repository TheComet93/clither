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

/*!
 * @brief Checks if the name of the event directory is valid or not.
 * Should be in the form of "my.event.name"
 */
static char
directory_name_is_valid(const char* directory);

/* ----------------------------------------------------------------------------
 * Exported functions
 * ------------------------------------------------------------------------- */
char
event_system_init(struct game_t* game)
{
    assert(game);

    /* this holds all of the game's events */
    ptree_init(&game->events, NULL);
}

/* ------------------------------------------------------------------------- */
void
event_system_deinit(struct game_t* game)
{
    assert(game);
    ptree_destroy_keep_root(&game->events);
}

/* ------------------------------------------------------------------------- */
struct event_t*
event_create(struct game_t* game, const char* directory)
{
    struct ptree_t* node;
    struct event_t* event;

    assert(game);
    assert(directory);

    /* make sure directory contains valid characters only */
    if(!directory_name_is_valid(directory))
    {
        log_message(LOG_ERROR, game, "Invalid event directory name %s", directory);
        return NULL;
    }

    /* allocate and initialise event object */
    if(!(event = (struct event_t*)MALLOC(sizeof(struct event_t))))
        OUT_OF_MEMORY("event_create()", malloc_event_failed);
    memset(event, 0, sizeof(struct event_t));

    unordered_vector_init(&event->listeners, sizeof(struct event_listener_t));

    /* copy directory */
    if(!(event->directory = malloc_string(directory)))
        goto set_directory_failed;

    /* create node in game's event directory and add event */
    if(!(node = ptree_set(&game->events, directory, event)))
        goto add_event_to_game_directory_failed;

    /* set the node's free function to event_free() to make deleting nodes
     * easier */
    ptree_set_free_func(node, (ptree_free_func)event_free);

    /* success! */
    return event;

    add_event_to_game_directory_failed : free_string(event->directory);
    set_directory_failed               : FREE(event);
    malloc_event_failed                : return NULL;
}

/* ------------------------------------------------------------------------- */
void
event_destroy(struct event_t* event)
{
    struct ptree_t* node;

    assert(event);
    assert(event->game);
    assert(event->directory);

    /*
     * The game object maintains a structure of event objects (using ptree).
     * Find the node in the ptree that is this event and delete it. This will
     * automatically also free the event object, because during creation,
     * ptree_set_free_func was set to event_free().
     */
    if(!(node = ptree_get_node(&event->game->events, event->directory)))
    {
        log_message(LOG_ERROR, event->game, "Attempted to destroy the event"
            " \"%s\", but the associated game object with name \"%s\" doesn't "
            "own it! The event will not be destroyed.",
             event->directory, event->game->name);
        return;
    }

    /* destroying the node will call event_free() automatically */
    ptree_destroy(node);
}

/* ------------------------------------------------------------------------- */
struct event_t*
event_get(const struct game_t* game, const char* directory)
{
    struct ptree_t* node;

    assert(game);
    assert(directory);

    if(!(node = ptree_get_node(&game->events, directory)))
        return NULL;
    /* The node can be NULL if the node is a "middle node". This doesn't
     * concern us, though, because we would be returning NULL anyway */
    return (struct event_t*)node->value;
}

/* ------------------------------------------------------------------------- */
char
event_register_listener(const struct game_t* game,
                        const char* event_directory,
                        event_callback_func callback)
{
    struct event_t* event;
    struct event_listener_t* new_listener;

    assert(game);
    assert(event_directory);
    assert(callback);

    /* make sure event exists */
    if(!(event = event_get(game, event_directory)))
    {
        log_message(LOG_WARNING, game, "Tried to register as a listener to "
            "event \"%s\", but the event does not exist.", event_directory);
        return 0;
    }

    /* make sure listener hasn't already registered to this event */
    UNORDERED_VECTOR_FOR_EACH(&event->listeners, struct event_listener_t, listener)
        if(listener->callback == callback)
        {
            log_message(LOG_WARNING, game, "Already registered as a listener"
                " to event \"%s\"", event->directory);
            return 0;
        }
    UNORDERED_VECTOR_END_EACH

    /* create event listener object */
    new_listener = (struct event_listener_t*) unordered_vector_push_emplace(&event->listeners);
    new_listener->callback = callback;

    return 1;
}

/* ------------------------------------------------------------------------- */
char
event_unregister_listener(const struct game_t* game,
                          const char* event_directory,
                          event_callback_func callback)
{
    struct event_t* event;

    if(!(event = event_get(game, event_directory)))
    {
        log_message(LOG_WARNING, game, "Tried to unregister from event \"%s\", "
            "but the event does not exist.", event_directory);
        return 0;
    }

    UNORDERED_VECTOR_FOR_EACH(&event->listeners, struct event_listener_t, listener)
        if(listener->callback == callback)
        {
            unordered_vector_erase_element(&event->listeners, listener);
            return 1;
        }
    UNORDERED_VECTOR_END_EACH

    log_message(LOG_WARNING, game, "Tried to unregister from event \"%s\", but "
        "the listener was not found.", event_directory);

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
    event_unregister_all_listeners(event);
    free_string(event->directory);
    unordered_vector_clear_free(&event->listeners);
    FREE(event);
}

/* ------------------------------------------------------------------------- */
static char
directory_name_is_valid(const char* directory)
{
    const char* p;
    char c;
    for(p = directory; (c = *p); ++p)
    {
        if((c < 48 || c > 57)  &&   /* 0-9 */
           (c < 65 || c > 90)  &&   /* A-Z */
           (c < 97 || c > 122) &&   /* a-z */
           (c != '_')          &&   /* underscore */
           (c != ptree_node_delim)) /* namespace delimiters */
        {
            return 0;
        }

    }

    return 1;
}
