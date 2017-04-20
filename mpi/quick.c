/** 
 * @file    quick.c
 * @author  Kevin Gillespie
 * @brief   quick sort is an algorithm to sort a list of elements. 
 *
 */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "mpi.h"

/* Defines */
#define ARRAY_LEN   1024
#define MAX_VAL     1000000

float array[ARRAY_LEN];
float array_b[ARRAY_LEN];


#define MIN(a,b) (((a)<(b))?(a):(b))
#define ABS(a) (((a)<0)?((-(a))):(a))

/* Globals */

/* Functions */


/**
 * @name     quick
 * @brief    recursive function for executing quick sort.
 * @param 
 *       @name   array
 *       @dir    I
 *       @type   float*
 *       @brief  Array  to be sorted.
 * @param 
 *       @name   len
 *       @dir    I
 *       @type   unsigned
 *       @brief  Length of the array to be sorted.
 *
 * @returns 0 for success, error status otherwise
 *
*/
void quicksort(float* array, unsigned len)
{
    float pivot, temp;
    unsigned  i, j;

    // Return if nothing to be sorted
    if(len <= 1) {
        return;
    }

    // Partition the array
    i = 0;
    j = len;
    pivot = array[0];
    while(1) {

        while((i < len) && (array[++i] < pivot)) {}
        while(array[--j] > pivot) {}

        if(i >= j) {
            break;
        }

        // Swap the elements at index i and j
        temp = array[i];
        array[i] = array[j]; 
        array[j] = temp;
    }

    // Swap the elements at index i-1 and 0
    temp = array[i-1];
    array[i-1] = array[0];
    array[0] = temp;

    // Sort the partitions
    quicksort(array, i-1);
    quicksort(array+i, len-i); 
}   
   

/**
 * @name     main
 * @brief    main function for quick.c
 * @param 
 *       @name   argc
 *       @dir    I
 *       @type   int 
 *       @brief  Number of arguments in argv.
 * @param 
 *       @name   argv
 *       @dir    I
 *       @type   char*[]
 *       @brief  Command line arguments.
 *
 * @returns 0 for success, error status otherwise
 *
 ******************************************************************************/
int main(int argc, char *argv[])
{
    int i;
    struct timeval start_time, stop_time, elapsed_time;
    double etime, flops;
    int id, p;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);


    float samples[p];
    float samples_collect[p*p];

    // Initialize the array to sort
    srand(time(NULL));
    for(i = 0; i < ARRAY_LEN; i++) {
        array[i] = (float)(MAX_VAL - (id*ARRAY_LEN) - i);
    }

    if(ARRAY_LEN < 32) {
        printf("Unsorted: ");
        for(i = 0; i < ARRAY_LEN; i++) {
            printf("%f ", array[i]);
        }
        printf("\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    gettimeofday(&start_time, NULL);

    // Execute the algorithm
    quicksort(array, ARRAY_LEN);

    // Select the samples
    for(i = 0; i < p; i++) { 
        samples[i] = array[(ARRAY_LEN/p)*i];
    }

    // Send/receive the samples
    if(id == 0) {

        for(i = 0; i < p; i++) {
            samples_collect[i] = samples[i];   
        }
        for(i = 1; i < p; i++) {
            MPI_Status recv_status;
            MPI_Recv(&samples_collect[i*p], p, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &recv_status); 
        }

        // Sort the pivot values
        quicksort(samples_collect, p*p);

        // Select the samples
        for(i = 0; i < (p-1); i++) {
            samples[i] = samples_collect[p + p*i];
        }

        // Broadcast the pivot values
        MPI_Bcast(samples, p-1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    } else {
        // Send the samples to process 0
        MPI_Send(samples, p, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
        
        // Wait for the pivot values
        MPI_Bcast(samples, p-1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }

    // Divide the lists and send the elements to the other processes
    


    MPI_Barrier(MPI_COMM_WORLD);


    gettimeofday(&stop_time, NULL);

    if(ARRAY_LEN < 32) {
        printf("Sorted  : ");
        for(i = 0; i < ARRAY_LEN; i++) {
            printf("%f ", array[i]);
        }
        printf("\n");
    }

    // Verify the results
    for(i = 0; i < (ARRAY_LEN-1); i++) {
        if(array[i] > array[i+1]) {
            printf("Error: i = %d\n", i);
            return 1;
        }
    }

    timersub(&stop_time, &start_time, &elapsed_time);    
    etime = elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0;

    if(id == 0) {
        flops = ((double)2 * (double)ARRAY_LEN * (double)ARRAY_LEN * (double)ARRAY_LEN)/etime;
        printf("%d, %f, %f, %d\n", ARRAY_LEN, etime, flops, 1);
    }
 
    MPI_Finalize();

    return 0;
}