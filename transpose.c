#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>


struct FuncArgs
{
   pthread_mutex_t mutex;  //For the purpose of serializing access to the common memory thread_id variable as multiple threads.
   pthread_cond_t idSet;
   int** A;
   int N;
   int Untransposed;
   int Thread_Num;
   int block_dim;
   int id;
   int blocks;
   int total_num_blocks;
};

typedef struct FuncArgs ArgsStruct;

void display(int** A,int N);
void diagonalPthreads(int** A,int N,int Thread_Num);
void *parallel_func_diagonal(void *ptr );
void *parallel_func_submatrix(void *ptr );
void *parallel_func_block_swap(void *ptr );
void *parallel_func_leftOver(void *ptr);
void blockPthreads(int** A,int N,int block_dim,int Thread_Num);
void Transp_block_elems(ArgsStruct Args);
void Transp_blocks(ArgsStruct Args);
void Left_over(ArgsStruct Args);





void main(int argc, char* argv[])
{
    int N = 128;  // smallest requested N value.
    int Thread_Num = 1;
    int block_dim = 2;
    
    if(argc > 1)
    {
        Thread_Num = atoi(argv[1]);
        N = atoi(argv[2]);
        block_dim = atoi(argv[3]);
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

    //diagonalPthreads(A,N,Thread_Num);
    blockPthreads(A,N,block_dim,Thread_Num);
     
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
    ArgsStruct Args;
    pthread_t threads[Thread_Num];

    pthread_mutex_init (&Args.mutex , NULL);
    pthread_cond_init (&Args.idSet, NULL);
    Args.A = A;
    Args.N = N;
    Args.Thread_Num = Thread_Num;
    pthread_mutex_lock (&Args.mutex);
    
        for(int thread_id = 0; thread_id < Thread_Num; thread_id++) 
        {
            Args.id = thread_id;
            pthread_create( &threads[thread_id], NULL, parallel_func_diagonal,&Args); // called NUM_THREADS times to make the required nuumber of threads.
            pthread_cond_wait (&Args.idSet, &Args.mutex); // signals the other threads to wait for the current thread.
            
        }
        for(int thread_id=0; thread_id< Thread_Num; thread_id++)
          {
               pthread_join( threads[thread_id], NULL );
          }

         
     pthread_mutex_destroy (&Args.mutex);
     pthread_cond_destroy (&Args.idSet);
}

void *parallel_func_diagonal(void *ptr )
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

void blockPthreads(int** A,int N,int block_dim,int Thread_Num)
{
    int blocks = N/block_dim; // Tells us how many blocks of a specified dimension can fit into N.
    int total_num_blocks = blocks * blocks;

    ArgsStruct Args;

    pthread_mutex_init (&Args.mutex , NULL);
    pthread_cond_init (&Args.idSet, NULL);
    Args.A = A;
    Args.N = N;
    Args.Thread_Num = Thread_Num;
    Args.block_dim = block_dim;
    Args.blocks = blocks;
    Args.total_num_blocks = total_num_blocks;
    pthread_mutex_lock (&Args.mutex);


    Transp_block_elems(Args);
    Transp_blocks(Args);
    Left_over(Args);


    pthread_mutex_destroy (&Args.mutex);
    pthread_cond_destroy (&Args.idSet);

}

void Transp_block_elems(ArgsStruct Args)
{
    pthread_t threads[Args.Thread_Num];
        for(int thread_id = 0; thread_id < Args.Thread_Num; thread_id++) 
        {
            Args.id = thread_id;
            pthread_create( &threads[thread_id], NULL, parallel_func_submatrix,&Args); // called NUM_THREADS times to make the required nuumber of threads.
            pthread_cond_wait (&Args.idSet, &Args.mutex); // signals the other threads to wait for the current thread.
            
        }
        for(int thread_id=0; thread_id< Args.Thread_Num; thread_id++)
          {
               pthread_join( threads[thread_id], NULL );
          }
    
}

void *parallel_func_submatrix(void *ptr )
{
    ArgsStruct *Args = (struct FuncArgs*) ptr;

     pthread_mutex_lock(&(Args->mutex));
     int id = Args->id;  
     pthread_mutex_unlock(&(Args->mutex)); //unlock the other thread(s) after the correct id has been recorded.
     
     pthread_cond_signal (&(Args->idSet)); //signal to other threads that they can proceed.

    for(int block = id; block < Args->total_num_blocks; block += Args->Thread_Num)
        {
            int row_top = (block / Args->blocks) * Args->block_dim;
	        int col_top = (block % Args->blocks) * Args->block_dim;
	        int row_bot = row_top + Args->block_dim;
	        int col_bot = col_top + Args->block_dim;

            int col_offset =1;
            for(int row = row_top; row < row_bot ; row+=Args->block_dim)
            {   
                int offset =1;
                for(int col = col_top + offset; col < col_bot ; col++)
                {
                    int temp = Args->A[row][col];
                    Args->A[row][col] = Args->A[row + offset][col - offset];
                    Args->A[row + offset][col - offset] = temp; 
                    offset++; 
                }

                col_offset++;
            }


        }
}

void Transp_blocks(ArgsStruct Args)
{
        pthread_t threads[Args.Thread_Num];
        for(int thread_id = 0; thread_id < Args.Thread_Num; thread_id++) 
        {
            Args.id = thread_id;
            pthread_create( &threads[thread_id], NULL, parallel_func_block_swap,&Args); // called NUM_THREADS times to make the required nuumber of threads.
            pthread_cond_wait (&Args.idSet, &Args.mutex); // signals the other threads to wait for the current thread.
            
        }
        for(int thread_id=0; thread_id< Args.Thread_Num; thread_id++)
          {
               pthread_join( threads[thread_id], NULL );
          }
            
}

void *parallel_func_block_swap(void *ptr )
{
    ArgsStruct *Args = (struct FuncArgs*) ptr;

    pthread_mutex_lock(&(Args->mutex));
    int id = Args->id;  
    pthread_mutex_unlock(&(Args->mutex)); //unlock the other thread(s) after the correct id has been recorded.
    pthread_cond_signal (&(Args->idSet)); //signal to other threads that they can proceed.


    for(int xblocks = id ; xblocks < Args->blocks ; xblocks += Args->Thread_Num) 
        {    for(int yblocks = xblocks; yblocks < Args->blocks ; yblocks++)
            {
                // starting indices for the two blocks to be swapped 1-first block, 2-second block
                int row_top1 = xblocks * Args->block_dim;
			    int col_top1 = (yblocks % Args->blocks) * Args->block_dim;
			    int row_top2 = col_top1;
			    int col_top2 = row_top1;

                int row_bot1 = row_top1 + Args->block_dim;
			    int col_bot1 = col_top1 + Args->block_dim;

                // No need to define a specific bottom point for block 2 because the block dimension is the same,
                // Incrementing block 2's row and col gets to the end of block 2 even if not specified.
                for(int row1 = row_top1, row2 = row_top2; row1 < row_bot1; row1++, row2++)	
				    for(int col1 = col_top1, col2 = col_top2; col1 < col_bot1; col1++, col2++)
				    {
					    int temp = Args->A[row1][col1];
					    Args->A[row1][col1] = Args->A[row2][col2];
					    Args->A[row2][col2] = temp;
				    }	
            }
        }

}

void Left_over(ArgsStruct Args)
{
    if(Args.N % Args.block_dim != 0)
	{
		int unTransposed = (Args.N - (Args.N % Args.block_dim)); // N value from which the rest of the matrix is not transposed
        Args.Untransposed = unTransposed;

		pthread_t threads[Args.Thread_Num];
        for(int thread_id = 0; thread_id < Args.Thread_Num; thread_id++) 
        {
            Args.id = thread_id;
            pthread_create( &threads[thread_id], NULL, parallel_func_leftOver,&Args); // called NUM_THREADS times to make the required nuumber of threads.
            pthread_cond_wait (&Args.idSet, &Args.mutex); // signals the other threads to wait for the current thread.
            
        }
        for(int thread_id=0; thread_id< Args.Thread_Num; thread_id++)
          {
               pthread_join( threads[thread_id], NULL );
          }
		    
	    
    }
}

void *parallel_func_leftOver(void *ptr)
{
    ArgsStruct *Args = (struct FuncArgs*) ptr;

    pthread_mutex_lock(&(Args->mutex));
    int id = Args->id;  
    pthread_mutex_unlock(&(Args->mutex)); //unlock the other thread(s) after the correct id has been recorded.
    pthread_cond_signal (&(Args->idSet)); //signal to other threads that they can proceed.
    printf("%d\n",id);

    for(int row = Args->Untransposed+id; row < Args->N; row += Args->Thread_Num)  // id=0,1,2...
		for(int col = 0; col < row; col++)
		{
			int temp = Args->A[row][col];
			Args->A[row][col] = Args->A[col][row];
			Args->A[col][row] = temp;
		}
}
