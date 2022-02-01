#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include </usr/include/semaphore.h>

#define BUFF_SIZE   5       /* tamanho do buffer */
#define NP          1       /* número de produtores*/
#define NCP1         5      /* número de consumidores/produtores  */
#define NCP2         4		  /* número de consumidores/produtores  */
#define NCP3         3		  /* número de consumidores/produtores  */
#define NC          1       /* número de consumidores */
#define M_SIZE      10       /* ordem da matriz */

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

void imprime_matriz(double **m){
  for(int i=0; i<M_SIZE; i++){
    for(int j=0; j<M_SIZE; j++)
      printf("%.2lf ", m[i][j]);
    printf("\n");
  }
}

void imprime_vetor(double *v){
  for(int i=0; i<M_SIZE; i++)
    printf("%.2lf ", v[i]);
  printf("\n");
}

void escreve_matriz(double **m, FILE *arq){
  for(int i=0; i<M_SIZE; i++){
    for(int j=0; j<M_SIZE; j++){
      fprintf(arq, "%.2lf ", m[i][j]);
    }
    fprintf(arq, "\n");
  }
}

void escreve_vetor(double *v, FILE *arq){
  for(int i=0; i<M_SIZE; i++)
    fprintf(arq, "%.2lf ", v[i]);
  fprintf(arq, "\n");
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

  // continua loop se conseguiu fazer a leitura da próxima linha
  while(fscanf(entrada, "%s", nome_arq) == 1){
    arq = fopen(nome_arq, "r");

    sem_wait(&shared[0].empty);
    sem_wait(&shared[0].mutex);

    /*
      Alocação de espaço na memória para as matrizes A, B, C
      para o vetor V e para o nome do arquivo que contém as matrizes
    */
    struct S *sP = (struct S*)malloc(sizeof(struct S));
    sP->a = (double **)malloc(sizeof(double *) * M_SIZE);
    sP->b = (double **)malloc(sizeof(double *) * M_SIZE);
    sP->c = (double **)malloc(sizeof(double *) * M_SIZE);
    sP->v = (double *)malloc(sizeof(double *)*M_SIZE);
    sP->nome = (char *)malloc(sizeof(char *)*25);

    for(int i=0; i<M_SIZE; i++){
      sP->a[i] = (double *)malloc(sizeof (double)*M_SIZE);
      sP->b[i] = (double *)malloc(sizeof (double)*M_SIZE);
      sP->c[i] = (double *)malloc(sizeof (double)*M_SIZE);
    }

    /* Cópia do contéudo do nome do arquivo para a estrutura compartilhada sP e
      inicialização do valor E
    */
    strcpy(sP->nome, nome_arq);
    sP->e = 0;

    // Leitura Matriz A
    for(int i=0; i<M_SIZE; i++){
      for(int j=0; j<M_SIZE; j++){
        fscanf(arq, "%lf ", &sP->a[i][j]);
      }
    }

    // Leitura Matriz B
    for(int i=0; i<M_SIZE; i++){
      for(int j=0; j<M_SIZE; j++){
        fscanf(arq, "%lf ", &sP->b[i][j]);
      }
    }

    printf("\nProdutor - Produzindo arq: %s\n", sP->nome);
    // imprime_matriz(sP->a);
    // imprime_matriz(sP->b);
    fflush(stdout);

    shared[0].buffer[shared[0].in] = sP;
    shared[0].in = (shared[0].in+1)%BUFF_SIZE;

    sem_post(&shared[0].mutex);
    sem_post(&shared[0].full);

    fclose(arq);
  }

  fclose(entrada);
}

void *CP1(void *arg){
  int id = (int)arg;

  // Porção de vezes que a thread deve executar
  int p = 50/NCP1;
  int r = 50%NCP1;
  int lim = (id+1)*p;

  if(id==NCP1-1)
    lim = lim+r;

  for(int i=id*p; i<lim; i++){
    // Consumindo elementos da região shared[0]
    sem_wait(&shared[0].full);
    sem_wait(&shared[0].mutex);

    struct S *sP;
    sP = shared[0].buffer[shared[0].out];
    shared[0].out = (shared[0].out+1)%BUFF_SIZE;

    printf("\nCP1 - Consumindo arq: %s\n", sP->nome);
    // imprime_matriz(sP->a);
    // imprime_matriz(sP->b);
    fflush(stdout);

    // Multiplicação das matrizes
    for(int i=0; i<M_SIZE; i++){
      for(int j=0; j<M_SIZE; j++){
        sP->c[i][j] = 0;
        for(int k=0; k<M_SIZE; k++)
          sP->c[i][j] += sP->a[i][k]*sP->b[k][j];
      }
    }

    printf("\nCP1 - Produzindo arq: %s\n", sP->nome);
    // imprime_matriz(sP->c);
    fflush(stdout);

    shared[2].buffer[shared[2].in] = sP;
    shared[2].in = (shared[2].in+1)%BUFF_SIZE;

    sem_post(&shared[0].mutex);
    sem_post(&shared[0].empty);

    // Produzindo elementos para a região shared[1]
    sem_wait(&shared[1].empty);
    sem_wait(&shared[1].mutex);

    shared[1].buffer[shared[1].in] = sP;
    shared[1].in = (shared[1].in+1)%BUFF_SIZE;

    sem_post(&shared[1].mutex);
    sem_post(&shared[1].full);
  }
}

void *CP2(void *arg){
  int id = (int)arg;

  // Porção de vezes que a thread deve executar
  int p = 50/NCP2;
  int r = 50%NCP2;
  int lim = (id+1)*p;

  if(id==NCP2-1)
    lim = lim+r;

  for(int i=id*p; i<lim; i++){
    // Consumindo elementos da região shared[1]
    sem_wait(&shared[1].full);
    sem_wait(&shared[1].mutex);

    struct S *sP;
    sP = shared[1].buffer[shared[1].out];
    shared[1].out = (shared[1].out+1)%BUFF_SIZE;

    printf("\nCP2 - Consumindo arq: %s\n", sP->nome);
    // imprime_matriz(sP->c);
    fflush(stdout);

    sem_post(&shared[1].mutex);
    sem_post(&shared[1].empty);

    // Produzindo elementos para a região shared[2]
    sem_wait(&shared[2].empty);
    sem_wait(&shared[2].mutex);

    // Soma das colunas da Matriz C
    for(int i=0; i<M_SIZE; i++){
      sP->v[i] = 0;
      for(int j=0; j<M_SIZE; j++){
        sP->v[i] += sP->c[j][i];
      }
    }

    printf("\nCP2 - Produzindo arq: %s\n", sP->nome);
    // imprime_vetor(sP->v);
    fflush(stdout);

    shared[2].buffer[shared[2].in] = sP;
    shared[2].in = (shared[2].in+1)%BUFF_SIZE;

    sem_post(&shared[2].mutex);
    sem_post(&shared[2].full);
  }
}

void *CP3(void *arg){
  int id = (int)arg;

  // Porção de vezes que a thread deve executar
  int p = 50/NCP3;
  int r = 50%NCP3;
  int lim = (id+1)*p;

  if(id==NCP3-1)
    lim = lim+r;

  for(int i=id*p; i<lim; i++){
    // Consumindo elementos da região shared[2]
    sem_wait(&shared[2].full);
    sem_wait(&shared[2].mutex);

    struct S *sP;
    sP = shared[2].buffer[shared[2].out];
    shared[2].out = (shared[2].out+1)%BUFF_SIZE;

    printf("\nCP3 - Consumindo arq: %s\n", sP->nome);
    fflush(stdout);

    sem_post(&shared[2].mutex);
    sem_post(&shared[2].empty);

    // Produzindo elementos para a região shared[3]
    sem_wait(&shared[3].empty);
    sem_wait(&shared[3].mutex);

    // Soma dos valores do vetor V armazenada em E
    for(int i=0; i<M_SIZE; i++)
      sP->e += sP->v[i];

    printf("\nCP3 - Produzindo arq: %s\n", sP->nome);
    fflush(stdout);

    shared[3].buffer[shared[3].in] = sP;
    shared[3].in = (shared[3].in+1)%BUFF_SIZE;

    sem_post(&shared[3].mutex);
    sem_post(&shared[3].full);
  }
}

void *Consumidor(void *arg){
  FILE *saida;
  saida = fopen("saida.out", "w");

  for(int i=0; i<50; i++){
    // Consumindo elementos da região shared[3]
    sem_wait(&shared[3].full);
    sem_wait(&shared[3].mutex);

    struct S *sP;
    sP = shared[3].buffer[shared[3].out];
    shared[3].out = (shared[3].out+1)%BUFF_SIZE;

    printf("\nConsumidor - Consumindo arq %s\n", sP->nome);
    // printf("%lf ", sP->e);
    // printf("\n###########################\n\n");
    fflush(stdout);

    // Construção do arquivo de saída
    fprintf(saida, "####### %s #######\n", sP->nome);
    fprintf(saida, "Matriz A\n");
    escreve_matriz(sP->a, saida);

    fprintf(saida, "\nMatriz B\n");
    escreve_matriz(sP->b, saida);

    fprintf(saida, "\nMatriz C\n");
    escreve_matriz(sP->c, saida);

    fprintf(saida, "\nVetor V\n");
    escreve_vetor(sP->v, saida);-

    fprintf(saida, "\nE\n");
    fprintf(saida, "%.2lf\n", sP->e);

    fprintf(saida, "\n");

    sem_post(&shared[3].mutex);
    sem_post(&shared[3].empty);
  }
  fclose(saida);
}

int main(int argc, char const *argv[]) {
  pthread_t idP, idC, idCP1[NCP1], idCP2[NCP2], idCP3[NCP3];

  for(int i=0; i<4; i++){
    sem_init(&shared[i].full, 0, 0);
    sem_init(&shared[i].empty, 0, BUFF_SIZE);
    sem_init(&shared[i].mutex, 0, 1);
  }

  // Criando instâncias do Produtor -  Recebe o nome do arquivo de entrada como argumento
  pthread_create(&idP, NULL, Produtor, (void *)argv[1]);

  // Criando instâncias do CP1
  for(int i=0; i<NCP1; i++){
    pthread_create(&idCP1[i], NULL, CP1, (void *)i);
  }

  // Criando instâncias do CP2
  for(int i=0; i<NCP2; i++){
    pthread_create(&idCP2[i], NULL, CP2, (void *)i);
  }

  // Criando instâncias da CP3
  for(int i=0; i<NCP3; i++){
    pthread_create(&idCP3[i], NULL, CP3, (void *)i);
  }

  // Criando instâncias do Consumidor
  pthread_create(&idC, NULL, Consumidor, 0);

  pthread_exit(NULL);

  return 0;
}
