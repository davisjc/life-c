
#include "render.h"

#include <stdint.h>
#include <stdio.h>

#include "SDL2/SDL.h"
#include "types.h"
#include "vars.h"


static Color color_grid[3] = COLOR_GRID;
static Color color_alive_a[3] = COLOR_ALIVE_A;
static Color color_alive_b[3] = COLOR_ALIVE_B;
static Color color_dead[3] = COLOR_DEAD;

/* Track FPS samples. */
static int32_t fps_samples[FPS_SAMPLE_COUNT];
static int32_t fps_samples_i = 0;
static int32_t ticks_last_render = 0;

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
get_color_for_cell(int32_t row, int32_t col, int32_t board_h, int32_t board_w,
                   Color *color)
{
    double proportion_b = (0.0 + col) / board_w;
    double proportion_a = 1.0 - proportion_b;
    for (int i = 0; i < 3; i++) {
        color[i] = (Color)(proportion_a * color_alive_a[i] +
                           proportion_b * color_alive_b[i]);
    }
}

void
render_blank(SDL_Renderer *ren)
{
    SDL_SetRenderDrawColor(ren,
                           color_grid[0], color_grid[1], color_grid[2],
                           255);
    SDL_RenderClear(ren);
}

void
render_cells(SDL_Renderer *ren, int32_t board_h, int32_t board_w,
             BoardRect rects, Board board)
{
    for (int32_t row = 0; row < board_h; row++) {
        for (int32_t col = 0; col < board_w; col++) {
            Color *color = NULL;
            Color color_alive[3];

            int is_alive = board[row][col];
            if (is_alive) {
                get_color_for_cell(row, col, board_h, board_w, color_alive);
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

    record_fps();
}

void
record_fps(void)
{
    int32_t ticks_cur_render = SDL_GetTicks();
    int32_t ticks_since_last_render = ticks_cur_render - ticks_last_render;
    fps_samples[fps_samples_i] = (int)(1000.0 / ticks_since_last_render);
    fps_samples_i = (fps_samples_i + 1) % FPS_SAMPLE_COUNT;
    if (fps_samples_i == 0) {
        int32_t fps = 0;
        for (int i = 0; i < FPS_SAMPLE_COUNT; i++)
            fps += fps_samples[i];
        fps = (int64_t)(fps / FPS_SAMPLE_COUNT);
        printf("FPS: %d\n", fps);
    }
    ticks_last_render = ticks_cur_render;
}

