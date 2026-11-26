/* Touch screen input: see touch.h. */
#include "touch.h"

#define MAXQ        32
#define MAXF        4
#define PRESS_DELAY 70      /* ms: tap mode waits a moment so a second finger can make it a right click */
#define HOLD_MS     400     /* ms: trackpad mode: a finger held still presses the button (drag) */
#define TAP_MS      350     /* ms: the longest tap */
#define MOVE_PX     10.0f   /* window points: more than this and it is a drag, not a tap */
#define GAIN        1.6f    /* trackpad mode: pointer distance per finger distance */

typedef struct {
    int used;
    SDL_FingerID id;
    float x, y;             /* window points */
} Finger;

static SDL_Event q[MAXQ];
static int qn, qh;

static int mode;                    /* 0 tap, 1 trackpad */
static int active;                  /* a touch screen has been used: the touch pointer is the mouse */
static float vx = 160, vy = 100;    /* the touch pointer, window points */
static int vleft, vright;
static SDL_WindowID vwin;
static float ww = 320, wh = 200;    /* the window's size, window points */

static Finger fing[MAXF];
static int nf, maxf;                /* fingers down now / most at once in this gesture */
static Uint64 t0;                   /* when the gesture began (ms) */
static float travel;                /* how far the first finger moved (window points) */
static int pressed;                 /* the left button was pressed by this gesture */
static const char *toast;
static int keyboard;

void touch_init(void)
{
#ifdef SDL_PLATFORM_IOS
    /* the fingers are turned into mouse events here, not by SDL */
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
#endif
}

int touch_present(void)
{
#ifdef SDL_PLATFORM_IOS
    return 1;
#else
    return SDL_getenv("SUP_TOUCH") != NULL;
#endif
}

void touch_set_mode(int trackpad) { mode = trackpad != 0; }
int  touch_get_mode(void) { return mode; }

static void push(const SDL_Event *e)
{
    if (qn < MAXQ) q[(qh + qn++) % MAXQ] = *e;
}

static void emit_motion(float dx, float dy)
{
    SDL_Event e;
    SDL_zero(e);
    e.motion.type = SDL_EVENT_MOUSE_MOTION;
    e.motion.windowID = vwin;
    e.motion.which = SDL_TOUCH_MOUSEID;
    e.motion.state = (vleft ? SDL_BUTTON_LMASK : 0) | (vright ? SDL_BUTTON_RMASK : 0);
    e.motion.x = vx;
    e.motion.y = vy;
    e.motion.xrel = dx;
    e.motion.yrel = dy;
    push(&e);
}

static void emit_button(int button, int down)
{
    SDL_Event e;
    SDL_zero(e);
    e.button.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
    e.button.windowID = vwin;
    e.button.which = SDL_TOUCH_MOUSEID;
    e.button.button = (Uint8)button;
    e.button.down = down != 0;
    e.button.clicks = 1;
    e.button.x = vx;
    e.button.y = vy;
    if (button == SDL_BUTTON_LEFT) vleft = down;
    else vright = down;
    push(&e);
}

static void place(float x, float y)
{
    x = SDL_clamp(x, 0, ww - 1);
    y = SDL_clamp(y, 0, wh - 1);
    float dx = x - vx, dy = y - vy;
    vx = x;
    vy = y;
    emit_motion(dx, dy);
}

static void click(int button)
{
    emit_button(button, 1);
    emit_button(button, 0);
}

/* the timers of a gesture that is under way */
static void timers(void)
{
    if (nf == 0 || pressed || maxf != 1) return;
    Uint64 now = SDL_GetTicks();
    if (mode == 0 && now - t0 >= PRESS_DELAY) {
        emit_button(SDL_BUTTON_LEFT, 1);
        pressed = 1;
    } else if (mode == 1 && travel < MOVE_PX && now - t0 >= HOLD_MS) {
        emit_button(SDL_BUTTON_LEFT, 1);
        pressed = 1;
    }
}

static Finger *find(SDL_FingerID id)
{
    for (int i = 0; i < MAXF; i++)
        if (fing[i].used && fing[i].id == id) return &fing[i];
    return NULL;
}

static void gesture_end(void)
{
    Uint64 dur = SDL_GetTicks() - t0;
    if (maxf == 1) {
        if (pressed) emit_button(SDL_BUTTON_LEFT, 0);
        else if (mode == 0) click(SDL_BUTTON_LEFT);                          /* a tap faster than the press delay */
        else if (travel < MOVE_PX && dur < TAP_MS) click(SDL_BUTTON_LEFT);   /* trackpad tap */
    } else {
        if (pressed) emit_button(SDL_BUTTON_LEFT, 0);
        else if (travel < MOVE_PX && dur < TAP_MS) {
            if (maxf == 2) click(SDL_BUTTON_RIGHT);
            else if (maxf == 3) {
                mode = !mode;
                toast = mode ? "TOUCH: TRACKPAD" : "TOUCH: TAP";
            } else if (maxf == 4) keyboard = 1;
        }
    }
    pressed = 0;
    maxf = 0;
}

/* a finger event: 1 if it was a touch screen's (and is now mouse events) */
static int finger(const SDL_Event *ev)
{
    SDL_TouchID dev = ev->tfinger.touchID;
    if (SDL_GetTouchDeviceType(dev) != SDL_TOUCH_DEVICE_DIRECT && !SDL_getenv("SUP_TOUCH"))
        return 0;
    active = 1;
    SDL_Window *win = SDL_GetWindowFromID(ev->tfinger.windowID);
    if (!win) win = SDL_GetKeyboardFocus();
    if (win) {
        int w, h;
        SDL_GetWindowSize(win, &w, &h);
        if (w > 0 && h > 0) { ww = (float)w; wh = (float)h; }
        vwin = SDL_GetWindowID(win);
    }
    float x = ev->tfinger.x * ww, y = ev->tfinger.y * wh;
    Finger *f = find(ev->tfinger.fingerID);

    switch (ev->type) {
    case SDL_EVENT_FINGER_DOWN: {
        if (f) break;
        for (int i = 0; i < MAXF && !f; i++)
            if (!fing[i].used) f = &fing[i];
        if (!f) break;
        f->used = 1;
        f->id = ev->tfinger.fingerID;
        f->x = x;
        f->y = y;
        if (nf == 0) {                      /* a new gesture */
            t0 = SDL_GetTicks();
            travel = 0;
            pressed = 0;
            maxf = 0;
            if (mode == 0) place(x, y);
        }
        nf++;
        if (nf > maxf) maxf = nf;
        break;
    }
    case SDL_EVENT_FINGER_MOTION:
        if (!f) break;
        {
            float dx = x - f->x, dy = y - f->y;
            f->x = x;
            f->y = y;
            if (maxf == 1) {                /* the first finger alone moves the pointer */
                travel += SDL_fabsf(dx) + SDL_fabsf(dy);
                if (mode == 0) place(x, y);
                else place(vx + dx * GAIN, vy + dy * GAIN);
            } else {
                travel += SDL_fabsf(dx) + SDL_fabsf(dy);
            }
        }
        break;
    case SDL_EVENT_FINGER_UP:
    case SDL_EVENT_FINGER_CANCELED:
        if (!f) break;
        f->used = 0;
        nf--;
        if (nf <= 0) { nf = 0; gesture_end(); }
        break;
    default:
        return 0;
    }
    return 1;
}

static int pop(SDL_Event *ev)
{
    if (qn == 0) return 0;
    *ev = q[qh];
    qh = (qh + 1) % MAXQ;
    qn--;
    return 1;
}

int touch_poll_event(SDL_Event *ev)
{
    timers();
    if (pop(ev)) return 1;
    while (SDL_PollEvent(ev)) {
        if ((ev->type == SDL_EVENT_FINGER_DOWN || ev->type == SDL_EVENT_FINGER_UP ||
             ev->type == SDL_EVENT_FINGER_MOTION || ev->type == SDL_EVENT_FINGER_CANCELED)) {
            if (finger(ev)) {
                if (pop(ev)) return 1;
                continue;
            }
        }
        /* on a touch screen the real mouse (SDL's own touch-to-mouse events) is ignored */
        if (active && (ev->type == SDL_EVENT_MOUSE_MOTION || ev->type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                       ev->type == SDL_EVENT_MOUSE_BUTTON_UP) &&
            ev->button.which == SDL_TOUCH_MOUSEID)
            continue;
        return 1;
    }
    timers();
    return pop(ev);
}

SDL_MouseButtonFlags touch_mouse_state(float *x, float *y)
{
    SDL_MouseButtonFlags real = SDL_GetMouseState(x, y);
    if (!active) return real;
    if (x) *x = vx;
    if (y) *y = vy;
    return (vleft ? SDL_BUTTON_LMASK : 0) | (vright ? SDL_BUTTON_RMASK : 0);
}

int touch_take_keyboard(void)
{
    int k = keyboard;
    keyboard = 0;
    return k;
}

const char *touch_take_toast(void)
{
    const char *t = toast;
    toast = NULL;
    return t;
}
