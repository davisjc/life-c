/*
 * Defines rendering functionality for Conway's Game of Life.
 *
 * @author: Johnathan Davis
 */

#ifndef LIFE_RENDER_H
#define LIFE_RENDER_H

#include <stdint.h>

#include "types.h"


int
sdl_init(char *title, int width, int height,
         SDL_Window **win, SDL_Renderer **ren);

void
sdl_teardown(SDL_Window *win,
             SDL_Renderer *ren,
             const char *offending_func_name /* optional name of SDL func */);

void
sdl_log_error(const char *offending_func_name);

void
get_color_for_cell(int32_t row, int32_t col, int32_t board_h, int32_t board_w,
                   Color *color);

void
render_blank(SDL_Renderer *ren);

void
render_cells(SDL_Renderer *ren, int32_t board_h, int32_t board_w,
             BoardRect rects, Board board);

void
record_fps(void);

#endif /* LIFE_RENDER_H */

