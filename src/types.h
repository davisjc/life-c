/*
 * Defines types used for Conway's Game of Life.
 *
 * @author: Johnathan Davis
 */

#ifndef LIFE_TYPES_H
#define LIFE_TYPES_H

#include "SDL2/SDL.h"
#include "vars.h"
#include <stdint.h>

typedef uint8_t Cell; /* 0 = dead; 1 = alive */

typedef Cell (*Board)[BOARD_W_INIT];

typedef SDL_Rect (*BoardRect)[BOARD_W_INIT];

typedef uint8_t Color;

#endif /* LIFE_TYPES_H */

