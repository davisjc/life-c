/*
 * Conway's Game of Life.
 *
 * An excercise in SDL and C.
 *
 * @author: Johnathan Davis
 */

#include "SDL2/SDL.h"
#include <stdio.h>
#include <stdlib.h>


#define WINDOW_TITLE "Conway's Game of Life"
#define WINDOW_WIDTH_DEFAULT 640
#define WINDOW_HEIGHT_DEFAULT 480

int
sdl_init(SDL_Window **win, /* populates with window */
         SDL_Renderer **ren /* populates with renderer */);

void
sdl_teardown(SDL_Window *win,
             SDL_Renderer *ren,
             const char *offending_func /* optional name of SDL func */);

void
sdl_log_error(const char *offending_func);

SDL_Texture *
texture_load(SDL_Renderer *ren, const char *filename);

int
main(int argc, char *argv[])
{
    /* Initialize SDL. */
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;
    if (sdl_init(&win, &ren))
        return 1;

    /* Load zoidberg! */
    SDL_Texture *zoidtex = texture_load(ren, "res/zoidberg.bmp");
    if (zoidtex == NULL) {
        sdl_teardown(win, ren, NULL);
        return 1;
    }

    for (int angle = 0; angle < 360 * 3; angle += 20) {
        SDL_RenderClear(ren);
        SDL_RenderCopyEx(ren, zoidtex, NULL, NULL, angle, NULL, 0);
        SDL_RenderPresent(ren);

        SDL_Delay(0);
    }

    printf("Exiting...\n");
    sdl_teardown(win, ren, NULL);
    return 0;
}

int
sdl_init(SDL_Window **win, SDL_Renderer **ren)
{
    if (SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    *win = SDL_CreateWindow(WINDOW_TITLE,
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            WINDOW_WIDTH_DEFAULT, /* width */
                            WINDOW_HEIGHT_DEFAULT, /* height */
                            SDL_WINDOW_OPENGL);
    if (win == NULL) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    *ren = SDL_CreateRenderer(*win,
                              -1, /* use any driver */
                              (SDL_RENDERER_ACCELERATED |
                               SDL_RENDERER_PRESENTVSYNC));
    if (ren == NULL) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    return 0;
}

void
sdl_teardown(SDL_Window *win, SDL_Renderer *ren, const char *offending_func)
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    if (offending_func)
        sdl_log_error(offending_func);
    SDL_Quit();
}

void
sdl_log_error(const char *offending_func)
{
    fprintf(stderr, "%s error: %s\n", offending_func, SDL_GetError());
}

SDL_Texture *
texture_load(SDL_Renderer *ren, const char *filename)
{
    SDL_Texture *tex = NULL;
    SDL_Surface *surf = SDL_LoadBMP(filename);
    if (surf == NULL) {
        sdl_log_error("SDL_LoadBMP");
        return NULL;
    }

    tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);
    surf = NULL;

    if (tex == NULL) {
        sdl_log_error("SDL_CreateTextureFromSurface");
        return NULL;
    }

    return tex;
}

