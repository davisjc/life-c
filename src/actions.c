
#include "actions.h"

void
populate_board(Board board)
{
    for (int32_t row = 0; row < BOARD_HEIGHT; row++)
        for (int32_t col = 0; col < BOARD_WIDTH; col++)
            board[row][col] = (rand() % 100 < LUCK_LIFE_START);
}

void
zero_board(Board board)
{
    for (int32_t row = 0; row < BOARD_HEIGHT; row++)
        for (int32_t col = 0; col < BOARD_WIDTH; col++)
            board[row][col] = 0;
}

void
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

size_t
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

Cell *
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

void
advance_all_cells(Board board_in, Board board_out)
{
    for (int32_t row = 0; row < BOARD_HEIGHT; row++) {
        for (int32_t col = 0; col < BOARD_WIDTH; col++) {
            advance_cell(row, col, board_in, board_out);
        }
    }
}

int
toggle_cells_from_clicks(Board board_clicks, Board board)
{
    int dirty = 0;
    for (int32_t row = 0; row < BOARD_HEIGHT; row++) {
        for (int32_t col = 0; col < BOARD_WIDTH; col++) {
            if (board_clicks[row][col]) {
                /* Kill if alive; revive if dead. */
                board[row][col] = !board[row][col];
                dirty = 1;
            }
        }
    }
    return dirty;
}

void
advance_cell(int32_t row, int32_t col, Board board_in, Board board_out)
{
    size_t neighbor_count = get_neighbor_count(board_in, row, col);
    int is_alive = board_in[row][col];
    if (is_alive) {
        if (neighbor_count < 2)
            board_out[row][col] = DEAD;
        else if (neighbor_count > 3)
            board_out[row][col] = DEAD;
        else
            board_out[row][col] = ALIVE;
    } else { /* dead */
        if (neighbor_count == 3)
            board_out[row][col] = ALIVE;
        else
            board_out[row][col] = DEAD;
    }
}

