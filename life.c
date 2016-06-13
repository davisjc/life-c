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

int init_sdl(SDL_Window **win, /* populates with window */
             SDL_Renderer **ren /* populates with renderer */);

void teardown_sdl(SDL_Window *win,
                  SDL_Renderer *ren,
                  const char *offending_func /* optional name of SDL func */);

int
main(int argc, char *argv[])
{
    /* Initialize SDL. */
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;
    if (init_sdl(&win, &ren))
        return 1;

    /* Load zoidberg! */
    SDL_Surface *zoidsurf = SDL_LoadBMP("res/zoidberg.bmp");
    if (zoidsurf == NULL) {
        teardown_sdl(win, ren, "SDL_LoadBMP");
        return 1;
    }

    SDL_Texture *zoidtex = SDL_CreateTextureFromSurface(ren, zoidsurf);
    SDL_FreeSurface(zoidsurf);
    zoidsurf = NULL;
    if (zoidtex == NULL) {
        teardown_sdl(win, ren, "SDL_CreateTextureFromSurface");
        return 1;
    }

    for (int angle = 0; angle < 360 * 3; angle += 20) {
        SDL_RenderClear(ren);
        SDL_RenderCopyEx(ren, zoidtex, NULL, NULL, angle, NULL, 0);
        SDL_RenderPresent(ren);

        SDL_Delay(0);
    }

    printf("Exiting...\n");
    teardown_sdl(win, ren, NULL);
    return 0;
}

int
init_sdl(SDL_Window **win, SDL_Renderer **ren)
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
teardown_sdl(SDL_Window *win, SDL_Renderer *ren, const char *offending_func)
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    if (offending_func)
        fprintf(stderr, "%s error: %s\n", offending_func, SDL_GetError());
    SDL_Quit();
}

