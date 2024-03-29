#include <stdio.h> // printf
#include <string.h> // strlen
#include <stdlib.h> // malloc?
// GLUT and OpenGL libraries
#include <GL/glut.h>
#include <GL/gl.h>

#include "globals.h" // MAX_COORD

#ifdef USE_MPI
#include <mpi.h>
#define MPI_NEXT_NODE ( (mpi_node_id+1)%mpi_node_count )
#define MPI_PREV_NODE ( (mpi_node_id+mpi_node_count-1)%mpi_node_count )
#endif

#include "evolution.h" // evo_iter, print_population_info
#include "evolutionLib.h" // mixin
#include "glut_impl.h" // This header
#include "qsortPopulation.h"

// ----------------------------------------------------------------------------

void drawString(char *s) {
	unsigned int i;
	for (i = 0; i < strlen(s); i++)
		glutBitmapCharacter (GLUT_BITMAP_HELVETICA_10, s[i]);
}

// ----------------------------------------------------------------------------

void draw_best(void) {

	int i;
	static char label[100];

	find_best();

	glColor3f(1.0f, 0.8f, 0.2f);
	glLineWidth(1);
	glBegin(GL_LINE_LOOP);
	for(i = 0; i < towns_count; i++) {
		glVertex2f(towns[population[best_index][i]].x,
			towns[population[best_index][i]].y);
	}
	glEnd();
	
	// Axis Labels
	glColor3f (1.0F, 1.0F, 1.0F);
	sprintf_s(label, 100, "Best value: %f Iteration: %lu", best_value, global_iteration_counter);
	glRasterPos2f (-MAX_COORD, -MAX_COORD*1.05);
	drawString(label);

}

// ----------------------------------------------------------------------------

void reshape(int w, int h) {
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glViewport (0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D (-MAX_COORD*1.1, MAX_COORD*1.1, -MAX_COORD*1.1, MAX_COORD*1.1);
	glEnable (GL_LINE_SMOOTH);
	glEnable (GL_LINE_STIPPLE);
}

// ----------------------------------------------------------------------------

void display(void) {
	
	int i;

	// Clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw outsite box
	glColor3f(0.1f, 0.8f, 0.1f);
	glLineWidth(1);
	glBegin(GL_LINE_LOOP);
	glVertex2f(-MAX_COORD, -MAX_COORD);
	glVertex2f( MAX_COORD, -MAX_COORD);
	glVertex2f( MAX_COORD,  MAX_COORD);
	glVertex2f(-MAX_COORD,  MAX_COORD);
	glEnd();
	
	// Draw population
	draw_best();

	// Draw towns
	glColor3f(1.0f, 0.2f, 0.2f);
	for(i = 0; i < towns_count; i++) {
		glBegin(GL_LINE_LOOP);
		glVertex2f(towns[i].x+5, towns[i].y+5);
		glVertex2f(towns[i].x+5, towns[i].y-5);
		glVertex2f(towns[i].x-5, towns[i].y-5);
		glVertex2f(towns[i].x-5, towns[i].y+5);
		glEnd();
 	}
	
	glutSwapBuffers();
}

// ----------------------------------------------------------------------------

void keyboard(unsigned char key_code, int xpos, int ypos) {
	switch (key_code) {
		case 'r':
		case 'R':
		case 32: // Spacebar
			display();
			break;
		case 'i':
		case 'I':
			print_population_info(1);
			break;
		case 's':
		case 'S':
			print_summary_info(1);
			break;
		case 'q':
		case 'Q':
		case 27 : // Esc
			glFinish();
			terminate();
		}
}

// ----------------------------------------------------------------------------

void ips_window_title(void) {

	static long start_time = 0;
	static unsigned long iters = 0;
	
	long time;
	float ips;
	char buf[30];
	
	if (iters == 0) {
		global_start_time = clock_ms();
	}

	time = clock_ms();

	if (time - start_time >= 1000) { // one second passed
		
		ips = (float)(global_iteration_counter - iters) / (time - start_time) * 1000;
		sprintf_s(buf, 30, "Evo-salesman %6.2f IPS", ips);
		start_time = time;
		iters = global_iteration_counter;
		glutSetWindowTitle(buf);

	} // if

} // fps_window_title()

// ----------------------------------------------------------------------------

void idle(void) {
	
#ifdef USE_MPI
	int *cities_array;
	int recv_flag;
	MPI_Status status;
	int i = 0,j = mi_constant, k = 0, count;
	unsigned seed;
	int y;

	seed = 25234 + global_iteration_counter;
#endif


#ifdef USE_MPI
	//Jeśli jest coś do odebrania to odbieramy i pomijamy tworzenie dzieci przyjmując za dzieci odebrane osobniki

	//******** Receiving ********//
	// Test if can receive message
	MPI_Iprobe(MPI_PREV_NODE, 0, MPI_COMM_WORLD, &recv_flag, &status);
	// If can receive
	if (recv_flag) {
		printf("can receive message %d\n", mpi_node_id);
		count = TRANSFER_COUNT * towns_count;
		// Alloc buffer
		cities_array = (int*)malloc(count * sizeof(int));

		// Blocking receive
		MPI_Recv((void*)cities_array, count, MPI_INT, MPI_PREV_NODE, 0, MPI_COMM_WORLD, &status);

		// Print what've received
		printf("Node %d received (from prev: %d) buffer: %d, %d, %d ...\n",
			mpi_node_id, MPI_PREV_NODE, cities_array[0], cities_array[1], cities_array[2]);
	
		//----------------------------------- wplatanie --------------------
		while(i < count){
			population[j][k] = cities_array[i];

			++i; ++k;
			if(i%towns_count == 0){
				overall_lengths[j] = calculate_overall_length(j);
				++j; k = 0;
			}
		}

		mixin(TRANSFER_COUNT+mi_constant);
		//--------------------------------------

		// Free buffer
		free(cities_array);
	}else{ 
		evo_iter();
	}
	
#else
	// Compute next generation
	evo_iter();
#endif


	// Increase counter
	++global_iteration_counter;

	// Every n'th iteration
	if (global_iteration_counter%PRINT_EVERY_ITERS == 0) {
		// Force to print population info
		print_population_info(1);
	}
	else {
		// Print only if changed
		print_population_info(0);
	}

	// Iterations per second
	ips_window_title();

#ifdef USE_MPI

	//******** Sending ********//
	// If not sending to itself

	if (mpi_node_id != MPI_NEXT_NODE && (rand_my(&seed)%SEND_EVERY_ITER)==0) {
		printf("sending data: %d\n",mpi_node_id);
		
		// Alloc buffer
		cities_array = (int*)malloc(TRANSFER_COUNT * towns_count * sizeof(int));

		//qsort population so best are first
		qsortPopulation(0,mi_constant);

		//copy values
		for(i = 0; i < TRANSFER_COUNT; ++i){
			y = i*towns_count;
			for(j = 0; j < towns_count; ++j){
				cities_array[y + j] = population[i][j];
			}
		}
		
		// // TODO: Prepare cities_array to send from best of population
		// cities_array[0] = 123456789;
		// cities_array[1] = mpi_node_id;
		// cities_array[2] = rand_my(&seed)%10;

		// Debug what we send
		// printf("Node %d sending (to next %d) buffer: %d, %d, %d ...\n", 
			// mpi_node_id, MPI_NEXT_NODE, cities_array[0], cities_array[1], cities_array[2]);
		
		// Blocking sending
		MPI_Send((void*)cities_array, TRANSFER_COUNT * towns_count, MPI_INT, MPI_NEXT_NODE, 0, MPI_COMM_WORLD);

		// Free buffer
		free(cities_array);
	}
	
#endif
}

// ----------------------------------------------------------------------------
