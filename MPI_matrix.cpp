#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
#include <windows.h>
#include <mpi.h>
#include <stdio.h>

using namespace std::chrono;
using namespace std;


// struct to pass parameters for block definition to the multiplication function
// struct ThreadArgs {
//     int start_row;
//     int end_row;
// };

// void* matrixBlockMultiplication(void* arg){
//     ThreadArgs* args = static_cast<ThreadArgs*>(arg);

//     for (int i = args->start_row; i < args->end_row; i++){
//         for (int j = 0; j < SZ; j++){
//             C[i][j] = 0;
//             for (int k = 0; k < SZ; k++){
//                 C[i][j] += A[i][k] * B [k][j];
//             }
//         }
//     }
//     return 0;
// }



int main(int argc, char** argv){
    srand(time(0));

    const int SZ = 4;    // size of matrices
    const int num_tasks;  // number of threads for parallel 
    const int block_size = SZ / num_tasks;

    int *A[SZ][SZ], *B[SZ][SZ], *C[SZ][SZ];

    int *A_sub, *B_sub, *C_sub;

    // MPI initalisation and core functions
    int rank, name_len, tag=1; 
    char name[MPI_MAX_PROCESSOR_NAME];
    int res; 
    MPI_Status Stat;

    // Initialize the MPI environment
    MPI_Init(&argc,&argv);
    // Get the number of tasks/process
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    // Get the rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // Find the processor name
    MPI_Get_processor_name(name, &name_len);

    // Allocate memory for each matrix block
    A_sub = (int *) malloc(block_size * SZ * sizeof(int *));
    B_sub = (int *) malloc(block_size * SZ * sizeof(int *));
    C_sub = (int *) malloc(block_size * SZ * sizeof(int *));

    // allocate memory on Head/Master for the original vectors
    if (rank == 0) {
        A = (int *) malloc(SZ * SZ * sizeof(int *));
        B = (int *) malloc(SZ * SZ * sizeof(int *));
        C = (int *) malloc(SZ * SZ * sizeof(int *));
    }

    // random matrices A and B
    for (int i = 0; i < SZ; i++)    
        for (int j = 0; j < SZ; j++)
        {
        // Assign a random number to each position in the matrices
            A[i][j] = rand() % 100;
            B[i][j] = rand() % 100;
        }

    // Set the start time for the clock 
    auto start = high_resolution_clock::now();

    // scatter blocks of matrices
    MPI_Scatter(&A[0][0], block_size, MPI_INT, &A_sub, 0, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(&B[0][0], block_size, MPI_INT, &B_sub, 0, MPI_INT, 0, MPI_COMM_WORLD);

    // for (size_t i = 0; i < block_size; i++){
    //     v3_sub[i] += v1_sub[i] + v2_sub[i];
    // }

     for (size_t i = 0; i < block_size; i++){
        for (int j = 0; j < SZ; j++){
            C[i][j] = 0;
            for (int k = 0; k < SZ; k++){
                C[i][j] += A[i][k] * B [k][j];
            }
        }
    }

    MPI_Gather(MPI_IN_PLACE, block_size, MPI_INT, &C[0][0], block_size, MPI_INT, 0, MPI_COMM_WORLD);



    // std::thread code (old)
    // // create arrays for 1) thread array of std::threads, 2) array of threadArg objects
    // std::thread threads[num_threads];
    // ThreadArgs threadArgs[num_threads];
    
    // MULTITHREADING
    
    //create threads and perform matrix multiplication in blocks
    // for (int t = 0; t < num_threads; t++){
    //     // set the start and end points for the block for each created thread
    //     threadArgs[t] = {t *block_size, (t + 1) * block_size};
    //     // create a thread to run the multiplication function, passing in the ThreadArgs object for that block
    //     threads[t] = std::thread(matrixBlockMultiplication, &threadArgs[t]);
    // }

    // Wait for threads to complete
    // for (int t = 0; t < num_threads; t++){
    //     // if (threads[t].joinable()){
    //         threads[t].join();
    //     // }
    // }



    auto stop = high_resolution_clock::now();

    // Calculate the duration of the addition process based on (stop time) - (start time) = duration
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    // open file and write output to file
    ofstream outputFile;
    outputFile.open ("output.txt");


    //display matrices in terminal

    // cout << endl
    //      << "Matrix A" << endl;
    // for (int i = 0; i < N; i++){
    //     for (int j = 0; j < N; j++){
    //         int value = A[i][j];
    //         cout <<value<< " ";
    //     }
    //         cout << endl;      
    // }

    // cout << endl
    //      << "Matrix B" << endl;
    // for (int i = 0; i < N; i++){
    //     for (int j = 0; j < N; j++){
    //         int value = B[i][j];
    //         cout <<value<< " ";
    //     }
    //     cout << endl;
    // }       
    
    // cout << endl
    //      << "Matrix C" << endl;
    // for (int i = 0; i < N; i++){
    //     for (int j = 0; j < N; j++){
    //         int value = C[i][j];
    //         cout <<value<< " ";
    //     }
    //     cout << endl;              
    // }

    // write to file     
    for (int i = 0; i < SZ; i++){
        for (int j = 0; j < SZ; j++){
            int value = C[i][j];
            outputFile <<value<< " ";
        }
        outputFile << endl;      
    }
    
    outputFile.close();

    // Finalize the MPI environment
    MPI_Finalize();

    return 0;
}