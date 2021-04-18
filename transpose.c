#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

void display(int** A,int N);

void main(int argc, char* argv[])
{
    int N = 128;  // smallest requetsed value.
    int Thread_Num = 1;
    
    if(argc > 1)
    {
        Thread_Num = atoi(argv[1]);
        N = atoi(argv[2]);
    }
    


    int **A = (int **)malloc(N * sizeof(int *));
    for (int i=0; i<N; i++)
    {
        A[i] = (int *)malloc(N * sizeof(int));
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

    //omp_set_num_threads(Thread_Num);

     //#pragma omp parallel
     //{
         //int thread_id = omp_get_thread_num();
         //int numThreads = omp_get_num_threads();

         //#pragma omp parallel for
         for(int row=0;row<N ; row++)
         {
            for(int col = row; col<N ; col++)
            {
               int temp = A[row][col];
               A[row][col] = A[col][row];
               A[col][row] = temp;  
    
            }
    
         }
         
     //}
     
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
