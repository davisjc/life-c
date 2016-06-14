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
#include <time.h>


#define WINDOW_TITLE "Conway's Game of Life (Q to quit)"
#define BOARD_WIDTH 300
#define BOARD_HEIGHT 150
#define SQUARE_SIZE 5
#define BOARD_ALIVE_COLOR {230, 170, 20}
#define BOARD_DEAD_COLOR {20, 20, 20}
#define WINDOW_WIDTH_DEFAULT (BOARD_WIDTH * SQUARE_SIZE + BOARD_WIDTH + 1)
#define WINDOW_HEIGHT_DEFAULT (BOARD_HEIGHT * SQUARE_SIZE + BOARD_HEIGHT + 1)
#define ALIVE 1
#define DEAD 0
#define LUCK_LIFE_START 15 /* out of 100 */
#define FRAME_DELAY_MS 30

/* Game board: 0 = dead; 1 = alive */
static uint8_t board_a[BOARD_HEIGHT][BOARD_WIDTH];
static uint8_t board_b[BOARD_HEIGHT][BOARD_WIDTH];

/* SDL's perspective of board. */
static SDL_Rect board_rects[BOARD_HEIGHT][BOARD_WIDTH];

static uint8_t color_alive[3] = BOARD_ALIVE_COLOR;
static uint8_t color_dead[3] = BOARD_DEAD_COLOR;

static int
sdl_init(SDL_Window **win, /* populates with window */
         SDL_Renderer **ren /* populates with renderer */);

static void
sdl_teardown(SDL_Window *win,
             SDL_Renderer *ren,
             const char *offending_func /* optional name of SDL func */);

static void
sdl_log_error(const char *offending_func);

int
get_neighbor_count(uint8_t (*board)[BOARD_WIDTH], int row, int col);

int
main(int argc, char *argv[])
{
    /* Initialize SDL. */
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;
    if (sdl_init(&win, &ren))
        return 1;

    /* Set the board. */
    srand(time(NULL));
    for (int row = 0; row < BOARD_HEIGHT; row++)
        for (int col = 0; col < BOARD_WIDTH; col++)
            board_a[row][col] = (rand() % 100 < LUCK_LIFE_START);
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            board_rects[row][col].x = 1 + SQUARE_SIZE * col + col;
            board_rects[row][col].y = 1 + SQUARE_SIZE * row + row;
            board_rects[row][col].w = SQUARE_SIZE;
            board_rects[row][col].h = SQUARE_SIZE;
        }
    }

    uint8_t (*board_cur)[BOARD_WIDTH] = board_a;
    uint8_t (*board_next)[BOARD_WIDTH] = board_b;
    int run = 1;
    int quit = 0;
    while (!quit) {
        int step = 0;
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_q) {
                        quit = 1;
                    } else if (e.key.keysym.sym == SDLK_SPACE) {
                        run = !run;
                    } else if (e.key.keysym.sym == SDLK_s) {
                        run = 0;
                        step = 1;
                    }
                    break;
                default:
                    break;
            }
        }

        if (!run && !step) {
            SDL_Delay(FRAME_DELAY_MS);
            continue;
        }

        /* Draw grid. */
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        for (int row = 0; row < BOARD_HEIGHT; row++) {
            for (int col = 0; col < BOARD_WIDTH; col++) {
                uint8_t *color;
                int alive = board_cur[row][col];
                if (alive)
                    color = color_alive;
                else
                    color = color_dead;

                /* Draw this cell. */
                SDL_SetRenderDrawColor(ren, color[0], color[1], color[2], 255);
                SDL_RenderFillRect(ren, &board_rects[row][col]);

                /* Update the life/death status of this cell. */
                int neighbor_count = get_neighbor_count(board_cur, row, col);
                if (alive) {
                    if (neighbor_count < 2)
                        board_next[row][col] = DEAD;
                    else if (neighbor_count > 3)
                        board_next[row][col] = DEAD;
                    else
                        board_next[row][col] = ALIVE;
                } else { /* dead */
                    if (neighbor_count == 3)
                        board_next[row][col] = ALIVE;
                    else
                        board_next[row][col] = DEAD;
                }
            }
        }

        uint8_t (*board_temp)[BOARD_WIDTH] = board_cur;
        board_cur = board_next;
        board_next = board_temp;

        SDL_RenderPresent(ren);
        SDL_Delay(FRAME_DELAY_MS);
    }

    printf("Exiting...\n");
    sdl_teardown(win, ren, NULL);
    return 0;
}

static int
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

static void
sdl_teardown(SDL_Window *win, SDL_Renderer *ren, const char *offending_func)
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    if (offending_func)
        sdl_log_error(offending_func);
    SDL_Quit();
}

static void
sdl_log_error(const char *offending_func)
{
    fprintf(stderr, "%s error: %s\n", offending_func, SDL_GetError());
}

int
get_neighbor_count(uint8_t (*board)[BOARD_WIDTH], int row, int col)
{
    int count = 0;
    if (0 < row)
        count += board[row - 1][col];
    if (row < BOARD_HEIGHT - 1)
        count += board[row + 1][col];
    if (0 < col)
        count += board[row][col - 1];
    if (col < BOARD_WIDTH - 1)
        count += board[row][col + 1];
    if (0 < row && 0 < col)
        count += board[row - 1][col - 1];
    if (0 < row && col < BOARD_WIDTH - 1)
        count += board[row - 1][col + 1];
    if (row < BOARD_HEIGHT - 1 && 0 < col)
        count += board[row + 1][col - 1];
    if (row < BOARD_HEIGHT - 1 && col < BOARD_WIDTH - 1)
        count += board[row + 1][col + 1];
    return count;
}

