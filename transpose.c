#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

void display(int** A,int N);
void diagonalOMP(int** A,int N,int Thread_Num);

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

    diagonalOMP(A,N,Thread_Num);
     
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


void diagonalOMP(int** A,int N,int Thread_Num)
{
    omp_set_num_threads(Thread_Num);
     #pragma omp parallel
     {

        #pragma omp for     // Introduces the threads scheduling, and does not leave it to the system
         for(int row=0;row<N ; row++)
         {
            for(int col = row+1; col<N ; col++)
            {
               int temp = A[row][col];
               A[row][col] = A[col][row];
               A[col][row] = temp;  
            }
         }
     }
}