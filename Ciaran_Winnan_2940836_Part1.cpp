/** includes **/
#include <iostream>
#include <cstdlib>
#include <mpi.h>
/** messages for communicating tasks **/
int COMPUTE_AVERAGE = 1;
/** the world rank and size that will be useful in many functions **/
int world_size;
int world_rank;

/* methods that will be used to generate the hash */
int worldsWorstworldsWorstHash1(int ip) {
	for (int l = 1; l < 100; l++) {
		ip = ip * l % 254;
	}
	return ip;
}

int worldsWorstworldsWorstHash2(int ip) {
	for (int l = 1; l < 50; l++) {
		ip = ((ip * l) + 2) % 254;
	}
	return ip;
}

void coordinator(void) {
	std::cout << "Ciaran Winnan 2940836 Ditributed Systems Assignment 1 - Part 1" << std::endl;
	int message;
	// tell the next node that we have to start computing an average
	MPI_Send(&COMPUTE_AVERAGE, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	// we will expect the last node to send us a message to compute our average. get it
	// and ignore it
	MPI_Recv(&message, 1, MPI_INT, world_size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	// compute our average and print out the results
	int difference_hashes = worldsWorstworldsWorstHash1(422) - worldsWorstworldsWorstHash2(233);
	//std::cout << "coordinator 0 Hash1 Result: " << first_hash << std::endl;
	//std::cout << "coordinator 0 Hash2 Result: " << second_hash << std::endl;
	std::cout << "Rank 0 sending: " << difference_hashes << std::endl;
	// ask node one to compute it's average by sending a single floating point value this
	// will add in it's average and pass to node 2 etc
	// when we get the result back we will have the full average
	MPI_Send(&difference_hashes, 1, MPI_FLOAT, 1, 0, MPI_COMM_WORLD);
	MPI_Recv(&difference_hashes, 1, MPI_FLOAT, world_size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	// print out the average by dividing by the number of nodes and exit

	// after the final node completes the calculation and the assignment is finished. The last node will send
	// its results to the coordinator and will do another calculation to complate the ring (not reuqired)
	int first_hash = worldsWorstworldsWorstHash1(difference_hashes);
	int second_hash = worldsWorstworldsWorstHash2(difference_hashes);
	//std::cout << "Final Result: Hash1 = " << first_hash << ", Hash2 = " << second_hash << " (Calculation made by coordinator after final node result!!)" << std::endl;
}

void computeAverage(void) {
	int message;
	int sent_difference;
	// wait till we get the compute message after this tell the next node to start
	// computing an average before computing our average
	MPI_Recv(&message, 1, MPI_INT, (world_rank + world_size - 1) % world_size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Send(&COMPUTE_AVERAGE, 1, MPI_INT, (world_rank + 1) % world_size, 0, MPI_COMM_WORLD);
	// at some point we will get a message that will contain a current total of the
	// overall average. take that value add our average
	// to it and pass it on to the next node
	MPI_Recv(&sent_difference, 1, MPI_FLOAT, (world_rank + world_size - 1) % world_size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	// storing the hash values returned from the method calls
	int first_hash = worldsWorstworldsWorstHash1(sent_difference);
	int second_hash = worldsWorstworldsWorstHash2(sent_difference);
	// Only if the result is from the last node, then run print command for the final result
	if ((world_size - 1) == world_rank) {
		std::cout << "Result:	hash1= " << first_hash << " hash2= " << second_hash << std::endl;
	}
	// calculate the difference
	int difference_hashes = first_hash - second_hash;
	// output what the node is sending
	if (!(world_size-1 == world_rank)) {
		std::cout << "Rank	" << world_rank << " sending: " << difference_hashes << std::endl;
	}
	// send the diffrence_hash value to the next node in the world rank 
	MPI_Send(&difference_hashes, 1, MPI_FLOAT, (world_rank + 1) % world_size, 0, MPI_COMM_WORLD);
}


int main(int argc, char** argv) {
	// see if we can initialise the mpi library this is always the first thing that we
	// must do at the start of an MPI program
	MPI_Init(NULL, NULL);
	// one of the important tasks that we have to establish is how many processes are in
	// this MPI instance. this will determine who and what we have to communicate with
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	// another important job is finding out which rank we are. we can use this rank
	// number to assign seperate jobs to different mpi units
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	// compute the average of 400 numbers by using ring like communication. whoever is
	// node zero should be given the job of coordinator
	if (world_rank == 0)
		coordinator();
	else
		computeAverage();
	// before we can end our application we need to finalise everything so MPI can shut
	// down properly
	MPI_Finalize();
	// standard C/C++ thing to do at the end of main
	return 0;
}