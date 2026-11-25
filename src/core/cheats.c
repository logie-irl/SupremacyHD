/* Optional cheats; see cheats.h. */
#include "cheats.h"

#define CHEAT_CREDITS_ADD 100000u
#define CHEAT_RESOURCE_TO 30000u
#define POP_MAX           30000u
#define REDRAW_ALL        0x7D3F6u      /* the bits the screens' full refresh sets */

static void top_up(Game *g, uint32_t p, unsigned off, uint16_t to)
{
    if (g16(g, p + off) < to) s16(g, p + off, to);
}

int cheat_apply(Game *g, int which)
{
    if (which < 0 || which > CHEAT_PEOPLE) return 0;
    int changed = 0;
    uint32_t p = g32(g, G_PLANET_TABLE);
    for (unsigned k = 0; k <= g16(g, 0xE410); k++, p += PLANET_SIZE) {
        if (g16(g, p + P_OWNER) != OWNER_PLAYER) continue;
        switch (which) {
        case CHEAT_CREDITS: {
            uint32_t c = g32(g, p + P_CREDITS) + CHEAT_CREDITS_ADD;
            s32(g, p + P_CREDITS, c < g32(g, p + P_CREDITS) ? 0xFFFFFFFFu : c);   /* no wrap */
            break;
        }
        case CHEAT_RESOURCES:
            top_up(g, p, P_FOOD, CHEAT_RESOURCE_TO);
            top_up(g, p, P_MINERALS, CHEAT_RESOURCE_TO);
            top_up(g, p, P_FUELS, CHEAT_RESOURCE_TO);
            top_up(g, p, P_ENERGY, CHEAT_RESOURCE_TO);
            break;
        case CHEAT_PEOPLE: {
            uint32_t pop = (uint32_t)g16(g, p + P_POP) * 2u;
            s16(g, p + P_POP, (uint16_t)(pop > POP_MAX ? POP_MAX : pop));
            s8(g, p + P_MORALE, 100);
            break;
        }
        }
        changed++;
    }
    if (changed) or32(g, G_REDRAW, REDRAW_ALL);
    return changed;
}
