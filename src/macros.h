/*
 * Global macros for Conway's Game of Life.
 *
 * @author: Johnathan Davis
 */

#ifndef LIFE_MACROS_H
#define LIFE_MACROS_H

#define swap(t, a, b) { \
    t temp = a; \
    a = b; \
    b = temp; \
}

#define cell_length_to_px(cell_length) (GRID_SIZE + (CELL_SIZE + GRID_SIZE) * \
    cell_length)

#endif /* LIFE_MACROS_H */

