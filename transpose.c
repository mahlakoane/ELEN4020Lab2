#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

void display(int** A,int N);
void diagonalOMP(int** A,int N,int Thread_Num);
void blockOMP(int** A,int N,int block_dim,int Thread_Num);
void Transp_block_elems(int** A, int N, int block_dim, int Thread_Num, int blocks, int total_num_blocks);
void Transp_blocks(int** A,int block_dim, int Thread_Num, int blocks, int total_num_blocks);
void Left_over(int** A,int N,int Thread_Num,int block_dim);


void main(int argc, char* argv[])
{
    int N = 128;  // smallest requested N value.
    int Thread_Num = 1;
    int block_dim =2;
    
    if(argc > 1)
    {
        Thread_Num = atoi(argv[1]);
        N = atoi(argv[2]);
        block_dim = atoi(argv[3]);
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
    
    //display(A,N);
    printf("\n");

    diagonalOMP(A,N,Thread_Num);
    //blockOMP(A,N,2,Thread_Num); 
     
    //display(A,N);
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

void blockOMP(int** A,int N,int block_dim,int Thread_Num)
{
    int blocks = N/block_dim; // Tells us how many blocks of a specified dimension can fit into N.
    int total_num_blocks = blocks * blocks;
    Transp_block_elems(A,N,block_dim,Thread_Num,blocks,total_num_blocks);
    Transp_blocks(A,block_dim,Thread_Num,blocks,total_num_blocks);
    Left_over(A,N,Thread_Num,block_dim);
}

void Transp_block_elems(int** A, int N, int block_dim, int Thread_Num, int blocks, int total_num_blocks)
{
    omp_set_num_threads(Thread_Num);
    #pragma omp parallel
    {
        #pragma omp for
        for(int block = 0; block < total_num_blocks; block++)
        {
            int row_top = (block / blocks) * block_dim;
	        int col_top = (block % blocks) * block_dim;
	        int row_bot = row_top + block_dim;
	        int col_bot = col_top + block_dim;

            int col_offset =1;
            for(int row = row_top; row < row_bot ; row+=block_dim)
            {   
                int offset =1;
                for(int col = col_top + offset; col < col_bot ; col++)
                {
                    int temp = A[row][col];
                    A[row][col] = A[row + offset][col - offset];
                    A[row + offset][col - offset] = temp; 
                    offset++; 
                }
            
                col_offset++;
            }
        
    
        }
    } // threads are released here, back to serial code
}

// Swap two blocks by swapping each corresponding element index.
void Transp_blocks(int** A,int block_dim, int Thread_Num, int blocks, int total_num_blocks)
{
    omp_set_num_threads(Thread_Num);
    #pragma omp parallel
    {
        #pragma omp for
        for(int xblocks =0 ; xblocks < blocks ; xblocks++)  //Nested for loop because blocks are also formed on a two dimensional Arix.
        {    for(int yblocks = xblocks; yblocks < blocks ; yblocks++)
            {
                // starting indices for the two blocks to be swapped 1-first block, 2-second block
                int row_top1 = xblocks * block_dim;
			    int col_top1 = (yblocks % blocks) * block_dim;
			    int row_top2 = col_top1;
			    int col_top2 = row_top1;

                int row_bot1 = row_top1 + block_dim;
			    int col_bot1 = col_top1 + block_dim;

                // No need to define a specific bottom point for block 2 because the block dimension is the same,
                // Incrementing block 2's row and col gets to the end of block 2 even if not specified.
                for(int row1 = row_top1, row2 = row_top2; row1 < row_bot1; row1++, row2++)	
				    for(int col1 = col_top1, col2 = col_top2; col1 < col_bot1; col1++, col2++)
				    {
					    int temp = A[row1][col1];
					    A[row1][col1] = A[row2][col2];
					    A[row2][col2] = temp;
				    }	
            }
        }    
   }
}

void Left_over(int** A,int N,int Thread_Num,int block_dim)
{
    if(N % block_dim != 0)
	{
		int unTransposed = (N - (N % block_dim)); // N value from which the rest of the matrix is not transposed
		omp_set_num_threads(Thread_Num);

        #pragma omp parallel
        {
		    #pragma omp for
		    for(int row = unTransposed; row < N; row++)
			    for(int col = 0; col < row; col++)
			    {
				    int temp = A[row][col];
				    A[row][col] = A[col][row];
				    A[col][row] = temp;
			    }
	    }
    }
}

