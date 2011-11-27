#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <pthread.h>

#define MAX_ITERATIONS 5
#define ABSTEMP 0.00001
//Cooling
#define COOLING 0.9999

//Thread Pool Size
#define POOL 5

typedef struct{
	char name[20];
	char comment[50];
	char type[10];
	int  dimension;
	char wtype[10];
} InstanceData;

void printTour(int*);
void printMatrix();
double** readEuc2D(char*);
double tourCost(int*);

//Nearest Neighbour Function
int* nearestNeighbour();

//Swap Move
void swap();

//Simulated Annealing Procedure
void* SimulatedAnnealing();

//Instance Data
InstanceData ins;

//Distance Matrix
double **matrix;

//Best Tour
int *bestTour;

//Current and Next Tour
//int *currentTour;
//int *nextTour;

//Temperature
double temperature;

//Mutex

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
	int i;
	void *status;
	
	pthread_t *threads;
	threads = (pthread_t *)malloc(sizeof(pthread_t)*POOL);
	
	temperature = 100000;
	matrix = readEuc2D(argv[1]);

	bestTour = (int *)malloc(sizeof(int)*ins.dimension);
	
	for (i = 0; i < ins.dimension;i++)
		bestTour[i] = i;
	
	
	pthread_mutex_init(&mutex,NULL);
	
	for (i = 0; i < POOL; i++)
	{
		if (pthread_create(&threads[i], NULL, SimulatedAnnealing, (void *)i))
        {
                exit(-1);
        }
	}
	
	for (i = 0; i < POOL; i++)
	{
		if (pthread_join(threads[i], &status))
		{
			exit(-1);
		}
	}
	pthread_mutex_destroy(&mutex);
	
	printf("Best Distance: %lf\n", tourCost(bestTour));
	
	return 0;
}

void printTour(int *tour)
{
	int i;
	printf("TOUR: ");
	for (i = 0; i < ins.dimension; i++)
		printf("%d ", tour[i]);
	printf("\n");
}

void printMatrix()
{
	int i,j;
	
	for (i = 0; i < ins.dimension; i++)
	{
		for (j = 0; j < ins.dimension; j++)
			printf("%lf ", matrix[i][j]);
		printf("\n");
	}
}

double** readEuc2D(char* name)
{
	FILE *file;
	double **matrix;
	double **coord;
	int i,j;
	
	file = fopen(name, "r");
	fscanf(file, "NAME: %[^\n]s", ins.name);
	fscanf(file, "\nTYPE: TSP%[^\n]s", ins.type);
	fscanf(file, "\nCOMMENT: %[^\n]s", ins.comment);
	fscanf(file, "\nDIMENSION: %d",&ins.dimension);
	fscanf(file, "\nEDGE_WEIGHT_TYPE: %[^\n]s", ins.wtype);
	fscanf(file, "\nNODE_COORD_SECTION");
	//fscanf(file, "%d", cities);
	//rewind(file);
	
	
	coord = (double **)malloc(ins.dimension*sizeof(double*));
	for(i=0;i<ins.dimension;i++) 
		coord[i] = (double*) malloc(2*sizeof(double));
	for(i=0;i<ins.dimension;i++)
		fscanf(file, "\n %*[^ ] %lf %lf", &coord[i][0], &coord[i][1]);
	
	
	//Build Distance Matrix
	matrix = (double **)malloc(sizeof(double*)*(ins.dimension));
	for (i = 0; i < ins.dimension; i++)
		matrix[i] = (double *)malloc(sizeof(double)*(ins.dimension));
	
	
	for (i = 0; i < ins.dimension; i++)
	{
		for (j = i + 1 ; j < ins.dimension; j++)
		{
			matrix[i][j] = sqrt(pow(coord[i][0] - coord[j][0],2) + pow(coord[i][1] - coord[j][1],2));
			matrix[j][i] = matrix[i][j];
		}
	}
	free(coord);
	return matrix;
}

double tourCost(int *tour)
{
	int i;
	double value = 0.0;
	
	for (i = 0; i < ins.dimension - 1; i++)
		value += matrix[tour[i]][tour[i+1]];
	
	return value + matrix[tour[0]][tour[ins.dimension-1]];
}

int* nearestNeighbour()
{
	int *tour;
	int *visited;
	double nearestDistance;
	int nearestIndex;
	int i,j;
	
	visited = (int*) malloc(ins.dimension*sizeof(int));
	tour    = (int*) malloc(ins.dimension*sizeof(int));
	for(i=0;i<ins.dimension;i++) visited[i] = 0;
	int start; 
	
	pthread_mutex_lock(&mutex);
	start = rand()%ins.dimension;
	pthread_mutex_unlock(&mutex);
	
	visited[start] = 1;
	tour[0] = start;
	
	for(i=1;i<ins.dimension;i++){
		nearestDistance = FLT_MAX;
		nearestIndex = 0;
		for(j=0;j<ins.dimension;j++){
			if(!visited[j] && matrix[tour[i-1]][j] < nearestDistance){
				nearestDistance = matrix[tour[i-1]][j];
				nearestIndex = j;
			}
		}
		tour[i] = nearestIndex;
		visited[nearestIndex] = 1;
	}
	
	free(visited);
	return tour;
}

void swap(int *currentTour, int *nextTour)
{
	int i,auxiliar;
	
	
	for (i = 0; i < ins.dimension; i++)
		nextTour[i] = currentTour[i];
		
	pthread_mutex_lock(&mutex);	
	int first = (rand() % (ins.dimension - 1)) + 1;
	int second = (rand() % (ins.dimension - 1)) + 1;
	pthread_mutex_unlock(&mutex);
	
	auxiliar = nextTour[first];
	nextTour[first] = nextTour[second];
	nextTour[second] = auxiliar;
	

}

void* SimulatedAnnealing(void *id)
{
	int *currentTour;
	int *nextTour;
	
	
	double distance;
	double delta;
	int i;
	long seed;
	
	seed = time(NULL);
	srand(seed+(int)id*12345);
	
	
	currentTour = nearestNeighbour();
	
	
	distance = tourCost(currentTour);
	
	nextTour = (int *)malloc(sizeof(int)*ins.dimension);
	
	while (temperature > ABSTEMP)
	{
		//Start Critical Section
		
		swap(currentTour, nextTour);
		delta = tourCost(nextTour) - distance;
		if (((delta < 0) || (distance > 0)) && (exp(-delta/temperature) > (double)rand()/RAND_MAX))
		{
			for (i = 0; i < ins.dimension; i++)
				currentTour[i] = nextTour[i];
			distance = delta + distance;
		}
		
		pthread_mutex_lock(&mutex);
		temperature *= COOLING;
		pthread_mutex_unlock(&mutex);
		//End Critical Section
		
	}
	
	pthread_mutex_lock(&mutex);
	if (tourCost(bestTour) > tourCost(currentTour))
	{
		
		for (i = 0; i < ins.dimension; i++)
			bestTour[i] = currentTour[i];
		
	}
	pthread_mutex_unlock(&mutex);	
	
	return(NULL);
}