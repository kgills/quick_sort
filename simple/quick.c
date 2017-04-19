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

/* Defines */
#define ARRAY_LEN   1024
#define MAX_VAL     1000000

unsigned array[ARRAY_LEN];


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
    unsigned pivot, temp, i, j;

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
    float sum;

    // Initialize the array to sort
    srand(time(NULL));
    for(i = 0; i < ARRAY_LEN; i++) {
        array[i] = rand() % MAX_VAL;
    }

    if(ARRAY_LEN < 32) {
        printf("Unsorted: ");
        for(i = 0; i < ARRAY_LEN; i++) {
            printf("%d ", array[i]);
        }
        printf("\n");
    }

    gettimeofday(&start_time, NULL);

    // Execute the algorithm
    quicksort(array, ARRAY_LEN);

    gettimeofday(&stop_time, NULL);

    if(ARRAY_LEN < 32) {
        printf("Sorted  : ");
        for(i = 0; i < ARRAY_LEN; i++) {
            printf("%d ", array[i]);
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

    printf("Array sorted\n");

    timersub(&stop_time, &start_time, &elapsed_time);    
    etime = elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0;


    flops = ((double)2 * (double)ARRAY_LEN * (double)ARRAY_LEN * (double)ARRAY_LEN)/etime;
    printf("%d, %f, %f, %d\n", ARRAY_LEN, etime, flops, 1);
 

    return 0;
}
