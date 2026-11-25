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

/* Peace: while on, the enemy sends no attacks at all. Turning it on also
 * removes enemy forces already sitting on your planets. Returns the number of
 * planets cleared (0 when turning it off). */
SUP_API int cheat_peace(Game *g, int on);

/* Returns the number of planets changed (0: none of yours, or an unknown cheat). */
SUP_API int cheat_apply(Game *g, int which);

#endif
