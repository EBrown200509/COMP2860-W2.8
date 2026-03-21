//
// Example of using pthreads to perform basic vector addition.
//
// Requires command line arguments for problem size N and number of threads p.
// Returns an error if N not divisible by p, or for any other error.
//
// Build as normal, e.g.
//
// > gcc -o vectorAddition vectorAddition.c
//
// and launch with the problem size and number of threads,
//
// > ./vectorAddition 10000 5
//


//
// Includes.
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>


//
// Struct for the arguments passed to the threads.
//
typedef struct {
    int start;
    int end;
    float *a;
    float *b;
    float *c; 
} args_t;


//
// The function called by each thread that performs part of the addition.
//
void* vecAdd_perThread( void *arguments )
{
    // Cast the (void*) arguments baxk to the struct.
    args_t *args = (args_t*) arguments;

    // Perform the loop over the range specified by the arguments.
    int i;
    for( i=args->start; i<args->end; i++ )
        args->c[i] = args->a[i] + args->b[i];

    return NULL;
}


//
// Main.
//
int main( int argc, char **argv )
{
 
    //
    // Parse command line arguments and check for validity.
    //
    if( argc!=3 )
    {
        printf( "Call with 2 command line arguments: N = problem size and p = no. threads.\n" );
        return EXIT_FAILURE;
    }

    // Parse problem size N.
    int N = atoi( argv[1] );
    if( N<1 )
    {
        printf( "Error: Problem size N must be a positive integer.\n" );
        return EXIT_FAILURE;
    }
    
    // Parse number of threads p.
    int p = atoi( argv[2] );
    if( p<1 )
    {
        printf( "Error: The number of threads must be a positive integer.\n" );
        return EXIT_FAILURE;
    }
    if( N%p )   // Recall the '%' operator is the remainder of integer division, so is non-zero if not a multiple.
    {
        printf( "Error: Problem size (N) must be a multiple of the number of threads (p) for this demonstration.\n" );
        return EXIT_FAILURE;
    }

    //
    // Set up the problem.
    //

    // Allocate memory.
    float
        *a = (float*) malloc( N*sizeof(float) ),
        *b = (float*) malloc( N*sizeof(float) ),
        *c = (float*) malloc( N*sizeof(float) );
    
    if( !a || !b || !c )
    {
        printf( "Error: Could not allocate memeory for the vector arrays.\n" );
        return EXIT_FAILURE;
    }

    // Initialise vectors a and b.
    for( int i=0; i<N; i++ )
        a[i] = b[i] = i;

    //
    // Perform the calculation in parallel using pthreads. Note that different argument structs are used for
    // each thread, as using the same struct runs the (small) risk of the struct fields being overwritten
    // before the thread is launched and reads them (recall we cannot control when threads actually start).
    // In practice the chances of this happening are very small, but they are not zero, so it would still 
    // be a mistake to re-use the same struct, even though it would likely pass even quite stringent tests.
    //
    pthread_t *ids = (pthread_t*) malloc( p*sizeof(pthread_t) );
    args_t *args = (args_t*) malloc( p*sizeof(args_t) );

    int NperThread = N / p;                 // Problem size per thread. Note we have already checked N is divisible by p.
    for( int i=0; i<p; i++ )
    {
        // Set arguments for the thread.
        args[i].start = i * NperThread;
        args[i].end = (i+1) * NperThread;
        args[i].a = a;
        args[i].b = b;
        args[i].c = c;

        // Create the thread from this main thread.
        pthread_create( &ids[i], NULL, vecAdd_perThread, &args[i] );
    }

    // Wait until all threads have completed.
    for( int i=0; i<p; i++ )
        pthread_join( ids[i], NULL );

    //
    // Serial check. Should work for any a[] and b[].
    //
    for( int i=0; i<N; i++ )
        if( fabs(a[i]+b[i]-c[i])>1e-3 )
        {
            printf( "Error: Vector addition failed for at least one element; first error found at i=%d.\n", i );
            return EXIT_FAILURE;
        }

    printf( "Vector addition performed correctly. Display first few elements:\n" );
    for( int i=0; i<(N>10?10:10); i++ )
        printf( "Element %d:\t%f + %f = %f\n", i, a[i], b[i], c[i] );

    //
    // Clean up and quit.
    //
    free( a );
    free( b );
    free( c );
    free( ids );
    free( args );

    return EXIT_SUCCESS;
}
