/*
 * Touch screen input (iPhone / iPad). The game only knows a mouse, so a
 * touch screen is turned into one:
 *
 *   TAP mode (default): the pointer jumps to the finger and the left button
 *       is pressed while the finger is down (a tap is a click, a drag drags).
 *   TRACKPAD mode: dragging one finger moves the pointer by the distance the
 *       finger moved (like a laptop trackpad); a tap clicks where the pointer
 *       is; holding a finger down for a moment holds the button (so a drag
 *       can be made).
 *
 * In both modes a two-finger tap is a right click, a three-finger tap
 * switches between the modes and a four-finger tap shows or hides the
 * on-screen keyboard (the game asks for typed names). Only touch screens (direct touch devices) are
 * handled: a Mac trackpad still works as the mouse it already is.
 *
 * The rest of the program calls touch_poll_event() instead of SDL_PollEvent()
 * and touch_mouse_state() instead of SDL_GetMouseState(): finger events turn
 * into the mouse events and mouse state the game already understands.
 */
#ifndef SUP_TOUCH_H
#define SUP_TOUCH_H

#include <SDL3/SDL.h>

/* Call once after SDL_Init (before the window is made). */
void touch_init(void);

/* 1 when the device has a touch screen to offer the touch settings for:
 * iOS, or any system with the environment variable SUP_TOUCH set (tests). */
int  touch_present(void);

/* 0 = tap, 1 = trackpad */
void touch_set_mode(int trackpad);
int  touch_get_mode(void);

/* SDL_PollEvent, with touch screen fingers turned into mouse events. */
int  touch_poll_event(SDL_Event *ev);

/* SDL_GetMouseState, once a touch screen has been used the touch pointer. */
SDL_MouseButtonFlags touch_mouse_state(float *x, float *y);

/* 1 once after a four-finger tap (show or hide the keyboard). */
int  touch_take_keyboard(void);

/* After a three-finger tap changed the mode: the text to show once, else NULL. */
const char *touch_take_toast(void);

#endif
