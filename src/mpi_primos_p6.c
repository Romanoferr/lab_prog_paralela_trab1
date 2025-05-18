#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

int primo (long int n) {
    int i;
       
    for (i = 3; i < (int)(sqrt(n) + 1); i+=2) {
        if(n%i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    double t_inicial, t_final;
    int cont = 0, total = 0;
    long int i, n;
    int meu_ranque, num_procs, inicio, salto;

    if (argc < 2) {
        printf("Valor inválido! Entre com um valor do maior inteiro\n");
        return 0;
    } else {
        n = strtol(argv[1], (char **) NULL, 10);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);    
    t_inicial = MPI_Wtime();
    inicio = 3 + meu_ranque*2;
    salto = num_procs*2;
    
    for (i = inicio; i <= n; i += salto) {    
        if(primo(i) == 1) cont++;
    }
        
    if(num_procs > 1) {
        if (meu_ranque != 0) {
            // Workers send their counts to master using standard send
            MPI_Send(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        } else {
            // Master receives from all workers using non-blocking receive
            total = cont;  // Initialize with master's count
            int *worker_counts = (int *)malloc((num_procs - 1) * sizeof(int));
            MPI_Request *requests = (MPI_Request *)malloc((num_procs - 1) * sizeof(MPI_Request));
            
            // Post all non-blocking receives
            for (i = 1; i < num_procs; i++) {
                MPI_Irecv(&worker_counts[i-1], 1, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[i-1]);
            }
            
            // Wait for all receives to complete
            MPI_Waitall(num_procs - 1, requests, MPI_STATUSES_IGNORE);
            
            // Sum up the results
            for (i = 0; i < num_procs - 1; i++) {
                total += worker_counts[i];
            }
            
            free(worker_counts);
            free(requests);
        }
    } else {
        total = cont;
    }
    
    t_final = MPI_Wtime();

    if (meu_ranque == 0) {
        total += 1;    /* Acrescenta o dois, que também é primo */
        printf("Quant. de primos entre 1 e n: %d \n", total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);     
    }
    MPI_Finalize();
    return(0);
} 