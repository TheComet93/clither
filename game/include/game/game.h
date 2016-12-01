#include "game/config.h"
#include "util/ptree.h"

C_HEADER_BEGIN

struct game_t
{
    char* name;
    struct ptree_t events;     /* directory of events for this game instance */
};

C_HEADER_END
