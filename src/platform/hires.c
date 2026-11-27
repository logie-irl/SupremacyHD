/* Hi-res picture packs, see hires.h. */
#include "hires.h"
#include <stdio.h>
#include <string.h>

static const struct { uint32_t addr; const char *name; } pics[] = {
    {PIC_MAIN, "main"}, {PIC_STATISTICS, "statistics"}, {PIC_SHIPYARD, "shipyard"},
    {PIC_FLEET, "fleet"}, {PIC_TROOPS, "troops"}, {PIC_CARGO, "cargo"},
    {PIC_SURFACE, "surface"}, {PIC_COMBAT, "combat"}, {PIC_TITLE, "title"},
};
#define NPICS ((int)(sizeof pics / sizeof pics[0]))

static SDL_Texture *tex[NPICS];
static SDL_Renderer *b_ren;
static const GameData *b_gd;
static int b_tried[NPICS];

/* Scale2x on 0xAARRGGBB pixels: w x h -> 2w x 2h. */
static void scale2x(const Uint32 *in, int w, int h, Uint32 *out)
{
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            Uint32 E = in[y * w + x];
            Uint32 B = in[(y > 0 ? y - 1 : y) * w + x], H = in[(y < h - 1 ? y + 1 : y) * w + x];
            Uint32 D = in[y * w + (x > 0 ? x - 1 : x)], F = in[y * w + (x < w - 1 ? x + 1 : x)];
            Uint32 e0 = E, e1 = E, e2 = E, e3 = E;
            if (B != H && D != F) {
                if (D == B) e0 = D;
                if (B == F) e1 = F;
                if (D == H) e2 = D;
                if (H == F) e3 = F;
            }
            Uint32 *o = out + (size_t)(2 * y) * (2 * w) + 2 * x;
            o[0] = e0; o[1] = e1; o[2 * w] = e2; o[2 * w + 1] = e3;
        }
}

/* Separable Gaussian blur of a float RGB image (3 floats per pixel), in place. */
static void blur(float *im, int w, int h, float sigma, float *tmp)
{
    int r = (int)(sigma * 3.0f + 0.5f);
    if (r < 1) return;
    float k[64];
    if (r > 31) r = 31;
    float sum = 0;
    for (int i = -r; i <= r; i++) { k[i + r] = SDL_expf(-(float)(i * i) / (2 * sigma * sigma)); sum += k[i + r]; }
    for (int i = 0; i <= 2 * r; i++) k[i] /= sum;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            for (int c = 0; c < 3; c++) {
                float a = 0;
                for (int i = -r; i <= r; i++) {
                    int xx = SDL_clamp(x + i, 0, w - 1);
                    a += k[i + r] * im[((size_t)y * w + xx) * 3 + c];
                }
                tmp[((size_t)y * w + x) * 3 + c] = a;
            }
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            for (int c = 0; c < 3; c++) {
                float a = 0;
                for (int i = -r; i <= r; i++) {
                    int yy = SDL_clamp(y + i, 0, h - 1);
                    a += k[i + r] * tmp[((size_t)yy * w + x) * 3 + c];
                }
                im[((size_t)y * w + x) * 3 + c] = a;
            }
}

static SDL_Texture *make_builtin(int i)
{
    static Picture pic;
    if (picture_decode(b_gd, pics[i].addr, &pic) != 0) return NULL;
    const int W = 2560, H = 1600, OW = 1280, OH = 800;
    Uint32 *a = SDL_malloc((size_t)W * H * 4), *b = SDL_malloc((size_t)W * H * 4);
    float *f = SDL_malloc((size_t)OW * OH * 12), *t = SDL_malloc((size_t)OW * OH * 12), *g = SDL_malloc((size_t)OW * OH * 12);
    SDL_Surface *sf = NULL;
    SDL_Texture *tx = NULL;
    if (!a || !b || !f || !t || !g) goto done;
    for (int k = 0; k < 320 * 200; k++) a[k] = 0xFF000000u | pic.palette[pic.pixels[k] & 31];
    scale2x(a, 320, 200, b);                    /* 640 x 400 */
    scale2x(b, 640, 400, a);                    /* 1280 x 800 */
    scale2x(a, 1280, 800, b);                   /* 2560 x 1600 */
    /* down to 1280x800 (2x2 average), then soften the stair steps */
    for (int y = 0; y < OH; y++)
        for (int x = 0; x < OW; x++)
            for (int c = 0; c < 3; c++) {
                int sh = 16 - 8 * c;
                float v = 0;
                for (int dy = 0; dy < 2; dy++)
                    for (int dx = 0; dx < 2; dx++)
                        v += (float)((b[(size_t)(2 * y + dy) * W + 2 * x + dx] >> sh) & 255);
                f[((size_t)y * OW + x) * 3 + c] = v / 4.0f;
            }
    blur(f, OW, OH, 1.1f, t);
    /* sharpen (unsharp mask) and a little more colour */
    SDL_memcpy(g, f, (size_t)OW * OH * 12);
    blur(g, OW, OH, 1.0f, t);
    sf = SDL_CreateSurface(OW, OH, SDL_PIXELFORMAT_ARGB8888);
    if (!sf) goto done;
    for (int y = 0; y < OH; y++) {
        Uint32 *row = (Uint32 *)((Uint8 *)sf->pixels + y * sf->pitch);
        for (int x = 0; x < OW; x++) {
            float v[3];
            for (int c = 0; c < 3; c++) {
                size_t k = ((size_t)y * OW + x) * 3 + c;
                v[c] = 1.6f * f[k] - 0.6f * g[k];
            }
            float m = (v[0] + v[1] + v[2]) / 3.0f;
            Uint32 p = 0xFF000000u;
            for (int c = 0; c < 3; c++) {
                float u = m + (v[c] - m) * 1.12f;
                p |= (Uint32)SDL_clamp((int)(u + 0.5f), 0, 255) << (16 - 8 * c);
            }
            row[x] = p;
        }
    }
    tx = SDL_CreateTextureFromSurface(b_ren, sf);
    if (tx) SDL_SetTextureScaleMode(tx, SDL_SCALEMODE_LINEAR);
done:
    if (sf) SDL_DestroySurface(sf);
    SDL_free(a); SDL_free(b); SDL_free(f); SDL_free(t); SDL_free(g);
    return tx;
}

void hires_builtin(SDL_Renderer *ren, const GameData *gd)
{
    b_ren = ren;
    b_gd = gd;
    SDL_memset(b_tried, 0, sizeof b_tried);
}

static SDL_Surface *load_image(const char *dir, const char *name)
{
    static const char *const ext[] = {"png", "jpg", "jpeg", "bmp"};
    char path[1400];
    for (size_t e = 0; e < sizeof ext / sizeof ext[0]; e++) {
        SDL_snprintf(path, sizeof path, "%s/%s.%s", dir, name, ext[e]);
        SDL_PathInfo pi;
        if (!SDL_GetPathInfo(path, &pi) || pi.type != SDL_PATHTYPE_FILE) continue;
        SDL_Surface *sf = NULL;
#if SDL_VERSION_ATLEAST(3, 4, 0)
        sf = SDL_LoadSurface(path);          /* PNG, JPEG, BMP ... */
#else
        if (!strcmp(ext[e], "bmp")) sf = SDL_LoadBMP(path);
#endif
        if (sf) return sf;
        SDL_Log("hires: %s: %s", path, SDL_GetError());
    }
    return NULL;
}

int hires_load(SDL_Renderer *ren, const char *dir_a, const char *dir_b)
{
    hires_free();
    const char *dirs[2] = {dir_a, dir_b};
    for (int d = 0; d < 2; d++) {
        if (!dirs[d] || !dirs[d][0]) continue;
        int n = 0;
        for (int i = 0; i < NPICS; i++) {
            if (tex[i]) continue;
            SDL_Surface *sf = load_image(dirs[d], pics[i].name);
            if (!sf) continue;
            SDL_Surface *c = SDL_ConvertSurface(sf, SDL_PIXELFORMAT_ARGB8888);
            SDL_DestroySurface(sf);
            if (!c) continue;
            tex[i] = SDL_CreateTextureFromSurface(ren, c);
            SDL_DestroySurface(c);
            if (!tex[i]) continue;
            SDL_SetTextureScaleMode(tex[i], SDL_SCALEMODE_LINEAR);
            n++;
        }
        if (n) {
            SDL_Log("hires: %d pictures from %s", n, dirs[d]);
            break;
        }
    }
    int total = 0;
    for (int i = 0; i < NPICS; i++) total += tex[i] != NULL;
    return total;
}

void hires_free(void)
{
    for (int i = 0; i < NPICS; i++)
        if (tex[i]) { SDL_DestroyTexture(tex[i]); tex[i] = NULL; }
}

SDL_Texture *hires_find(uint32_t addr)
{
    for (int i = 0; i < NPICS; i++)
        if (pics[i].addr == addr) {
            if (!tex[i] && b_gd && !b_tried[i]) {       /* no pack file: make it once */
                b_tried[i] = 1;
                tex[i] = make_builtin(i);
            }
            return tex[i];
        }
    return NULL;
}

int hires_export(const GameData *gd, const char *dir)
{
    static Picture pic;
    int n = 0;
    SDL_CreateDirectory(dir);
    for (int i = 0; i < NPICS; i++) {
        if (picture_decode(gd, pics[i].addr, &pic) != 0) continue;
        SDL_Surface *sf = SDL_CreateSurface(320, 200, SDL_PIXELFORMAT_ARGB8888);
        if (!sf) return -1;
        for (int y = 0; y < 200; y++) {
            Uint32 *row = (Uint32 *)((Uint8 *)sf->pixels + y * sf->pitch);
            for (int x = 0; x < 320; x++) row[x] = 0xFF000000u | pic.palette[pic.pixels[y * 320 + x] & 31];
        }
        char path[1400];
#if SDL_VERSION_ATLEAST(3, 4, 0)
        SDL_snprintf(path, sizeof path, "%s/%s.png", dir, pics[i].name);
        int ok = SDL_SavePNG(sf, path);
#else
        SDL_snprintf(path, sizeof path, "%s/%s.bmp", dir, pics[i].name);
        int ok = SDL_SaveBMP(sf, path);
#endif
        SDL_DestroySurface(sf);
        if (ok) n++;
        else fprintf(stderr, "export: %s: %s\n", path, SDL_GetError());
    }
    return n;
}
