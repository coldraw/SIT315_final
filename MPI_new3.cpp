#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
// #include <windows.h>
#include <mpi.h>
#include <stdio.h>

using namespace std::chrono;
using namespace std;


int SZ = 16;    // size of matrices
int **A, **B, **C;


int num_tasks;
int tag = 1;
int rows;
int offset;

void init(int **&A, int rows, int cols, bool initialise);
void head(int num_tasks);
void node (int rank, int num_tasks);
void free_memory();

int main(int argc, char** argv){
    srand(time(0));
    

    // MPI initalisation and core functions
    int rank, name_len, tag=1; 
    char name[MPI_MAX_PROCESSOR_NAME];
    int res; 
    MPI_Status Stat;
    MPI_Request request;

    // Initialize the MPI environment
    MPI_Init(&argc,&argv);
    // Get the number of tasks/process
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    // Get the rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // Find the processor name
    MPI_Get_processor_name(name, &name_len);

    // Set the start time for the clock 
    auto start = high_resolution_clock::now();

    // initialise matrices
    int rows = SZ/num_tasks;
    init(A, SZ, SZ, true);
    init(B, SZ, SZ, true);
    init(C, SZ, SZ, false);

    if (rank == 0){
        // head(int num_tasks)    

        for (int dest = 1; dest < num_tasks; dest++){

        MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
        MPI_Send(&A[offset][0], rows * SZ, MPI_INT, dest, 1, MPI_COMM_WORLD);
        // MPI_Send(&B, SZ * SZ, MPI_INT, dest, 1, MPI_COMM_WORLD);
        offset = offset + rows;
        }

               cout << endl
         << "Head: blocks sent." << endl;    

    }

    MPI_Bcast(&B[0][0], SZ*SZ, MPI_INT, 0, MPI_COMM_WORLD);



    if (rank > 0){
        // node(rank,num_tasks);


    // Receive blocks
        int source = 0;
        MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&rows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&A[offset][0], rows * SZ, MPI_INT, source, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // MPI_Recv(&B, SZ * SZ, MPI_INT, source, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);        
    
             cout << endl
         << "Node: blocks received" << endl;


    // Calculation
        for (int i = offset; i < rows; i++){
            for (int j = 0; j < SZ; j++){
                C[i][j] = 0;
                for (int k = 0; k < SZ; k++){
                    C[i][j] += A[i][k] * B [k][j];
                }
            }
        }



                 cout << endl
         << "Node: calc completed." << endl;


    // Send blocks

        MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(&C[offset][0], rows * SZ, MPI_INT, 0, 2, MPI_COMM_WORLD);
    


             cout << endl
         << "Node: resutls sent" << endl;
    }

    if (rank == 0){
            // wait for return


    // receive blocks
        for (int i = 1; i < num_tasks; i++){
            int source = i;
            MPI_Recv(&offset, 1, MPI_INT, source, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&C[offset][0], rows * SZ, MPI_INT, source, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }


if (rank == 0){
        cout << endl
         << "Matrix C" << endl;
    for (int i = 0; i < SZ; i++){
        for (int j = 0; j < SZ; j++){
            int value = C[i][j];
            cout <<value<< " ";
        }
        cout << endl;              
    }
}
    
   
    if (rank == 0){
       auto stop = high_resolution_clock::now();

    // Calculate the duration of the addition process based on (stop time) - (start time) = duration
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl; 
    }
    
    free_memory();

    // Finalize the MPI environment
    MPI_Finalize();

    return 0;
}



void init(int **&A, int rows, int size, bool initialise)
{
    A = (int **)malloc(sizeof(int *) * size * size);
    int *tmp = (int *)malloc(sizeof(int) * size * size);

    for (int i = 0; i < size; i++)
    {
        A[i] = &tmp[i * size];
    }

    if (!initialise){
        return;
    }

    for (long i = 0; i < rows; i++)
    {
        for (long j = 0; j < size; j++)
        {
            A[i][j] = rand() % 6; // any number less than 100
        }
    }
}

void free_memory(){
    free(A);
    free(B);
    free(C);
}