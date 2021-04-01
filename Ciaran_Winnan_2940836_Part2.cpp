#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

// method to print the elements of the passed array
void printArray(int *arr, int size)
{
	for (int i = 0; i < size - 1; i++)
	{
		printf("%d,", arr[i]);
	}

	// Will remove the last comma from the last element in the array
	if (size > 0)
	{
		printf("%d\n", arr[size - 1]);
	}
}

// method to sum the elements in the passed array
int sum(int *arr, int size)
{
	int total_sum = 0;
	for (int i = 0; i < size; i++)
	{
		total_sum += arr[i];
	}

	return total_sum;
}

// method to calculate the sum of differences of the elements in the array
float sum_differences(int *arr, int size, float mean)
{

	float sum_difference = 0;

	for (int i = 0; i < size; i++)
	{
		float difference = arr[i] - mean;

		float difference_squared = difference * difference;

		sum_difference += difference_squared;
	}

	return sum_difference;
}

int world_size;
int world_rank;

int main(int argc, char **argv)
{

	// see if we can initialise the mpi library this is always the first thing that we
	// must do at the start of an MPI program
	MPI_Init(NULL, NULL);

	// one of the important tasks that we have to establish is how many processes are in
	// this MPI instance. this will determine who and what we have to communicate with
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// another important job is finding out which rank we are. we can use this rank
	// number to assign seperate jobs to different mpi units
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	std::cout << "Ciaran Winnan 2940836 Distributed Systems Assignment 1 - Part 2" << std::endl;

	// Saving start time of the execution of program
	double start_time = MPI_Wtime();

	// intializing the full size of the array
	int full_array_size = 0;

	if (world_rank == 0)
	{
		// getting user input for size of the array
		std::cout << "Input a size for the array:" << std::endl;
		std::cin >> full_array_size;
	}

	// Intializing the array
	int *full_array = (int *)malloc(full_array_size * sizeof(int));

	// partition size used by each node
	int partiton_size = 0;

	if (world_rank == 0)
	{

		// coordinator will add random numbers to full array
		for (int i = 0; i < full_array_size; i++)
		{
			full_array[i] = (rand() % 50) + 1;
		}

		// coordinator will decide on partition size
		partiton_size = full_array_size / world_size;
	}

	// Coordinator will broadcast the partition size to all slave nodes
	MPI_Bcast(&partiton_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	;

	// intializing the array that each node will be responcible for, will use the partition size determined by the coordinator
	int *node_array = (int *)malloc(partiton_size * sizeof(int));

	//Scatter operation to all nodes by the coordinator
	MPI_Scatter(full_array, partiton_size, MPI_INT, node_array, partiton_size, MPI_INT, 0, MPI_COMM_WORLD);

	// Nodes will calculate their individual means
	float node_mean = sum(node_array, partiton_size) / (float)partiton_size;

	// Coordinator will collect sums from each node
	float sum_of_means = 0;
	MPI_Reduce(&node_mean, &sum_of_means, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

	// Coordinator will calculate and broadcast the mean of means to all slave nodes
	float mean_of_means = 0;
	if (world_rank == 0)
	{
		mean_of_means = sum_of_means / world_size;
	}
	MPI_Bcast(&mean_of_means, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

	// Each node will compute their sum differences
	float node_sum_difference = sum_differences(full_array, partiton_size, mean_of_means);

	// Coordinator will calculate the total sum from all the nodes
	float total_sum_difference = 0;
	MPI_Reduce(&node_sum_difference, &total_sum_difference, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

	// Coordinator will print out the array, mean, standard deviation and time
	if (world_rank == 0)
	{
		// overall standard deviation calculation
		float standard_deviation = sqrt(total_sum_difference / partiton_size);

		// Full array printout
		std::cout << "\nDataset:" << std::endl;
		printArray(full_array, full_array_size);

		// overall mean
		std::cout << "\nMean: " << mean_of_means << std::endl;

		// overall standard deviation
		std::cout << "\nStandard deviation: " << standard_deviation << std::endl;

		// overall time taken
		std::cout << "\nTime: " << MPI_Wtime() - start_time << " seconds" << std::endl;
	}

	// clear up our memory before the program finishes
	delete full_array;
	delete node_array;
	MPI_Finalize();
	return 0;
}