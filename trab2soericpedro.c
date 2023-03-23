/*

INF1316 - SISTEMAS OPERACIONAIS - 2022.2 - 3WA
Tarefa 2 - Threads, tempo sequencial e tempo paralelo, comparação com processos
Nome: Eric Leão Matrícula: 2110694
Nome: Pedro Machado Peçanha Matrícula: 2110535

*/

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define TAM 1000
#define BLOCO 125
#define VAR1 1
#define VAR2 2

int arrt1[TAM], arrt2[TAM], arrt3[TAM];

void *somaVetor(void *a) {
  for (long i = (long)a * BLOCO; i < (long)a * BLOCO + BLOCO; i += 1) {
    arrt3[i] = arrt1[i] + arrt2[i];
  }
  pthread_exit(NULL); /*not necessary*/
}

void *fakethread(void *a) { pthread_exit(NULL); /*not necessary*/ }

int main(void) {
  int segmento1, segmento2, segmento3, *arr1, *arr2, *arr3, status, id;
  struct timeval inicio, fim;
  double total;
  segmento1 = shmget(IPC_PRIVATE, TAM * sizeof(int),
                     IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  segmento2 = shmget(IPC_PRIVATE, TAM * sizeof(int),
                     IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  segmento3 = shmget(IPC_PRIVATE, TAM * sizeof(int),
                     IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

  arr1 = (int *)shmat(segmento1, 0, 0);
  arr2 = (int *)shmat(segmento2, 0, 0);
  arr3 = (int *)shmat(segmento3, 0, 0);
  long a;
  for (a = 0; a < TAM; a++) {
    arr1[a] = VAR1;
    arr2[a] = VAR2;
    arr3[a] = 0;
  }
  gettimeofday(&inicio, NULL);
  for (a = 0; a < TAM / BLOCO; a++) {
    if ((id = fork()) < 0) {
      puts("Erro na criação do novo processo");
      exit(-2);
    } else if (!id) {
      for (int i = a * BLOCO; i < a * BLOCO + BLOCO; i += 1) {
        arr3[i] = arr1[i] + arr2[i];
      }
      shmdt(arr1);
      shmdt(arr2);
      shmdt(arr3);
      exit(0);
    }
  }
  for (int i = 0; i < TAM / BLOCO; i++)
    wait(&status);
  shmctl(segmento1, IPC_RMID, 0);
  shmctl(segmento2, IPC_RMID, 0);
  shmctl(segmento3, IPC_RMID, 0);
  gettimeofday(&fim, NULL);
  total = (fim.tv_sec - inicio.tv_sec) * 1000.0f +
          (fim.tv_usec - inicio.tv_usec) / 1000.0f;
  int maxError = 0;
  int diffError = 0;
  for (a = 0; a < TAM; a++)
    maxError =
        (maxError > (diffError = fabs((double)(arr3[a] - (VAR1 + VAR2)))))
            ? maxError
            : diffError;
  printf("erros com fork = %d, tendo durado %f ms\n", maxError, total);

  pthread_t threads[TAM / BLOCO];
  for (a = 0; a < TAM; a++) {
    arrt1[a] = VAR1;
    arrt2[a] = VAR2;
    arrt3[a] = 0;
  }
  gettimeofday(&inicio, NULL);
  for (a = 0; a < TAM / BLOCO; a++) {
    pthread_create(&threads[a], NULL, somaVetor, (void *)a);
  }
  for (long i = 0; i < TAM / BLOCO; i++)
    pthread_join(threads[i], NULL);
  gettimeofday(&fim, NULL);
  total = (fim.tv_sec - inicio.tv_sec) * 1000.0f +
          (fim.tv_usec - inicio.tv_usec) / 1000.0f;
  maxError = 0;
  diffError = 0;
  for (a = 0; a < TAM; a++)
    maxError =
        (maxError > (diffError = fabs((double)(arr3[a] - (VAR1 + VAR2)))))
            ? maxError
            : diffError;
  printf("erros com thread = %d, tendo durado %f ms\n", maxError, total);
  for (a = 0; a < TAM; a++)
    arr3[a] = arr2[a] + arr1[a];
  gettimeofday(&fim, NULL);
  /*
  for (a = 0; a < TAM / BLOCO; a++) {
    pthread_create(&threads[a], NULL, fakethread, NULL);
  }
  for (long i = 0; i < TAM / BLOCO; i++)
    pthread_join(threads[i], NULL);
*/
  total = (fim.tv_sec - inicio.tv_sec) * 1000.0f +
          (fim.tv_usec - inicio.tv_usec) / 1000.0f;
  maxError = 0;
  diffError = 0;
  for (a = 0; a < TAM; a++)
    maxError =
        (maxError > (diffError = fabs((double)(arr3[a] - (VAR1 + VAR2)))))
            ? maxError
            : diffError;
  printf("erros sequencial = %d, tendo durado %f ms\n", maxError, total);
  return 0;
}