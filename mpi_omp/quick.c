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
#define ARRAY_LEN   2048
#define MAX_VAL     1000000

unsigned array[ARRAY_LEN];
unsigned array_b[ARRAY_LEN];
unsigned array_c[ARRAY_LEN];


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
 *       @type   unsigned*
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
void quicksort(unsigned* array, unsigned len)
{
    unsigned pivot, temp;
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
    #pragma omp sections
    {
        #pragma omp section
        quicksort(array, i-1);
        #pragma omp section
        quicksort(array+i, len-i); 
    }
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
    int i, j, k, l, m;
    struct timeval start_time, stop_time, elapsed_time;
    double etime, flops;
    int id, p;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    unsigned samples[p];
    unsigned samples_collect[p*p];
    MPI_Request requests[p];
    MPI_Status send_status[p];
    MPI_Status recv_status[p];

    // Initialize the array to sort
    srand(time(NULL));
    for(i = 0; i < (ARRAY_LEN/p); i++) {
        array[i] = (unsigned)(MAX_VAL - (id*(ARRAY_LEN/p)) - i);
    }

    if(ARRAY_LEN < 32) {
        printf("Unsorted: ");
        for(i = 0; i < (ARRAY_LEN/p); i++) {
            printf("%d ", array[i]);
        }
        printf("\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    gettimeofday(&start_time, NULL);

    // Execute the algorithm
    quicksort(array, ARRAY_LEN/p);

    // Select the samples
    for(i = 0; i < p; i++) { 
        samples[i] = array[(ARRAY_LEN/p/p)*i];
    }

    // Send/receive the samples
    if(id == 0) {

        for(i = 0; i < p; i++) {
            samples_collect[i] = samples[i];   
        }
        for(i = 1; i < p; i++) {
            MPI_Status recv_status;
            MPI_Recv(&samples_collect[i*p], p, MPI_INT, i, 0, MPI_COMM_WORLD, &recv_status); 
        }

        // Sort the pivot values
        quicksort(samples_collect, p*p);

        // Select the samples
        for(i = 0; i < (p-1); i++) {
            samples[i] = samples_collect[p + p*i];
        }

        // Broadcast the pivot values
        MPI_Bcast(samples, p-1, MPI_INT, 0, MPI_COMM_WORLD);
    
    } else {
        // Send the samples to process 0
        MPI_Send(samples, p, MPI_INT, 0, 0, MPI_COMM_WORLD);
        
        // Wait for the pivot values
        MPI_Bcast(samples, p-1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    
    // Set a fake pivot point for the last element
    samples[p] = array[ARRAY_LEN/p*p - 1];

    // Divide the lists and send the elements to the other processes
    j=0;
    l = ARRAY_LEN/p;
    for(i = 0; i < p; i++) {
    
        // Reset the array
        memset(array_b, ARRAY_LEN*sizeof(unsigned), 0);

        // Skip send/receive from this device, incremente j
        if(id == i) {
            while(array[j++] <= samples[i]) {
                if(j >= ARRAY_LEN/p) {
                    break;
                }
            }
            continue;
        }

        // Figure out which values we need to send to this other process
        k = 0;
        while(1) {
            if((array[j] <= samples[i]) && j < (ARRAY_LEN/p)) {
                array_b[k++] = array[j];
                // Remove element j from the array
                // for(m = j; m < (l-2); m++) {
                //    array[m] = array[m+1];
                // }            
                // l--;
                j++;
            } else {
                j++;    
                break;
            }
        }
        
        MPI_Isend(array_b, ARRAY_LEN/p, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[i]);
    }

    // Receive the data from the other processes
    for(i = 0; i < p; i++) {
        if(i == id) {
            continue;
        }
        MPI_Recv(&array_c[i*(ARRAY_LEN/p)], ARRAY_LEN/p, MPI_INT, i, 0, MPI_COMM_WORLD, &recv_status[i]);
    }

    // Wait for the send functions to complete
    for(i = 0; i < p; i++) {
        if(i == id) {
            continue;
        }
        MPI_Wait(&requests[i], &send_status[i]);
    }

    // Merge the values together
    i = 0;
    j = 0;
    while((array_c[i] != 0) && (i < ARRAY_LEN/p)) {
        while(j < l) {
            if(array_c[i] < array[j] || j == l) {

                // Move the elements to the right one
                for(k = l; k > j; k++) {
                    array[k+1] = array[k];
                }
                l++;
            
                // Insert the element into the array
                array[j] = array_c[i];

                i++;
                break;
            }
            j++;
        }
        if(j == l) {
            // Move the elements to the right one
            for(k = l; k > j; k++) {
                array[k+1] = array[k];
            }
            l++;
            
            // Insert the element into the array
            array[j] = array_c[i];
            i++;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);


    gettimeofday(&stop_time, NULL);

    if(ARRAY_LEN < 32) {
        printf("Sorted  : ");
        for(i = 0; i < ARRAY_LEN/p; i++) {
            printf("%d ", array[i]);
        }
        printf("\n");
    }

    // Verify the results
    for(i = 0; i < (ARRAY_LEN/p-1); i++) {
        if(array[i] > array[i+1]) {
            printf("Error: i = %d\n", i);
            break;
            // return 1;
        }
    }

    timersub(&stop_time, &start_time, &elapsed_time);    
    etime = elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0;

    if(id == 0) {
        flops = ((double)ARRAY_LEN)/etime;
        printf("%d, %f, %f, %d\n", ARRAY_LEN, etime, flops, p);
    }
 
    MPI_Finalize();

    return 0;
}
