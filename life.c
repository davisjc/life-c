/*
 * Conway's Game of Life.
 *
 * An excercise in SDL and C.
 *
 * @author: Johnathan Davis
 */

#include "SDL2/SDL.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define WINDOW_TITLE "Conway's Game of Life (Q to quit)"
#define BOARD_WIDTH 150
#define BOARD_HEIGHT 80
#define CELL_SIZE 7
#define GRID_SIZE 1
#define GRID_COLOR {0, 0, 0}
#define BOARD_ALIVE_COLOR {230, 170, 20}
#define BOARD_DEAD_COLOR {20, 20, 20}
#define WINDOW_WIDTH_DEFAULT (GRID_SIZE + (CELL_SIZE + GRID_SIZE) * \
                              BOARD_WIDTH)
#define WINDOW_HEIGHT_DEFAULT (GRID_SIZE + (CELL_SIZE + GRID_SIZE) * BOARD_HEIGHT)
#define ALIVE 1
#define DEAD 0
#define LUCK_LIFE_START 15 /* out of 100 */
#define FRAME_DELAY_INITIAL_MS 30
#define FRAME_DELAY_CHANGE_STEP_MS 10

/* Game board: 0 = dead; 1 = alive */
static uint8_t board_a[BOARD_HEIGHT][BOARD_WIDTH];
static uint8_t board_b[BOARD_HEIGHT][BOARD_WIDTH];

/* SDL's perspective of board. */
static SDL_Rect board_rects[BOARD_HEIGHT][BOARD_WIDTH];

static uint8_t color_grid[3] = GRID_COLOR;
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

static void
populate_board(uint8_t (*board)[BOARD_WIDTH]);

static void
init_board_rects(SDL_Rect (*board_rects)[BOARD_WIDTH]);

static int
get_neighbor_count(uint8_t (*board)[BOARD_WIDTH], int row, int col);

static uint8_t *
get_cell_by_coord(uint8_t (*board)[BOARD_WIDTH], int32_t x, int32_t y);

int
main(int argc, char *argv[])
{
    /* Initialize SDL. */
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;
    if (sdl_init(&win, &ren))
        return 1;

    srand(time(NULL));

    /* Set the board. */
    populate_board(board_a);

    /* Specify SDL_Rect dimensions once for the board. */
    init_board_rects(board_rects);

    int32_t frame_delay = FRAME_DELAY_INITIAL_MS;

    uint8_t (*board_cur)[BOARD_WIDTH] = board_a;
    uint8_t (*board_next)[BOARD_WIDTH] = board_b;
    int run = 1;
    int quit = 0;
    while (!quit) {
        int step = 0;
        SDL_Event e;

        /* Process events. */
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
                    } else if (e.key.keysym.sym == SDLK_LEFTBRACKET) {
                        frame_delay += FRAME_DELAY_CHANGE_STEP_MS;
                        if (frame_delay > INT_MAX - FRAME_DELAY_CHANGE_STEP_MS)
                            frame_delay = INT_MAX - FRAME_DELAY_CHANGE_STEP_MS;
                    } else if (e.key.keysym.sym == SDLK_RIGHTBRACKET) {
                        frame_delay -= FRAME_DELAY_CHANGE_STEP_MS;
                        if (frame_delay < 0)
                            frame_delay = 0;
                    } else if (e.key.keysym.sym == SDLK_r) {
                        populate_board(board_cur);
                        frame_delay = FRAME_DELAY_INITIAL_MS;
                        if (!run)
                            run = 1;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                {
                    int32_t x = e.button.x;
                    int32_t y = e.button.y;
                    uint8_t *cell = get_cell_by_coord(board_cur, x, y);
                    *cell = !*cell;
                }
                    break;
                default:
                    break;
            }
        }

        /* Draw grid. */
        SDL_SetRenderDrawColor(ren, color_grid[0], color_grid[1], color_grid[2],
                               255);
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
                if (run || step) {
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
        }

        if (run || step) {
            uint8_t (*board_temp)[BOARD_WIDTH] = board_cur;
            board_cur = board_next;
            board_next = board_temp;
        }

        SDL_RenderPresent(ren);
        SDL_Delay(frame_delay);
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
sdl_teardown(SDL_Window *win,
                    SDL_Renderer *ren,
                    const char *offending_func)
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

static void
populate_board(uint8_t (*board)[BOARD_WIDTH])
{
    for (int row = 0; row < BOARD_HEIGHT; row++)
        for (int col = 0; col < BOARD_WIDTH; col++)
            board[row][col] = (rand() % 100 < LUCK_LIFE_START);
}

static void
init_board_rects(SDL_Rect (*board_rects)[BOARD_WIDTH])
{
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            board_rects[row][col].x = GRID_SIZE + (CELL_SIZE + GRID_SIZE) * col;
            board_rects[row][col].y = GRID_SIZE + (CELL_SIZE + GRID_SIZE) * row;
            board_rects[row][col].w = CELL_SIZE;
            board_rects[row][col].h = CELL_SIZE;
        }
    }
}

static int
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

static uint8_t *
get_cell_by_coord(uint8_t (*board)[BOARD_WIDTH], int32_t x, int32_t y)
{
    uint32_t col = x / (GRID_SIZE + CELL_SIZE);
    uint32_t row = y / (GRID_SIZE + CELL_SIZE);
    if (row >= BOARD_HEIGHT)
        row = BOARD_HEIGHT - 1;
    if (col >= BOARD_WIDTH)
        col = BOARD_WIDTH - 1;
    return board[row] + col;
}

