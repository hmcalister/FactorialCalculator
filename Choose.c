#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ULLI_MAX 18446744073709551615ULL

/**
 * Tests if a multiplying two unsigned long long ints a,b will cause overflow
 * @param a the unsigned long long int to check for overflow
 * @param b the unsigned long long int to check for overflow
 * @returns 0 if no overflow will ocurr, 1 otherwise
 */
int test_overflow(unsigned long long int *a, unsigned long long int *b){
    return ULLI_MAX / *a < *b;
}

/**
 * Returns the greatest common denominator (factor) of two unsigned long long ints.
 * Uses Euclids algorithm
 * @param a the first unsigned long long int to find the factor of
 * @param b the second unsigned long long int to find the factor of
 * @return the greatest common denominator of a,b
 */
unsigned long long int gcd(unsigned long long int a, unsigned long long int b){
    unsigned long long int temp;
    //Ensure that a > b
    if(a < b){
        temp = a;
        a = b;
        b = temp;
    }

    while(a > 0){
        temp = a;
        a = b % a;
        b = temp;
    }

    return b;
}

/**
 * Compute n choose k (n combination k, nCk) using 64 bit integers
 * @param n the size of the set to choose from
 * @param k the number of objects to choose
 * @param v verbose setting, 0 for false
 * @return The value of n choose k, or 0 if an error occurs, such as overflow
 */
unsigned long long int choose(unsigned long long int *n, unsigned long long int *k, int verbose){
    unsigned long long int *n_less_k, result, numerator, denominator;
    unsigned long long int temp;
    
    /* Note: to increase efficiency we will never calculate n! or k! on their own
    Instead we will calculate n!/k! and call this n_k_factorial
    We can then take n_k_factorial and divide by n_less_k_factorial, and still get choose
    This saves many CPU cycles, and avoids an entire overflow case
    See below for more details on how this is done*/
    
    /* Handle degenerate cases */
    //If n or k is 0, there is only 1 way to choose from this set
    if(*n==0 || *k==*n){
        return 1;
    }

    result = 1;

    /* Ensure that n > k > n_less_k */
    n_less_k = malloc(sizeof(n_less_k));
    *n_less_k = *n - *k;
    if(*k < *n_less_k){
        temp = *k;
        *k = *n_less_k;
        *n_less_k = temp;
    }

    if(verbose) printf("%llu \n", result);
    numerator = *n;
    for(denominator=1; denominator<=*n_less_k; denominator++){
        if(test_overflow(&result, &numerator)){
            unsigned long long int reduced_numerator, reduced_denominator, factor;
            if(verbose) printf("REDUCE: %llu, %llu\n", numerator, denominator);
            //Account for factor_1, between numerator and denominator
            factor = gcd(numerator, denominator);
            reduced_numerator = numerator/factor;
            reduced_denominator = denominator/factor;
            if(verbose) printf("\t%llu, %llu / %llu => %llu, %llu\n", numerator, denominator, factor, reduced_numerator, reduced_denominator);
            //Account for factor_2, between result and denominator
            factor = gcd(result, reduced_denominator);
            if(verbose) printf("\t%llu, %llu / %llu => %llu, %llu\n", result, denominator, factor, result/factor, reduced_denominator/factor);
            result = result/factor;
            reduced_denominator = reduced_denominator/factor;
            //Test for overflow again, now not possible to prevent
            if(test_overflow(&result, &reduced_numerator)){
                return 0;
            }
            result *= reduced_numerator;
            result /= reduced_denominator;
            if(verbose) printf("* %llu / %llu = %llu\n", numerator+1, denominator, result);
            //Don't forget to decrease the term we are one
            numerator--;
        } else {
            //No chance of overflow
            result*=numerator--;
            result/=denominator;
            if(verbose) printf("* %llu / %llu = %llu\n", numerator+1, denominator, result);
        }
    }

    /* Free the memory we have allocated */
    free(n_less_k);
    return result;
}

int main(int argc, char *argv[]){
    // Declare some variables for use in the program
    // Make these pointers for speed performance
    unsigned long long int *n = malloc(sizeof(n)), *k = malloc(sizeof(k)), temp;

    if(argc>=2 && !strcmp(argv[1], "-validate")){
        //This section is for testing data from a file, such as in test_data.txt
        //It reads 3 integers from each line, where the third integer is the expected result
        //
        unsigned long long int *result = malloc(sizeof(result));
        while(3==scanf("%llu %llu %llu", n, k, result)){
            printf("n = %llu, k = %llu: ", *n, *k);
            temp = choose(n, k, 0);
            if(temp==0){
                printf("OVERFLOW\n");
                //fprintf(stderr, "ERROR: nCk overflows a 64 bit integer\n");
            } else if(*result == temp) {
                printf("PASSED\n");
            } else {
                printf("FAILED\n\tFOUND: %llu\n\tEXPECTED: %llu\n\n", temp, *result);
            }
        }
        free(result);
    } else if(argc>=2 && !strcmp(argv[1], "-f")){
        //Check if data coming from file
        while(2==scanf("%llu %llu", n, k)){
            temp = choose(n, k, 0);
            if(temp==0){
                fprintf(stderr, "ERROR: nCk overflows a 64 bit integer\n");
            } else {
                printf("%llu\n\n", temp);
            }
        }
    } else {
        printf("Enter n, k (one line, space separated)\n");
        while(2==scanf("%llu %llu", n, k)){
            if(*n<*k){
                fprintf(stderr, "Cannot compute: n must be larger than k");
                continue;
            }
            temp = choose(n, k, 0);
            if(temp==0){
                fprintf(stderr, "ERROR: nCk overflows a 64 bit integer\n");
            } else {
                printf("%llu\n\n", temp);
            }
        }
    }

    free(n);
    free(k);
    return 0;
}