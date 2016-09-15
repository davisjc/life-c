
#include "actions.h"

void
populate_board(int32_t board_h, int32_t board_w, Board board)
{
    for (int32_t row = 0; row < board_h; row++)
        for (int32_t col = 0; col < board_w; col++)
            board[row][col] = (rand() % 100 < LUCK_LIFE_START);
}

void
zero_board(int32_t board_h, int32_t board_w, Board board)
{
    for (int32_t row = 0; row < board_h; row++)
        for (int32_t col = 0; col < board_w; col++)
            board[row][col] = 0;
}

void
zero_board_region(int32_t board_h1, int32_t board_w1,
                  int32_t board_h2, int32_t board_w2,
                  Board board)
{
    int32_t board_h_low = board_h1;
    int32_t board_h_high = board_h2;
    if (board_h_high < board_h_low)
        swap(int32_t, board_h_high, board_h_low);
    int32_t board_w_low = board_w1;
    int32_t board_w_high = board_w2;
    if (board_w_high < board_w_low)
        swap(int32_t, board_w_high, board_w_low);

    for (int32_t row = 0; row < board_h_high; row++)
        for (int32_t col = 0; col < board_w_high; col++)
            if (row >= board_h_low || col >= board_w_low)
                board[row][col] = 0;
}

void
init_board_rects(BoardRect board_rects)
{
    for (int32_t row = 0; row < BOARD_H_MAX; row++) {
        for (int32_t col = 0; col < BOARD_W_MAX; col++) {
            board_rects[row][col].x = GRID_SIZE + (CELL_SIZE + GRID_SIZE) * col;
            board_rects[row][col].y = GRID_SIZE + (CELL_SIZE + GRID_SIZE) * row;
            board_rects[row][col].w = CELL_SIZE;
            board_rects[row][col].h = CELL_SIZE;
        }
    }
}

Cell *
get_cell_by_coord(int32_t board_h, int32_t board_w, int32_t x, int32_t y,
                  Board board)
{
    uint32_t col = x / (GRID_SIZE + CELL_SIZE);
    uint32_t row = y / (GRID_SIZE + CELL_SIZE);
    if (row >= board_h)
        row = board_h - 1;
    if (col >= board_w)
        col = board_w - 1;
    return board[row] + col;
}

void
advance_all_cells(int32_t board_h, int32_t board_w,
                  Board board_in, Board board_out)
{
    size_t neighbor_count;
    int32_t row_prev;
    int32_t row_next;
    int32_t col_prev;
    int32_t col_next;
    int is_alive;

    for (int32_t row = 0; row < board_h; row++) {
        for (int32_t col = 0; col < board_w; col++) {
            /* Wrap around the edge of the board. */
            row_prev = (0 < row) ? row - 1 : board_h - 1;
            row_next = (row < board_h - 1) ? row + 1 : 0;
            col_prev = (0 < col) ? col - 1 : board_w - 1;
            col_next = (col < board_w - 1) ? col + 1 : 0;

            /* Count cell's neighbors. */
            neighbor_count = 0;
            neighbor_count += board_in[row_prev][col]; /* upper */
            neighbor_count += board_in[row_next][col]; /* lower */
            neighbor_count += board_in[row][col_prev]; /* left */
            neighbor_count += board_in[row][col_next]; /* right */
            neighbor_count += board_in[row_prev][col_prev]; /* upper-left */
            neighbor_count += board_in[row_prev][col_next]; /* upper-right */
            neighbor_count += board_in[row_next][col_prev]; /* lower-left */
            neighbor_count += board_in[row_next][col_next]; /* lower-right */

            /* Determine life. */
            is_alive = board_in[row][col];
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
    }
}

int
toggle_cells_from_clicks(int32_t board_h, int32_t board_w,
                         Board board_clicks, Board board)
{
    int dirty = 0;
    for (int32_t row = 0; row < board_h; row++) {
        for (int32_t col = 0; col < board_w; col++) {
            if (board_clicks[row][col]) {
                /* Kill if alive; revive if dead. */
                board[row][col] = !board[row][col];
                dirty = 1;
            }
        }
    }
    return dirty;
}

