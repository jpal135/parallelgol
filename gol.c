/**
 * File: gol.c
 *
 * Implementation of the game of life simulator functions.
 */

#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>

#include "gol.h"

/**
 * Given 2D coordinates, compute the corresponding index in the 1D array.
 *
 * @param col The x-coord we are converting
 * @param row The y-coord we are converting
 * @param num_cols The width of the world (i.e. number of columns)
 * @param num_rows The height of the world (i.e. number of rows)
 *
 * @return Index into the 1D world array that corresponds to (x,y)
 */
unsigned int translate_to_1D(int col, int row, unsigned num_cols, unsigned num_rows) {
	// If col or row coordinates are out of bounds, wrap them arounds

	
	
	if (col < 0) {
        col += num_cols;
    }
    else if (col >= (int)num_cols) {
        col -= num_cols;
    }

    if (row < 0) {
        row += num_rows;
    }
    else if (row >= (int)num_rows) {
        row -= num_rows;
    }

	return row*num_cols + col;
}

/**
 * Returns the number of neighbors around a given (x,y) point that are alive.
 *
 * @param world The world we are simulating
 * @param x x-coord whose neighbors we are examining
 * @param y y-coord whose neighbors we are examining
 * @param num_cols The width of the world
 * @param num_rows The height of the world
 *
 * @return The number of live neighbors around (x,y)
 */
unsigned int count_live_neighbors(int *world, int x, int y,
									unsigned num_cols, unsigned num_rows) {
	unsigned sum = 0;

	for (int col = x-1; col <= x+1; col++) {
		for (int row = y-1; row <= y+1; row++) {
			if (col == x && row == y) continue;
			unsigned index = translate_to_1D(col,row,num_cols,num_rows);
			if (world[index] == 1) {
				++sum;
			}
		}
	}

	return sum;
}

/**
 * Updates cell at given coordinate.
 *
 * @param curr_world World for the current turn (read-only).
 * @param next_world World for the next turn.
 * @param x x-coord whose neighbors we are examining
 * @param y y-coord whose neighbors we are examining
 * @param num_cols The width of the world
 * @param num_rows The height of the world
 */
void update_cell(int *curr_world, int *next_world, int x, int y, 
					int num_cols, int num_rows) {
	unsigned index = translate_to_1D(x, y, num_cols, num_rows);
	unsigned num_live_neighbors = count_live_neighbors(curr_world, x, y, num_cols, num_rows);
	if (curr_world[index] == 1
			&& (num_live_neighbors < 2 || num_live_neighbors > 3)) {
		/*
		 * With my cross-bow,
		 * I shot the albatross.
		 */
		next_world[index] = 0;
	}
	else if (num_live_neighbors == 3) {
		/*
		 * Oh! Dream of joy! Is this indeed
		 * The light-house top I see?
		 */
		next_world[index] = 1;
	}
}

int *initialize_world(char *config_filename, int *num_cols, int *num_rows) {
	FILE *config_file = fopen(config_filename, "r");
	if (config_file == NULL) {
		return NULL;
	}

	unsigned int num_pairs;
	if (fscanf(config_file, "%d", num_rows) != 1) {
		return NULL;
	}
	if (fscanf(config_file, "%d", num_cols) != 1) {
		return NULL;
	}
	if (fscanf(config_file, "%d", &num_pairs) != 1) {
		return NULL;
	}

	int *world = calloc(*num_cols * *num_rows, sizeof(int));

	for (unsigned i = 0; i < num_pairs; i++) {
		unsigned col, row;
		if (fscanf(config_file, "%u %u", &col, &row) != 2) {
			// couldn't read the coordinate pair!
			return NULL;
		}

		unsigned index = translate_to_1D(col, row, *num_cols, *num_rows);
		world[index] = 1;
	}

	fclose(config_file);

	return world;
}

void update_world(int *world, int *world_copy, int num_cols, int num_rows, int start_row, int end_row) {


	for (int y = start_row; y <= end_row; y++) {
		for (int x = 0; x < num_cols; x++) {
			update_cell(world_copy, world, x, y, num_cols, num_rows);
		}
	}

}

void print_world(int *world, int num_cols, int num_rows, int turn) {
	clear(); // clears the screen

	for (int row = 0; row < num_rows; row++) {
		for (int col = 0; col < num_cols; col++) {
			unsigned int index = translate_to_1D(col, row, num_cols, num_rows);
			if (world[index] == 1) {
				mvaddch(row, col, '@');
			}
			else {
				mvaddch(row, col, '.');
			}

		}
	}

	// print the turn number string below the board


	char turn_str[20];
    memset(turn_str, 0, 20); // init string to all 0's

    sprintf(turn_str, "Time Step: %d", turn);
    mvaddstr(num_rows + 1, 0, turn_str);


	refresh(); // displays the text we've added
}
