/*
 * Supremacy port - the launcher: a small options window shown when the
 * program is started with no arguments (from its icon), with the settings
 * the command line offers and a wide PLAY GAME button. It is drawn with
 * SDL's built-in 8x8 text font, so it works before the game data is found.
 * The settings are kept in supremacy.cfg next to the program.
 */
#ifndef SUP_LAUNCHER_H
#define SUP_LAUNCHER_H

#include <SDL3/SDL.h>

/* The window title and settings folder name: the separate SupremacyHD app
 * (cmake -DSUP_HD=ON) or the plain game. */
#ifdef SUP_HD
#define APP_TITLE "SupremacyHD"
#define PREF_APP "SupremacyHD"
#else
#define APP_TITLE "Supremacy"
#define PREF_APP "Supremacy"
#endif

typedef struct Settings {
    char data[1024];        /* the SupremacyData folder */
    int  intro;             /* 1: start with the intro (pick the opponent there) */
    int  opponent;          /* 0-3, without the intro */
    int  speed;             /* 0 NORMAL (47 passes/s, the original), 1-4: x1.25 .. x2 */
    int  scale;             /* window size: scale x 320 by scale x 200 */
    int  fullscreen;
    int  trainer;           /* 1: the cheat keys F5-F7 work (F8 switches in the game) */
    int  bundled;           /* 1: the game files are inside the app, so no folder is asked for (not saved) */
    int  bugfixes;          /* 1: the original's bugs are fixed (see README, "Bug fixes") */
    int  display;           /* 0 PIXEL (sharp, whole-number scaling), 1 SMOOTH (fills the window), 2 4:3 (Amiga monitor shape; F12) */
    int  hires;             /* 1: use a HiRes picture pack when there is one (default on) */
    int  scanlines;         /* 1: darken the lower part of every picture line (F10 in the game) */
    char replay[1024];      /* a session log to replay, or "" (not remembered) */
    int  win_w, win_h;      /* the launcher window's size (default 1280 x 800) */
    int  load_slot;         /* LOAD SAVED GAME: the slot 0-3 picked (not remembered) */
} Settings;

/* `base` is the program's folder: the default data folder and the one the
 * extractor writes are <base>SupremacyData. */
void settings_defaults(Settings *s, const char *base);
/* passes per second for a speed setting */
double settings_passes(int speed);
void settings_load(Settings *s, const char *path);
int  settings_save(const Settings *s, const char *path);

/* 1 = PLAY NEW GAME, 2 = LOAD SAVED GAME (s->load_slot), 3 = CREDITS, 0 = the
 * window was closed. The saves are SSG1-SSG4 in <base>save. */
/* The trainer's keys on a cracktro-style screen with an OK button (shown when
 * the trainer is switched on), drawn with `ren`, whose state is restored. */
void trainer_help(SDL_Window *win, SDL_Renderer *ren);

int  launcher_run(SDL_Window *win, SDL_Renderer *ren, Settings *s, const char *base);

#endif
