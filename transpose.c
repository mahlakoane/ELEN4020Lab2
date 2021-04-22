#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

void display(int** A,int N);
void diagonalPthreads(int** A,int N,int Thread_Num);
void *parallel_func(void *ptr );

struct FuncArgs
{
   pthread_mutex_t mutex;  //For the purpose of serializing access to the common memory thread_id variable as multiple threads.
   pthread_cond_t idSet;
   int** A;
   int N;
   int Thread_Num;
   int id;
};
typedef struct FuncArgs ArgsStruct;


void main(int argc, char* argv[])
{
    int N = 128;  // smallest requested N value.
    int Thread_Num = 1;
    
    if(argc > 1)
    {
        Thread_Num = atoi(argv[1]);
        N = atoi(argv[2]);
    }
    


    int **A = (int **)malloc(N * sizeof(int *));
    for (int thread_id=0; thread_id<N; thread_id++)
    {
        A[thread_id] = (int *)malloc(N * sizeof(int));
    }

    int RandomNumberBound = N;

     srand(time(NULL));

     for(int a=0;a<N; a++)
     {
          for(int b=0;b<N;b++)
          {
             A[a][b] = rand()%(RandomNumberBound+1); // range includes the upper bound.
          }
     }
    
    display(A,N);
    printf("\n");

    diagonalPthreads(A,N,Thread_Num);
     
    display(A,N);
    printf("\n");

}

void display(int** A,int N)
{  
     for(int a=0;a<N; a++)
     {
         for(int b =0; b<N; b++)
         {
            printf("%d ", A[a][b]);
         }
         printf("\n");       
     }    
}


void diagonalPthreads(int** A,int N,int Thread_Num)
{
    pthread_t threads[Thread_Num];
    ArgsStruct Args;

    pthread_mutex_init (&Args.mutex , NULL);
    pthread_cond_init (&Args.idSet, NULL);
    Args.A = A;
    Args.N = N;
    Args.Thread_Num = Thread_Num;
    pthread_mutex_lock (&Args.mutex);
    
        for(int thread_id = 0; thread_id < Thread_Num; thread_id++) 
        {
            Args.id = thread_id;
            pthread_create( &threads[thread_id], NULL, parallel_func,&Args); // called NUM_THREADS times to make the required nuumber of threads.
            pthread_cond_wait (&Args.idSet, &Args.mutex); // signals the other threads to wait for the current thread.
            
        }
        for(int thread_id=0; thread_id< Thread_Num; thread_id++)
          {
               pthread_join( threads[thread_id], NULL );
          }

         
     pthread_mutex_destroy (&Args.mutex);
     pthread_cond_destroy (&Args.idSet);
}

void *parallel_func(void *ptr )
{
     ArgsStruct *Args = (struct FuncArgs*) ptr;

     pthread_mutex_lock(&(Args->mutex));
     int id = Args->id;  
     pthread_mutex_unlock(&(Args->mutex)); //unlock the other thread(s) after the correct id has been recorded.

     pthread_cond_signal (&(Args->idSet)); //signal to other threads that they can proceed.

     //printf("%d \n", id);  

        for(int row = id; row < (Args->N) ; row += Args->Thread_Num)
        {
            for(int col = row+1; col<Args->N ; col++)
            {
                int temp = Args->A[row][col];
                Args->A[row][col] = Args->A[col][row];
                Args->A[col][row] = temp;  
            }
        }
}