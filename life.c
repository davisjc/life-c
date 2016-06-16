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
#define COLOR_GRID {0, 0, 0}
#define COLOR_ALIVE_A {255, 20, 0}
#define COLOR_ALIVE_B {255, 150, 0}
#define COLOR_DEAD {20, 20, 20}
#define WINDOW_WIDTH_DEFAULT (GRID_SIZE + (CELL_SIZE + GRID_SIZE) * \
                              BOARD_WIDTH)
#define WINDOW_HEIGHT_DEFAULT (GRID_SIZE + (CELL_SIZE + GRID_SIZE) * \
                               BOARD_HEIGHT)
#define ALIVE 1
#define DEAD 0
#define LUCK_LIFE_START 15 /* out of 100 */
#define FPS_MAX 60
#define TICKS_PER_FRAME 1000.0 / FPS_MAX
#define TICK_RATE_INITIAL_MS 30
#define TICK_RATE_STEP 10

typedef uint8_t Cell; /* 0 = dead; 1 = alive */
typedef Cell (*Board)[BOARD_WIDTH];
typedef SDL_Rect (*BoardRect)[BOARD_WIDTH];
typedef uint8_t Color;

/* Use two boards to maintain a backbuffer. */
static Board board_active;
static Board board_backbuffer;

/* Track clicks */
static Board board_clicks;

/* SDL's perspective of board. */
static BoardRect board_rects;

/* Define some colors. */
static Color color_grid[3] = COLOR_GRID;
static Color color_alive_a[3] = COLOR_ALIVE_A;
static Color color_alive_b[3] = COLOR_ALIVE_B;
static Color color_dead[3] = COLOR_DEAD;

static int32_t tick_interval = TICK_RATE_INITIAL_MS;

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
populate_board(Board board);

static void
zero_board(Board board);

static void
init_board_rects(BoardRect board_rects);

static size_t
get_neighbor_count(Board board, int32_t row, int32_t col);

static Cell *
get_cell_by_coord(Board board, int32_t x, int32_t y);

static void
advance_cell(int32_t row,
             int32_t col,
             Board board_active,
             Board board_backbuffer);

static void
get_color_for_cell(int32_t row, int32_t col, Color *color);

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

    /* Initialize the board. */
    board_active = malloc(BOARD_WIDTH * BOARD_HEIGHT *
                          sizeof(**board_active));
    board_backbuffer = malloc(BOARD_WIDTH * BOARD_HEIGHT *
                              sizeof(**board_backbuffer));
    board_clicks = malloc(BOARD_WIDTH * BOARD_HEIGHT * sizeof(**board_clicks));
    board_rects = malloc(BOARD_WIDTH * BOARD_HEIGHT * sizeof(**board_rects));
    populate_board(board_active);
    init_board_rects(board_rects);

    /* Track the last time the game state advanced. */
    int32_t ticks_last_step = 0;

    int run = 1; /* keep at 1 to keep running the game at tick_interval */
    int quit = 0; /* set to 1 to exit the game loop */
    while (!quit) {
        int32_t ticks_start = SDL_GetTicks();

        int step = 0; /* set to 1 if the game should advance 1 tick */
        int restart = 0; /* set to 1 if the board should be reset */
        int clear = 0; /* set to 1 if the board should be cleared */
        int clicked = 0; /* set to 1 if a click happens */
        int dirty = 0; /* set to 1 if a render should happen */
        zero_board(board_clicks);

        /*
         * Handle events.
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
                        tick_interval += TICK_RATE_STEP;
                        if (tick_interval > INT_MAX - TICK_RATE_STEP)
                            tick_interval = INT_MAX - TICK_RATE_STEP;
                    } else if (e.key.keysym.sym == SDLK_RIGHTBRACKET) {
                        tick_interval -= TICK_RATE_STEP;
                        if (tick_interval < 0)
                            tick_interval = 0;
                    } else if (e.key.keysym.sym == SDLK_r) {
                        restart = 1;
                        run = 1;
                        tick_interval = TICK_RATE_INITIAL_MS;
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
         * Game logic.
         */
        int user_provoked_tick = (step || restart || clear || clicked);
        int next_tick_due = (SDL_GetTicks() - ticks_last_step >= tick_interval);
        if (user_provoked_tick || next_tick_due) {
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
                Board board_temp = board_active;
                board_active = board_backbuffer;
                board_backbuffer = board_temp;
            }

            dirty = 1;
            ticks_last_step = SDL_GetTicks();
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

        /*
         * Render.
         */
        if (dirty) {
            /* Blank screen; this will become the grid. */
            SDL_SetRenderDrawColor(ren,
                                   color_grid[0], color_grid[1], color_grid[2],
                                   255);
            SDL_RenderClear(ren);

            /* Cells. */
            for (int32_t row = 0; row < BOARD_HEIGHT; row++) {
                for (int32_t col = 0; col < BOARD_WIDTH; col++) {
                    Color *color = NULL;
                    Color color_alive[3];
                    if (is_alive(board_active[row][col])) {
                        get_color_for_cell(row, col, color_alive);
                        color = color_alive;
                    } else {
                        color = color_dead;
                    }

                    /* Draw this cell. */
                    SDL_SetRenderDrawColor(ren, color[0], color[1], color[2],
                                           255);
                    SDL_RenderFillRect(ren, &board_rects[row][col]);
                }
            }

            SDL_RenderPresent(ren);
        }

        /* Cap the frame rate. */
        int32_t ticks_ellapsed = SDL_GetTicks() - ticks_start;
        if (ticks_ellapsed < TICKS_PER_FRAME)
            SDL_Delay(TICKS_PER_FRAME - ticks_ellapsed);
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
populate_board(Board board)
{
    for (int32_t row = 0; row < BOARD_HEIGHT; row++)
        for (int32_t col = 0; col < BOARD_WIDTH; col++)
            board[row][col] = (rand() % 100 < LUCK_LIFE_START);
}

static void
zero_board(Board board)
{
    for (int32_t row = 0; row < BOARD_HEIGHT; row++)
        for (int32_t col = 0; col < BOARD_WIDTH; col++)
            board[row][col] = 0;
}

static void
init_board_rects(BoardRect board_rects)
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
get_neighbor_count(Board board, int32_t row, int32_t col)
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
get_cell_by_coord(Board board, int32_t x, int32_t y)
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
             Board board_active,
             Board board_backbuffer)
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

static void
get_color_for_cell(int32_t row, int32_t col, Color *color)
{
    double proportion_b = (0.0 + col) / BOARD_WIDTH;
    double proportion_a = 1.0 - proportion_b;
    for (int i = 0; i < 3; i++) {
        color[i] = (Color)(proportion_a * color_alive_a[i] +
                           proportion_b * color_alive_b[i]);
    }
}

