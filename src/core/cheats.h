/*
 * Supremacy port - optional cheats (not in the original).
 *
 * Each cheat edits the player's planets (owner 10) in the saved state block
 * and asks the screens to redraw. The port's keys: F5 credits, F6 resources,
 * F7 people (population and morale); see main.c.
 */
#ifndef SUP_CHEATS_H
#define SUP_CHEATS_H

#include "game.h"

enum {
    CHEAT_CREDITS = 0,      /* +100,000 credits on every planet of yours */
    CHEAT_RESOURCES,        /* food, minerals, fuels and energy topped up to 30,000 */
    CHEAT_PEOPLE            /* morale 100, population doubled (at most 30,000) */
};

/* Returns the number of planets changed (0: none of yours, or an unknown cheat). */
SUP_API int cheat_apply(Game *g, int which);

#endif
