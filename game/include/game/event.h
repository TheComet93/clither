#include "game/config.h"
#include "util/bst_hashed_vector.h"
#include "util/unordered_vector.h"

C_HEADER_BEGIN

struct event_t;
struct game_t;

typedef void (*event_callback_func)(struct event_t*, void*);

struct event_listener_t
{
    event_callback_func callback;
};

struct event_t
{
    char* name;
    struct game_t* game;
    struct unordered_vector_t listeners;   /* holds event_listener_t objects */
};

struct event_system_t
{
    struct game_t* game;
    struct bsthv_t events;
};

/*!
 * @brief Creates a new event system for the specified game instance.
 * This must be called before any events can be fired.
 * @return Returns 1 if successful, 0 if otherwise.
 */
GAME_PUBLIC_API char
event_system_create(struct game_t* game);

/*!
 * @brief Destroys an event system. This will unregister all listeners and
 * destroy all registered events.
 */
GAME_PUBLIC_API void
event_system_destroy(struct game_t* game);

/*!
 * @brief Creates and registers a new event.
 *
 * Before events can be fired, they first need to be registered to the game
 * instance  using a unique name. An event object is returned, which can be
 * used to fire events. Or you can just ignore the returned object and
 * retrieve it later with event_get().
 *
 * @param[in] game The game this event should be created in.
 * @param[in] name A unique name of the event. Can be used by other parts of
 * the program to retrieve an event object with event_get().
 * @note The name string is copied to an internal buffer, so you are free to
 * delete it when it is no longer used.
 * @return Returns a new event object which should be later deleted with
 * event_unregister() when no longer required.
 */
GAME_PUBLIC_API struct event_t*
event_register(struct game_t* game, const char* name);

/*!
 * @brief Unregisters and destroys an event object.
 * @note This also destroys all registered event listeners and removes it from
 * the assigned game object.
 * @warning Make sure other parts of the program are NOT holding pointers to a
 * destroyed event object.
 * @param[in] event The event object to destroy.
 */
GAME_PUBLIC_API void
event_unregister(struct event_t* event);

/*!
 * @brief Returns an event object matching the specified name.
 * @return If the event object does not exist, NULL is returned, otherwise the
 * event object is returned.
 */
GAME_PUBLIC_API struct event_t*
event_get(const struct game_t* game, const char* name);

/*!
 * @brief Registers a listener to the specified event.
 * @note The same callback function will not be registered twice.
 * @param[in] event_system The game hosting the event you want to listen to.
 * @param[in] event The event object to register to.
 * @param[in] callback The callback function to call when the event is fired.
 */
GAME_PUBLIC_API char
event_register_listener(struct event_t* event,
                        event_callback_func callback);

/*!
 * @brief Unregisters a listener from the specified event.
 */
GAME_PUBLIC_API char
event_unregister_listener(struct event_t* event,
                          event_callback_func callback);

/*!
 * @brief Unregisters all listeners from the specified event.
 */
GAME_PUBLIC_API void
event_unregister_all_listeners(struct event_t* event);

#define event_fire(event, data) do {                   \
        UNORDERED_VECTOR_FOR_EACH(&(event)->listeners, struct event_listener_t, listener_##event) \
            (listener_##event)->callback(event, data); \
        UNORDERED_VECTOR_END_EACH                      \
    } while(0)


C_HEADER_END
