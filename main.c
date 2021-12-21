/**
 * File: main.c
 *
 * Main function for the parallel version of Conway's Game of Life.
 * Authors: Julia Paley and Tyler Kreider
 * Date: 12/08/21
 *
 
 */

#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <curses.h>
#include <pthread.h>

#include "gol.h"
//declare the ThreadData fields
struct ThreadData {
	int id;
	int *world;
	int width;
	int height;
	int delay;
	int num_turns;
	int start_row;
	int end_row;
	pthread_barrier_t *barrier;	
	int *world_copy;
};
//initialize the functions 
typedef struct ThreadData ThreadData;
void* thread_function(void* args);
void run_threads(int num_threads, int num_turns, int *world, int width, int height, int delay);
/**
 * Function that prints out how to use the program, in case the user forgets.
 *
 * @param prog_name The name of the executable.
 */
static void usage(char *prog_name) {
	fprintf(stderr, "usage: %s [-s] -c <config-file> -t <number of turns> -d <delay in ms> -p <parallelism>\n", prog_name);
	exit(1);
}

/*
 * Main function to run parallel game of life simulation
 *
 * @param argc, number of arguments
 * @param *argv[] the arguments in an array
 */
int main(int argc, char *argv[]) {

	// Step 1: Parse command line args 
	char *config_filename = NULL;

	int delay = 100; // default value for delay between turns is 100
	int num_turns = 20; // default to 20 turns per simulation
	char ch;
	int p = 1; //default value for p is 1
	int num_threads = 2; //default value for num_threads is 2

	// reads from the argument line assigniing -c, -t, -d, and -p or sets them
	// to default if no user entry
	while ((ch = getopt(argc, argv, "c:t:d:p:")) != -1) {
		switch (ch) {
			case 'c':
				config_filename = optarg;
				break;
			case 't':
				if (sscanf(optarg, "%d", &num_turns) != 1) {
					fprintf(stderr, "Invalid value for -t: %s\n", optarg);
					usage(argv[0]);
				}
				break;
			case 'd':
				if (sscanf(optarg, "%d", &delay) != 1) {
					fprintf(stderr, "Invalid value for -d: %s\n", optarg);
					usage(argv[0]);
				}
				break;
			case 'p':
				if (sscanf(optarg, "%d", &num_threads) != 1){
					fprintf(stderr, "Invalid value for -p: %s\n", optarg);
					usage(argv[0]);
				}
				break;
			default:
				usage(argv[0]);
		}
	}

	// if config_filename is NULL, then the -c option was missing.
	if (config_filename == NULL) {
		fprintf(stderr, "Missing -c option\n");
		usage(argv[0]);
	}

	// Print summary of simulation options
	fprintf(stdout, "Config Filename: %s\n", config_filename);
	fprintf(stdout, "Number of turns: %d\n", num_turns);
	fprintf(stdout, "Delay between turns: %d ms\n", delay);
	fprintf(stdout, "Parallelism: %d\n", p);
	fprintf(stdout, "Num threads: %d\n", num_threads);
	// Step 2: Set up the text-based ncurses UI window.
	initscr(); 	// initialize screen
	cbreak(); 	// set mode that allows user input to be immediately available
	noecho(); 	// don't print the characters that the user types in
	clear();  	// clears the window


	// Step 3: Create and initialze the world.
	int width, height;
	//creates initial world graph
	int *world = initialize_world(config_filename, &width, &height);

	if (world == NULL) {
		endwin();
		fprintf(stderr, "Error initializing the world.\n");
		exit(1);
	}
	// Step 4: Simulate for the required number of steps, printing the world
	// after each step.


	run_threads(num_threads, num_turns, world, width, height, delay);
	print_world(world, width, height, num_turns); // print final world

	// Step 5: Wait for the user to type a character before ending the
	// program. Don't change anything below here.

	// print message to the bottom of the screen (i.e. on the last line)
	mvaddstr(LINES-1, 0, "Press any key to end the program.");

	getch(); // wait for user to enter a key
	endwin(); // close the ncurses UI window
	free(world);//free the world memory
	return 0;
}


/*
 * This function uses barriers to synchronize multiple threads runnning the simulation
 * 
 * @param args The ThreadData struct which contains the parameters to the thread
 * function
 */
void* thread_function(void* args){
	ThreadData *myargs = (ThreadData*)args; //cast back to struct
	int total_rows = (myargs->end_row) - (myargs->start_row) + 1; //calculate total rows
	fprintf(stdout, "\rid %d: rows: %d:%d (%d)\n", myargs-> id, myargs-> start_row, myargs->end_row, total_rows);
	//iterate through number of turns
	for (int turn_number = 0; turn_number < myargs->num_turns; turn_number++) {
		//wait for threads and check for errors
		int bar = pthread_barrier_wait(myargs->barrier);
		if(bar != 0 && bar != PTHREAD_BARRIER_SERIAL_THREAD){
			perror("pthread_barrier_wait");
			exit(EXIT_FAILURE);
		}   
		
		//only the first thread prints and makes a copy of the world
		if(myargs->id == 0){ 
			for(int i = 0; i < myargs->width*myargs->height; i++){
				myargs->world_copy[i] = myargs->world[i];
			} 
			print_world(myargs->world,myargs-> width, myargs->height, turn_number);
        	usleep(1000 * myargs->delay);  //adds delay to see changes
		}   
		//wait for threads and check for errors
		bar = pthread_barrier_wait(myargs->barrier);
		if(bar != 0 && bar != PTHREAD_BARRIER_SERIAL_THREAD){
			perror("pthread_barrier_wait");
			exit(EXIT_FAILURE);
		}   

		update_world(myargs->world,myargs->world_copy, myargs->width, myargs->height, myargs->start_row, myargs->end_row);

	}
	return NULL;
}


/*
 * Function that will create Pthread threads, initialize the ThreadData struct
 * passed in as a parameter for our thread_function, and determines start and
 * end row for individual threads.
 *
 * @param num_threads The number of threads completing the simulation
 * @param num_turns The number of simulation turns
 * @param *world The world
 * @param width Total number of columns
 * @param height Total number of rows
 * @param delay Delay between turns
 */

void run_threads(int num_threads, int num_turns, int *world, int width, int height, int delay){
	int remainder = height % num_threads;
	int cur = 0;
	unsigned rows_per_thread = height/num_threads;

	//creates a new struct
	ThreadData *td = malloc(num_threads * sizeof(ThreadData));
	//creates space for new pthread ids
	pthread_t *tids = malloc(sizeof(pthread_t)*num_threads);
	//creates space for a copy of the world
	int *world_copy = malloc(width*height*sizeof(int));
	pthread_barrier_t shared_barrier;
	//inititalize barrier and check for errors
	if (pthread_barrier_init(&shared_barrier, NULL, num_threads) != 0) {
		perror("pthread_barrier_init");
		exit(EXIT_FAILURE);
	}
	int start = 0, end = 0;   
	//makes sure that a single row isn't split between multiple threads
	//thread row dimensions differences is never greater than 1
	for(int i=0; i < num_threads; i++){
		if(remainder > 0) {
			start = cur;
			end = cur + rows_per_thread;
			cur = end + 1;
			remainder--;
		}
		else{
			start = cur;
			end = cur + rows_per_thread - 1;
			cur = end + 1;

		}

		//these lines initialize the struct fields of the newly created struct
		td[i].id = i;
		td[i].num_turns = num_turns;
		td[i].world = world;
		td[i].width = width;
		td[i].height = height;
		td[i].delay =  delay;
		td[i].barrier = &shared_barrier;
		td[i].world_copy = world_copy;
		td[i].start_row = start;
		td[i].end_row = end;
	}
	//create threads and check for failure
	for(int i = 0; i < num_threads; i++){
		if(pthread_create(&tids[i], NULL, thread_function, &td[i]) != 0){
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}
	//join threads and check for failure
	for(int i = 0; i < num_threads; i++){
		if(pthread_join(tids[i], NULL) != 0){
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}
	}

	if(pthread_barrier_destroy(&shared_barrier) != 0){
		perror("pthread_barrier_destroy");
		exit(EXIT_FAILURE);
	}
	free(world_copy);
	free(tids);
	free(td);

}
