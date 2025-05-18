#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#define TAMANHO 500000

int primo (int n) {
    int i;
    for (i = 3; i < (int)(sqrt(n) + 1); i+=2) {
        if(n%i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    double t_inicial, t_final;
    int cont = 0, total = 0;
    int i, n;
    int meu_ranque, num_procs, inicio, dest, raiz=0, tag=1, stop=0;
    MPI_Status estado;
    MPI_Request recv_request;

    if (argc < 2) {
        printf("Entre com o valor do maior inteiro como parâmetro para o programa.\n");
        return 0;
    } else {
        n = strtol(argv[1], (char **) NULL, 10);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (num_procs < 2) {
        printf("Este programa deve ser executado com no mínimo dois processos.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return(1);
    }

    t_inicial = MPI_Wtime();

    if (meu_ranque == 0) { 
        int *worker_counts = (int *)malloc((num_procs - 1) * sizeof(int));
        MPI_Request *recv_requests = (MPI_Request *)malloc((num_procs - 1) * sizeof(MPI_Request));
        
        // Master process distributes initial work
        for (dest=1, inicio=3; dest < num_procs && inicio < n; dest++, inicio += TAMANHO) {
            // Post receive for this worker's result before sending work
            MPI_Irecv(&worker_counts[dest-1], 1, MPI_INT, dest, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_requests[dest-1]);
            MPI_Send(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }

        // Process results and send more work
        int active_workers = num_procs - 1;
        while (active_workers > 0) {
            // Wait for any worker to complete
            int index;
            MPI_Waitany(num_procs - 1, recv_requests, &index, &estado);
            total += worker_counts[index];
            dest = index + 1;  // Convert index back to rank
            
            if (inicio > n) {
                tag = 99;  // No more work
                active_workers--;
            } else {
                // Post new receive before sending more work
                MPI_Irecv(&worker_counts[index], 1, MPI_INT, dest, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_requests[index]);
            }
            
            MPI_Send(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            inicio += TAMANHO;
        }
        
        free(worker_counts);
        free(recv_requests);
    } else { 
        // Worker processes
        // Post initial receive
        MPI_Irecv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request);
        MPI_Wait(&recv_request, &estado);
        
        while (estado.MPI_TAG != 99) {
            // Process the work
            for (i = inicio, cont=0; i < (inicio + TAMANHO) && i < n; i+=2) 
                if (primo(i) == 1)
                    cont++;
            
            // Send result and post next receive
            MPI_Send(&cont, 1, MPI_INT, raiz, tag, MPI_COMM_WORLD);
            MPI_Irecv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request);
            MPI_Wait(&recv_request, &estado);
        } 
    }

    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1;    // Add 2 as prime
        printf("Quant. de primos entre 1 e %d: %d \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);     
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return(0);
} 