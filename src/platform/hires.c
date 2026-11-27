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
        if (pics[i].addr == addr) return tex[i];
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
