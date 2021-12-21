#ifndef __GOL_H__
#define __GOL_H__
/**
 * File: gol.h
 *
 * Header file of the game of life simulator functions.
 */

/**
 * Creates an initializes the world based on the given configuration file.
 *
 * @param config_filename The name of the file containing the simulation
 *    configuration data (e.g. world dimensions)
 * @param num_cols Location where to store the width of the world.
 * @param num_rows Location where to store the height of the world.
 *
 * @return A 1D array representing the created/initialized world, or NULL if
 *   if there was an problem with initialization.
 */
int *initialize_world(char *config_filename, int *num_cols, int *num_rows);

/**
 * Updates the world for one step of simulation, based on the rules of the
 * game of life.
 *
 * @param world The world to update.
 * @param num_cols The width of the world.
 * @param num_rows The height of the world.
 */
void update_world(int *world, int *world_copy, int num_cols, int num_rows, int start_row, int end_row);

/**
 * Prints the given world using the ncurses UI library.
 *
 * @param world The world to print.
 * @param num_cols The width of the world.
 * @param num_rows The height of the world.
 * @param turn The current turn number.
 */
void print_world(int *world, int num_cols, int num_rows, int turn);

#endif
