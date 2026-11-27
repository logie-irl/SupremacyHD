/* Hi-res picture packs: replacement backdrops for the nine full-screen pictures.
 *
 * A pack is a folder of images named main, statistics, shipyard, fleet, troops,
 * cargo, surface, combat and title (.png, .jpg or .bmp; any size, 8:5 shape is
 * best, 1280x800 or 1600x1000 is plenty). Where a screen's pixels are still the
 * backdrop's, the game shows the hi-res picture; text, icons, panels that the
 * game draws over it, and the pointer stay as the game draws them.
 *
 * `--export-pictures DIR` writes the original nine pictures for upscaling. */
#ifndef HIRES_H
#define HIRES_H

#include <SDL3/SDL.h>
#include "gamedata.h"

/* Load the pack found in `dir_a` or else `dir_b` (either may be NULL).
 * Returns how many pictures were loaded. */
int hires_load(SDL_Renderer *ren, const char *dir_a, const char *dir_b);
void hires_free(void);

/* The hi-res texture for the picture at original address `addr`, or NULL. */
SDL_Texture *hires_find(uint32_t addr);

/* Write the nine original pictures into `dir` (PNG, or BMP with an old SDL).
 * Returns how many were written, or -1. */
int hires_export(const GameData *gd, const char *dir);

#endif
