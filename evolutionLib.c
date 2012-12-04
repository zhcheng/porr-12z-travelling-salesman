
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include "globals.h"
#include "evolutionLib.h"

//8948061100002391241

void swap(int *a, int *b){
	int tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

void swapf(float *a, float* b){
	float tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

void swapRows(int* a, int* b){
	int i,tmp;
	for(i = 0; i<towns_count; ++i){
		tmp = a[i];
		a[i] = b[i];
		b[i] = tmp;
	}
}

void prepareChild(int a, int b, int child, int x, int y){
	int i = 0;
	for(i = 0; i < x; ++i){
		population[child][i] = population[b][i];
	}
	for(i = x; i < y; i++){
		population[child][i] = population[a][i];
	}
	for(i = y; i < towns_count; i++){
		population[child][i] = population[b][i];
	}
}

void checkDuplicates(int a, int b, int child, int x, int y){
	int *counts;
	int i,tmp;

	counts = (int*)malloc((sizeof(int))*towns_count);
	memset(counts,-1,(sizeof(int))*towns_count);

	for(i = x; i<y; ++i){
		counts[population[child][i]] = i;
	}
	for(i = 0; i<x; ++i){
		counts[population[child][i]] = i;
	}
	for(i = y; i<towns_count; ++i){
		counts[population[child][i]] = i;
	}

	for(i = x; i<y; ++i){
		if((counts[population[child][i]] < x) || (counts[population[child][i]] >= y)){
			tmp = i;

			while(counts[population[b][tmp]] >= 0){
				tmp = counts[population[b][tmp]];	
			}
			population[child][i] = population[b][tmp];
		}
	}

	free(counts);
}

int pmx(int parentA, int parentB, int childA, int childB, unsigned *seed){
	int x,y;

	x = rand_my(seed) % towns_count;
	y = rand_my(seed) % towns_count;

	if(x>y){
		swap(&x,&y);
	}

	//printf("x: %d , y: %d\n",x,y);
	
	prepareChild(parentA,parentB,childA,x,y);
	checkDuplicates(parentA,parentB,childA,x,y);

	if(childB >= 0){
		prepareChild(parentB,parentA,childB,x,y);
		checkDuplicates(parentB,parentA,childB,x,y);
	}

	return 0;
 }

 void mutate_random(int child, unsigned *seed){
 	int i,r,tmp;
 	int* newChild;
 	float childOverallLength = 0;

 	newChild = (int*)malloc(sizeof(int)*towns_count);
 	memset(newChild,-1,(sizeof(int))*towns_count);

 	for(i = 0; i < towns_count; ++i){
	 			newChild[i] = population[child][i];
 	}

 	for(i = 0; i < towns_count; ++i){
		if(rand_my(seed)%8 == 0){
			r = rand_my(seed)%towns_count;
			tmp = newChild[r];
			newChild[r] = newChild[i];
			newChild[i] = tmp;
		}
 	}

	for(i = 0; i < towns_count-1; i++){	
		childOverallLength += weights[newChild[i]][newChild[i+1]];
	}
 	
 	if(childOverallLength < overall_lengths[child]){
 		swapRows(newChild,population[child]);
 		overall_lengths[child] = childOverallLength;
 	}

 	free(newChild);
 }

  void mutate_swap_neighbours(int child, unsigned* seed){
 	int a,i, tmp;
	float orig_length, new_length;
 	for(i = 0; i < 20; ++i){
 		a = rand_my(seed) % (towns_count - 3);
	 	a += 1;
		orig_length = weights[population[child][a-1]][population[child][a]] +
	 		weights[population[child][a+1]][population[child][a+2]];
	 	new_length = weights[population[child][a-1]][population[child][a+1]] +
	 		weights[population[child][a]][population[child][a+2]];

	 	if(orig_length > new_length){
	 		tmp = population[child][a];
	 		population[child][a] = population[child][a+1];
	 		population[child][a+1] = tmp;
	 	}
	}
  }

 void mutate_reverse_swap(int child, unsigned* seed){
 	int a,b,i, tmp;
	float orig_length, new_length;

 	for(i = 0; i < 20; ++i){
 		a = rand_my(seed) % (towns_count - 10);
		while((b = rand_my(seed) % towns_count) == a || b+1 == a || b-1 == a){}
 		
 		if(a > b){
 			tmp = a; a = b; b = tmp;
 		}

	 	orig_length = weights[population[child][a]][population[child][a+1]] +
	 		weights[population[child][b-1]][population[child][b]];
	 	new_length = weights[population[child][a]][population[child][b-1]] +
	 		weights[population[child][a+1]][population[child][b]];

	 	if(orig_length > new_length){
	 		while(a < b){
	 			tmp = population[child][a];
	 			population[child][a] = population[child][b];
	 			population[child][b] = tmp;
	 			++a; --b;
	 		}
	 	}
	}
}



 void mixinChildren(){
 	int i,j,c,r;

 	c = mi_constant/m_constant;

 	for(i = mi_constant; i < M_MI; ++i){
 		for(j = 0; j<c; ++j){
 			r = rand() % mi_constant;
 			if(overall_lengths[r]>overall_lengths[i]){
 				swapRows(population[r],population[i]);
 				swapf(&overall_lengths[r],&overall_lengths[i]);
 				break;
 			}
 		}
 	}
 }









