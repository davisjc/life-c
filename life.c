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

/* A cell is just a byte: 0 = dead; 1 = alive */
typedef uint8_t Cell;

typedef uint8_t Color;

/* Use two boards to maintain a backbuffer. */
static Cell board_a[BOARD_HEIGHT][BOARD_WIDTH];
static Cell board_b[BOARD_HEIGHT][BOARD_WIDTH];

/* Track clicks */
static Cell board_clicks[BOARD_HEIGHT][BOARD_WIDTH];

/* SDL's perspective of board. */
static SDL_Rect board_rects[BOARD_HEIGHT][BOARD_WIDTH];

/* Define some colors. */
static Color color_grid[3] = GRID_COLOR;
static Color color_alive[3] = BOARD_ALIVE_COLOR;
static Color color_dead[3] = BOARD_DEAD_COLOR;

static int
sdl_init(SDL_Window **win, /* populates with window */
         SDL_Renderer **ren /* populates with renderer */);

static void
sdl_teardown(SDL_Window *win,
             SDL_Renderer *ren,
             const char *offending_func_name /* optional name of SDL func */);

static void
sdl_log_error(const char *offending_func_name);

static void
populate_board(Cell (*board)[BOARD_WIDTH]);

static void
zero_board(Cell (*board)[BOARD_WIDTH]);

static void
init_board_rects(SDL_Rect (*board_rects)[BOARD_WIDTH]);

static size_t
get_neighbor_count(Cell (*board)[BOARD_WIDTH], int32_t row, int32_t col);

static Cell *
get_cell_by_coord(Cell (*board)[BOARD_WIDTH], int32_t x, int32_t y);

static void
advance_cell(int32_t row,
             int32_t col,
             Cell (*board_active)[BOARD_WIDTH],
             Cell (*board_backbuffer)[BOARD_WIDTH]);

#define is_alive(cell) (!!cell)

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

    Cell (*board_active)[BOARD_WIDTH] = board_a;
    Cell (*board_backbuffer)[BOARD_WIDTH] = board_b;
    int run = 1;
    int quit = 0;
    while (!quit) {
        int step = 0;
        int restart = 0;
        int clear = 0;
        int clicked = 0;
        int dirty = 0;
        zero_board(board_clicks);

        /*
         * Events.
         */
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
                    } else if (e.key.keysym.sym == SDLK_LEFTBRACKET) {
                        frame_delay += FRAME_DELAY_CHANGE_STEP_MS;
                        if (frame_delay > INT_MAX - FRAME_DELAY_CHANGE_STEP_MS)
                            frame_delay = INT_MAX - FRAME_DELAY_CHANGE_STEP_MS;
                    } else if (e.key.keysym.sym == SDLK_RIGHTBRACKET) {
                        frame_delay -= FRAME_DELAY_CHANGE_STEP_MS;
                        if (frame_delay < 0)
                            frame_delay = 0;
                    } else if (e.key.keysym.sym == SDLK_r) {
                        restart = 1;
                        run = 1;
                        frame_delay = FRAME_DELAY_INITIAL_MS;
                    } else if (e.key.keysym.sym == SDLK_c) {
                        clear = 1;
                        run = 0;
                        step = 1;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                {
                    int32_t x = e.button.x;
                    int32_t y = e.button.y;
                    Cell *cell = get_cell_by_coord(board_clicks, x, y);

                    /* Flip this cell's clicked/unclicked state. */
                    *cell = !*cell;
                    clicked = 1;
                }
                    break;
                default:
                    break;
            }
        }

        /*
         * Logic.
         */
        if (restart)
            populate_board(board_active);
        else if (clear)
            zero_board(board_active);

        if (run || step) {
            /* Write the updated life/death statuses to the backbuffer. */
            for (int32_t row = 0; row < BOARD_HEIGHT; row++) {
                for (int32_t col = 0; col < BOARD_WIDTH; col++) {
                    advance_cell(row, col, board_active, board_backbuffer);
                }
            }

            /* Make the backbuffer active. */
            Cell (*board_temp)[BOARD_WIDTH] = board_active;
            board_active = board_backbuffer;
            board_backbuffer = board_temp;
        }

        if (clicked) {
            for (int32_t row = 0; row < BOARD_HEIGHT; row++) {
                for (int32_t col = 0; col < BOARD_WIDTH; col++) {
                    if (board_clicks[row][col]) {
                        /* Kill if alive; revive if dead. */
                        board_active[row][col] = !board_active[row][col];
                        dirty = 1;
                    }
                }
            }
        }

        if (restart || run || step)
            dirty = 1;

        /*
         * Render.
         */
        if (dirty) {
            /* Grid. */
            SDL_SetRenderDrawColor(ren,
                                   color_grid[0], color_grid[1], color_grid[2],
                                   255);
            SDL_RenderClear(ren);

            /* Cells. */
            for (int32_t row = 0; row < BOARD_HEIGHT; row++) {
                for (int32_t col = 0; col < BOARD_WIDTH; col++) {
                    Color *color;
                    if (is_alive(board_active[row][col]))
                        color = color_alive;
                    else
                        color = color_dead;

                    /* Draw this cell. */
                    SDL_SetRenderDrawColor(ren, color[0], color[1], color[2],
                                           255);
                    SDL_RenderFillRect(ren, &board_rects[row][col]);
                }
            }

            SDL_RenderPresent(ren);
        }

        /* TODO - use a timer; don't sleep. */
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
                    const char *offending_func_name)
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    if (offending_func_name)
        sdl_log_error(offending_func_name);
    SDL_Quit();
}

static void
sdl_log_error(const char *offending_func_name)
{
    fprintf(stderr, "%s error: %s\n", offending_func_name, SDL_GetError());
}

static void
populate_board(Cell (*board)[BOARD_WIDTH])
{
    for (int32_t row = 0; row < BOARD_HEIGHT; row++)
        for (int32_t col = 0; col < BOARD_WIDTH; col++)
            board[row][col] = (rand() % 100 < LUCK_LIFE_START);
}

static void
zero_board(Cell (*board)[BOARD_WIDTH])
{
    for (int32_t row = 0; row < BOARD_HEIGHT; row++)
        for (int32_t col = 0; col < BOARD_WIDTH; col++)
            board[row][col] = 0;
}

static void
init_board_rects(SDL_Rect (*board_rects)[BOARD_WIDTH])
{
    for (int32_t row = 0; row < BOARD_HEIGHT; row++) {
        for (int32_t col = 0; col < BOARD_WIDTH; col++) {
            board_rects[row][col].x = GRID_SIZE + (CELL_SIZE + GRID_SIZE) * col;
            board_rects[row][col].y = GRID_SIZE + (CELL_SIZE + GRID_SIZE) * row;
            board_rects[row][col].w = CELL_SIZE;
            board_rects[row][col].h = CELL_SIZE;
        }
    }
}

static size_t
get_neighbor_count(Cell (*board)[BOARD_WIDTH], int32_t row, int32_t col)
{
    uint32_t count = 0;
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

static Cell *
get_cell_by_coord(Cell (*board)[BOARD_WIDTH], int32_t x, int32_t y)
{
    uint32_t col = x / (GRID_SIZE + CELL_SIZE);
    uint32_t row = y / (GRID_SIZE + CELL_SIZE);
    if (row >= BOARD_HEIGHT)
        row = BOARD_HEIGHT - 1;
    if (col >= BOARD_WIDTH)
        col = BOARD_WIDTH - 1;
    return board[row] + col;
}

static void
advance_cell(int32_t row,
             int32_t col,
             Cell (*board_active)[BOARD_WIDTH],
             Cell (*board_backbuffer)[BOARD_WIDTH])
{
    size_t neighbor_count = get_neighbor_count(board_active, row, col);
    if (is_alive(board_active[row][col])) {
        if (neighbor_count < 2)
            board_backbuffer[row][col] = DEAD;
        else if (neighbor_count > 3)
            board_backbuffer[row][col] = DEAD;
        else
            board_backbuffer[row][col] = ALIVE;
    } else { /* dead */
        if (neighbor_count == 3)
            board_backbuffer[row][col] = ALIVE;
        else
            board_backbuffer[row][col] = DEAD;
    }
}

