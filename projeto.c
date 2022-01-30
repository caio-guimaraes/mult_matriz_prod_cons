#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include </usr/include/semaphore.h>

#define BUFF_SIZE   5		/* número total de arquivos abertos por vez */
#define NP          1		/* número de produtores*/
#define NCP         3		/* número de consumidores/produtores */
#define NC          1		/* número de consumidores */

typedef struct S{
  char *nome;
	double **a, **b, **c;
	double *v;
  double e;
}S;

typedef struct {
  int in;
  int out;
  S *buffer[BUFF_SIZE];
  sem_t full;
  sem_t empty;
  sem_t mutex;
} pc_buf;

pc_buf shared[4]; // cria 4 regiões de memória compartilhadas

void imprime_matriz(double **m1){
  for(int i=0; i<2; i++){
    for(int j=0; j<2; j++){
      printf("%.2lf ", m1[i][j]);
    }
    printf("\n");
  }
}

void *Produtor(void *arg){
  FILE *entrada, *arq;
  char *nome_entrada = (char *)arg;

  entrada = fopen(nome_entrada, "r");

  char nome_arq[25];

  if (!entrada){
    printf("Erro! Não foi possível abrir o arquivo\n");
    exit(-1);
  }

  while(fscanf(entrada, "%s", nome_arq) == 1){ // se conseguiu fazer a leitura da próxima linha
    arq = fopen(nome_arq, "r");
    // printf("Arquivo: %s\n", nome_arq);

    sem_wait(&shared[0].empty);
    sem_wait(&shared[0].mutex);

    struct S *sP = (struct S*)malloc(sizeof(struct S));
    sP->a = (double **)malloc(sizeof(double *) * 2);
    sP->b = (double **)malloc(sizeof(double *) * 2);
    sP->c = (double **)malloc(sizeof(double *) * 2);

    for(int i=0; i<2; i++){
      sP->a[i] = (double *)malloc(sizeof (double)*2);
      sP->b[i] = (double *)malloc(sizeof (double)*2);
      sP->c[i] = (double *)malloc(sizeof (double)*2);
    }

    sP->nome = nome_arq;

    for(int i=0; i<2; i++){
      for(int j=0; j<2; j++){
        fscanf(arq, "%lf ", &sP->a[i][j]);
      }
    }

    // imprime_matriz(sP->a);
    // printf("\n");
    // fflush(stdout);

    for(int i=0; i<2; i++){
      for(int j=0; j<2; j++){
        fscanf(arq, "%lf ", &sP->b[i][j]);
      }
    }

    // imprime_matriz(sP->b);
    // printf("\n###########################\n\n");
    // fflush(stdout);

    shared[0].buffer[shared[0].in] = sP;
    shared[0].in = (shared[0].in+1)%BUFF_SIZE;

    sem_post(&shared[0].mutex);
    sem_post(&shared[0].full);

    fclose(arq);
  }

  fclose(entrada);
}

void *CP1(void *arg){
  // Consumindo a região shared[0]
  sem_wait(&shared[0].full);
  sem_wait(&shared[0].mutex);

  struct S *sP;
  sP = shared[0].buffer[shared[0].out];
  shared[0].out = (shared[0].out+1)%BUFF_SIZE;

  printf("\nMatriz A\n");
  imprime_matriz(sP->a);
  // printf("\n###########################\n\n");
  fflush(stdout);
  printf("\nMatriz B\n");
  imprime_matriz(sP->b);
  // printf("\n###########################\n\n");
  fflush(stdout);

  // Multiplicação das matrizes
  for(int i=0; i<2; i++){
    for(int j=0; j<2; j++){
      sP->c[i][j] = 0;
      for(int k=0; k<2; k++)
        sP->c[i][j] += sP->a[i][k]*sP->b[k][j];
    }
  }
  printf("\nMatriz C\n");
  imprime_matriz(sP->c);
  printf("\n###########################\n\n");
  fflush(stdout);

  sem_post(&shared[0].mutex);
  sem_post(&shared[0].empty);

  // Produzindo para a região shared[1]
  sem_wait(&shared[1].empty);
  sem_wait(&shared[1].mutex);

  sem_post(&shared[1].mutex);
  sem_post(&shared[1].full);

}

void *CP2(void *arg){

}

void *CP3(void *arg){

}

void *Consumidor(void *arg){

}

int main(int argc, char const *argv[]) {

  pthread_t idP, idC, idCP1[5], idCP2[4], idCP3[3];

  for(int i=0; i<4; i++){
    sem_init(&shared[i].full, 0, 0);
    sem_init(&shared[i].empty, 0, BUFF_SIZE);
    sem_init(&shared[i].mutex, 0, 1);
  }

  pthread_create(&idP, NULL, Produtor, (void *)argv[1]);

  for(int i=0; i<5; i++){
    pthread_create(&idCP1[i], NULL, CP1, (void *)i);
  }

  // pthread_join(idC, NULL);
  pthread_exit(NULL);

  return 0;
}
