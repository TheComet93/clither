#include "game/config.h"
#include "util/unordered_vector.h"

C_HEADER_BEGIN

struct event_t;
struct event_data_t;
struct game_t;

typedef void (*event_callback_func)(struct event_t*, struct event_data_t*);

struct event_t
{
    char* directory;
    struct game_t* game;
    struct unordered_vector_t listeners;   /* holds event_listener_t objects */
};

struct event_listener_t
{
    event_callback_func callback;
};

/*!
 * @brief Initialises the event system.
 * @note Must be called before calling any other event related functions.
 */
GAME_PUBLIC_API char
event_system_init(struct game_t* game);

GAME_PUBLIC_API void
event_system_deinit(struct game_t* game);

/*!
 * @brief Creates and registers a new event in the host program.
 *
 * @param[in] game The game object this event should be created in.
 * @param[in] directory The name of the event. Should be unique per game
 * instance.
 * @note The name string is copied to an internal buffer, so you are free to
 * delete it when it is no longer used.
 * @return Returns a new event object which should be later deleted with
 * event_destroy().
 */
GAME_PUBLIC_API struct event_t*
event_create(struct game_t* game,
             const char* directory);


/*!
 * @brief Destroys an event object.
 * @note This also destroys all registered event listeners and removes it from
 * the assigned game object.
 * @param[in] event The event object to destroy.
 * @return Returns 1 if successful, 0 if otherwise.
 */
GAME_PUBLIC_API void
event_destroy(struct event_t* event);

/*!
 * @brief Destroys all events that were registered by the specified plugin.
 * @note This also destroys all registered event listeners.
 * @param[in] plugin The plugin to destroy the events from.
 */
/* TODO implement */
GAME_PUBLIC_API void
event_destroy_all_matching(const char* pattern);

/*!
 * @brief Returns an event object with the specified name.
 * @return If the event object does not exist, NULL is returned, otherwise the
 * event object is returned.
 */
GAME_PUBLIC_API struct event_t*
event_get(const struct game_t* game, const char* directory);

/*!
 * @brief Registers a listener to the specified event.
 * @note The same callback function will not be registered twice.
 * @param[in] game The game hosting the event you want to listen to.
 * @param[in] event_name The name of the event to register to.
 * @param[in] callback The callback function to call when the event is fired.
 */
GAME_PUBLIC_API char
event_register_listener(const struct game_t* game,
                        const char* event_directory,
                        event_callback_func callback);

/*!
 * @brief Unregisters a listener from the specified event.
 */
GAME_PUBLIC_API char
event_unregister_listener(const struct game_t* game,
                          const char* event_directory,
                          event_callback_func callback);

/*!
 * @brief Unregisters all listeners from the specified event.
 */
GAME_PUBLIC_API void
event_unregister_all_listeners(struct event_t* event);


C_HEADER_END
