
#include "SDL2/SDL.h"
#include "render.h"
#include <stdio.h>

int
sdl_init(char *title, int width, int height,
         SDL_Window **win, SDL_Renderer **ren)
{
    if (SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    *win = SDL_CreateWindow(title,
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            width,
                            height,
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
sdl_teardown(SDL_Window *win,
                    SDL_Renderer *ren,
                    const char *offending_func_name)
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    if (offending_func_name)
        sdl_log_error(offending_func_name);
    SDL_Quit();
}

void
sdl_log_error(const char *offending_func_name)
{
    fprintf(stderr, "%s error: %s\n", offending_func_name, SDL_GetError());
}

void
get_color_for_cell(int32_t row, int32_t col, Color *color)
{
    extern Color color_alive_a[3];
    extern Color color_alive_b[3];

    double proportion_b = (0.0 + col) / BOARD_W_INIT;
    double proportion_a = 1.0 - proportion_b;
    for (int i = 0; i < 3; i++) {
        color[i] = (Color)(proportion_a * color_alive_a[i] +
                           proportion_b * color_alive_b[i]);
    }
}

void
render_cells(SDL_Renderer *ren, BoardRect rects, Board board)
{
    extern Color color_dead[3];
    for (int32_t row = 0; row < BOARD_H_INIT; row++) {
        for (int32_t col = 0; col < BOARD_W_INIT; col++) {
            Color *color = NULL;
            Color color_alive[3];

            int is_alive = board[row][col];
            if (is_alive) {
                get_color_for_cell(row, col, color_alive);
                color = color_alive;
            } else {
                color = color_dead;
            }

            /* Draw this cell. */
            SDL_SetRenderDrawColor(ren, color[0], color[1], color[2],
                                   255);
            SDL_RenderFillRect(ren, &rects[row][col]);
        }
    }
}

