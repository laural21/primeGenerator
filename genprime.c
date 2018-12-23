/* Lab 2 - Laura Lehoczki
 *
 * This parallel program finds all prime numbers between 2 and N.
 *
 * To compile: gcc -g -Wall -fopenmp -o genprime genprimes.c
 * To run: ./genprime N numthreads
 *
 * Algorithm:
 * 1. Generate all numbers from 2 to N
 * 2. Going from left to right, take each number and remove all of its multiples.
 * 3. First, remove multiples of 2, then those of 3, next number is 5, because we'd removed 4... and so on.
 * 4. Continue until you reach the middle of the original array (floor((N+1)/2))
 * 5. The remaining numbers are primes
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <string.h>

int N; // Upper limit for prime numbers
int thread_count; // How many threads to work with
int *numbers; // Array of all numbers bw. 2-N
int *primes; // Array of the prime numbers in this interval
int *primes_local;

int main(int argc, char *argv[]) {
    double tstart = 0.0, ttaken;

    N = atoi(argv[1]);
    thread_count = atoi(argv[2]);
    numbers = (int *)malloc((N-1)* sizeof(int));

    /* Primes array is a copy of numbers for now. We'll mark the indexes of non-prime numbers
     * in this one, but the actual primes are going to be printed from numbers, from the non-
     * marked indices.
     */
    primes = (int *)malloc((N-1)* sizeof(int));
    int max = (N+1)/2;

    FILE * fp;
    char output[30] ="";

    // Generate numbers from 2 to N
    for (int k = 2; k <= N; k++){
        numbers[k-2] = k;
        primes[k-2] = k;
    }


    /*
     * To parallelize: give each thread a section of numbers, instead of a number to look for.
     * Analogous to the finding the number of ASCII characters in a file. - Performance can be improved
     * and critical sections can be eliminated in the same way.
     *
     * Make a local number array, every thread will replace composite numbers by 0's
     * Then collect all of the indexes across all threads that contain a non-0 number
     * For each index, use a && reduction. If any is 0, the number is not a prime.
     */

    tstart = omp_get_wtime();
    // Replace non-prime numbers by 0's
    // Manual reduction to each array index by copying the numbers array to a numbers_local array
    // and reducing each item in the arrays to its value.

    for (int i = 0; i < max; i++){
        if (numbers[i]!=0){
#pragma omp parallel for num_threads(thread_count)
            for (int j = i+1; j < N; j++){
                if(numbers[j]!=0){
                    if((numbers[j])%(numbers[i])==0){
                        if(primes[j]!=0){
                            //primes_local[j] = 0;
                            primes[j] = 0;
                            // This is not a critical section, since we only care if it's modified by at least
                            // one thread.
                            //primes[j] = primes[j] && primes_local[j];
                            //prime_count--;
                        }
                    }
                }
            }
        }
    }


    ttaken = omp_get_wtime() - tstart;
    printf("Time taken for the main part: %f\n", ttaken);
    /*
    printf("Primes after cancelling non-primes:\n");
    for (int i = 0; i < N-1; i++){
        printf("%d ", primes[i]);
    }
    printf("\n");
    //printf("prime count: %d\n", prime_count);


    // Test
    for (int i = 0; i < N; i++){
        printf("%d ", numbers[i]);
    }
    */

    /*
    primes = (int *)malloc(prime_count* sizeof(int));

    int prime_idx = 0;
    for (int i = 0; i <= N-2; i++){
        if(numbers[i]!=0){
            primes[prime_idx] = numbers[i];
            prime_idx++;
        }
    }
    */

    /*
    // Test
    for (int i = 0; i < prime_count; i++){
        printf("%d ", primes[i]);
    }
    printf("\n");
*/

    sprintf(output,"%d.txt",N);
    fp = fopen(output,"w");
    if(!fp)
    {
        printf("Cannot create the file %s\n", output);
        exit(1);
    }

    int prime_idx = 0;
    for(int p = 0; p < N; p++){
        // Iterate check the numbers from the primes array, but print from original numbers array
        if(primes[p]){
            if (p == 0){
                fprintf(fp,"%d, %d, %d\n", prime_idx+1, numbers[p], 0);
                prime_idx++;
            } else if (numbers[prime_idx]!=0){
                fprintf(fp,"%d, %d, %d\n", prime_idx+1, numbers[p], numbers[p]-numbers[p-(p-prime_idx)]);
                prime_idx++;
            }
        }
    }


    free(numbers);
    free(primes);
    fclose(fp);
    return 0;
}