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
    
    // aloca o buffer
    int buffer_size = MPI_BSEND_OVERHEAD + sizeof(int);
    void* buffer = malloc(buffer_size);
    MPI_Buffer_attach(buffer, buffer_size);
    
    t_inicial = MPI_Wtime();
    inicio = 3 + meu_ranque*2;
    salto = num_procs*2;
    
    for (i = inicio; i <= n; i += salto) {    
        if(primo(i) == 1) cont++;
    }
        
    if(num_procs > 1) {
        if (meu_ranque != 0) {
            // processos não-raiz enviam suas contagens para o processo raiz com o bsend
            MPI_Bsend(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        } else {
            total = cont; 
            for (i = 1; i < num_procs; i++) {
                int other_process_count;
                MPI_Recv(&other_process_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                total += other_process_count;
            }
        }
    } else {
        total = cont;
    }
    
    // liberando o buffer
    MPI_Buffer_detach(&buffer, &buffer_size);
    free(buffer);
    
    t_final = MPI_Wtime();

    if (meu_ranque == 0) {
        total += 1;    /* Acrescenta o dois, que também é primo */
        printf("Quant. de primos entre 1 e n: %d \n", total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);     
    }
    MPI_Finalize();
    return(0);
}
