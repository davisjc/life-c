/*
 * Defines some game actions for Conway's Game of Life.
 *
 * @author: Johnathan Davis
 */

#ifndef LIFE_ACTIONS_H
#define LIFE_ACTIONS_H

#include "types.h"
#include "vars.h"

void
populate_board(Board board);

void
zero_board(Board board);

void
init_board_rects(BoardRect board_rects);

size_t
get_neighbor_count(Board board, int32_t row, int32_t col);

Cell *
get_cell_by_coord(Board board, int32_t x, int32_t y);

void
advance_all_cells(Board board_in, Board board_out);

int
toggle_cells_from_clicks(Board board_clicks, Board board);

void
advance_cell(int32_t row, int32_t col, Board board_in, Board board_out);

#endif /* LIFE_ACTIONS_H */

