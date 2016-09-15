/*
 * Defines some game actions for Conway's Game of Life.
 *
 * @author: Johnathan Davis
 */

#ifndef LIFE_ACTIONS_H
#define LIFE_ACTIONS_H

#include "macros.h"
#include "types.h"
#include "vars.h"

void
populate_board(int32_t board_h, int32_t board_w, Board board);

void
zero_board(int32_t board_h, int32_t board_w, Board board);

void
zero_board_region(int32_t board_h1, int32_t board_w1,
                  int32_t board_h2, int32_t board_w2,
                  Board board);

void
init_board_rects(BoardRect board_rects);

Cell *
get_cell_by_coord(int32_t board_h, int32_t board_w, int32_t x, int32_t y,
                  Board board);

void
advance_all_cells(int32_t board_h, int32_t board_w,
                  Board board_in, Board board_out);

int
toggle_cells_from_clicks(int32_t board_h, int32_t board_w,
                         Board board_clicks, Board board);

#endif /* LIFE_ACTIONS_H */

