#include "gmock/gmock.h"
#include "game/event.h"
#include "game/game.h"

#define NAME event

using namespace ::testing;

TEST(NAME, create_destroy)
{
    /* Event system only uses game->events and nothing else. No need to create
     * a game object */
    game_t game;
    EXPECT_THAT(event_system_create(&game), Ne(0));
    event_system_destroy(&game);
}

TEST(NAME, register_and_unregister_events)
{
    game_t game;
    ASSERT_THAT(event_system_create(&game), Ne(0));

    event_t* event = event_register(&game, "evt1");
    ASSERT_THAT(event, NotNull());
    EXPECT_THAT(event_get(&game, "evt1"), Eq(event));
    EXPECT_THAT(event_get(&game, "shit"), IsNull());

    event_t* event2 = event_register(&game, "evt2");
    ASSERT_THAT(event2, NotNull());
    EXPECT_THAT(event_get(&game, "evt2"), Eq(event2));

    event_unregister(event);
    EXPECT_THAT(event_get(&game, "evt1"), IsNull());
    EXPECT_THAT(event_get(&game, "evt2"), Eq(event2));

    event_unregister(event2);
    EXPECT_THAT(event_get(&game, "evt1"), IsNull());
    EXPECT_THAT(event_get(&game, "evt2"), IsNull());

    event_system_destroy(&game);
}

static unsigned g_counter1 = 0;
static void listener1(event_t* event, void* data)
{
    g_counter1++;
}
static unsigned g_counter2 = 0;
static void listener2(event_t* event, void* data)
{
    g_counter2++;
}
TEST(NAME, register_and_unregister_listeners)
{
    game_t game;
    ASSERT_THAT(event_system_create(&game), Ne(0));

    event_t* event = event_register(&game, "event");
    ASSERT_THAT(event, NotNull());

    event_register_listener(event, listener1);
    event_register_listener(event, listener2);
    event_unregister_listener(event, listener1);
    event_unregister_listener(event, listener2);

    event_unregister(event);
    event_system_destroy(&game);
}

TEST(NAME, fire_to_multiple_listeners)
{
    game_t game;
    ASSERT_THAT(event_system_create(&game), Ne(0));

    event_t* event = event_register(&game, "event");
    ASSERT_THAT(event, NotNull());

    event_register_listener(event, listener1);
    event_register_listener(event, listener2);
    g_counter1 = 0; g_counter2 = 0;

    event_fire(event, NULL);
    EXPECT_THAT(g_counter1, Eq(1));
    EXPECT_THAT(g_counter2, Eq(1));
    event_unregister_listener(event, listener1);
    event_fire(event, NULL);
    EXPECT_THAT(g_counter1, Eq(1));
    EXPECT_THAT(g_counter2, Eq(2));

    event_unregister_listener(event, listener2);
    event_unregister(event);
    event_system_destroy(&game);
}

TEST(NAME, cleanup)
{

}
