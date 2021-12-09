/*
** UC: 21111 - Sistemas Operativos
** e-fólio B 2019-20 (mtex.c)
**
** Aluno: 1701898 - Diogo Antão
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>

#define NT_MIN       1    /* numero minimo de tarefas Ty */
#define NT_MAX      99    /* numero maximo de tarefas Ty */
#define N_MAX      999    /* quantidade maxima de valores gerados pela tarefa Tx */
#define DIMBUF_MIN   1    /* dimensao minima do buffer */
#define KMAX_MIN     1    /* dimensao minima de kmax */
#define RANGE_MIN   -1    /* alcance minimo dos valores gerados pela tarefa Tx */
#define RANGE_MAX    1    /* alcance maximo dos valores gerados pela tarefa Tx */
#define EMPTY       -2    /* valor de referencia para testar se um elemento de um array esta vazio ou nao */

/* declaracao de variaveis globais */
int nt, n, dimbuf, kmax, total_count_n=0;
float buffer_x[N_MAX], buffer_y[N_MAX];
pthread_mutex_t mtx;

/* tipo do argumento da(s) tarefa(s) Ty */
typedef struct{
  int i;
  int count;
} trf_arg_t;

/* prototipos */
void *Tx();          /* funcao da tarefa que gera n numeros reais */
void *Ty(void *arg); /* funcao da tarefa que calcula as exponenciais */
void *Tp();          /* funcao da tarefa que imprime os resultados */

int main(int argc, char *argv[]){
  int i, flag, r;

  /* variaveis para ID das tarefas */
  pthread_t trf_tx_id, trf_ty_id[NT_MAX], trf_tp_id;
  /* variavel para atributos das tarefas */
  pthread_attr_t trf_atr;
  /* variavel para argumento da(s) tarefa(s) Ty */
  trf_arg_t ty_arg[NT_MAX];

  /* verificar validade dos argumentos */
  flag = 1;
  if( argc != 5 )
    flag = 0;
  else{
    nt = atoi(argv[1]);
    n = atoi(argv[2]);
    dimbuf = atoi(argv[3]);
    kmax = atoi(argv[4]);
    if( !(nt>=NT_MIN && nt<=NT_MAX && n>=nt && n<=N_MAX && dimbuf>=DIMBUF_MIN && dimbuf<=n && kmax>=KMAX_MIN && kmax<= dimbuf) )
      flag = 0;
  }
  if ( !flag ){
    printf("\nUtilizacao: ./mtex nt n dimbuf kmax\n");
    printf("**         %d <= nt <= %d        **\n", NT_MIN, NT_MAX);
    printf("**        nt <= n <= %d        **\n", N_MAX);
    printf("**       %d <= dimbuf <= n        **\n", DIMBUF_MIN);
    printf("**      %d <= kmax <= dimbuf      **\n", KMAX_MIN);
    printf("***********************************\n\n");
    exit(1);
  }

  /* mensagem inicial */
  printf("Calculo de %d valor(es) de e^x com %d tarefa(s), dimbuf=%d e kmax=%d\n", n, nt, dimbuf, kmax);

  /* inicializar buffers */
  for(i=0; i<dimbuf; i++){
    buffer_x[i] = EMPTY;
    buffer_y[i] = EMPTY;
  }

  /* inicializar variavel de atributos com valores por defeito */
  pthread_attr_init(&trf_atr);

  /* modificar estado de desacoplamento para joinable */
  pthread_attr_setdetachstate(&trf_atr, PTHREAD_CREATE_JOINABLE);

  /* inicializar mutex com valores por defeito para os atributos */
  pthread_mutex_init(&mtx, NULL);

  /* criar e iniciar execução da tarefa Tx */
  r = pthread_create(&trf_tx_id, &trf_atr, Tx, NULL);
  if( r ){
    /* erro */
    printf("Ocorreu um erro na criacao da tarefa!\n");
    exit(1);
  }

  /* inicializar estruturas de dados que servem de argumento da(s) tarefa(s) Ty */
  for(i=0; i<nt; i++){
    /* numero de ordem de cada tarefa Ty */
    ty_arg[i].i = i;

    ty_arg[i].count = 0;

    /* criar e iniciar execução da(s) tarefa(s) Ty */
    r = pthread_create(&trf_ty_id[i], &trf_atr, Ty, (void*) &ty_arg[i]);
    if( r ){
      /* erro */
      printf("Ocorreu um erro na criacao da tarefa!\n");
      exit(1);
    }
  }

  /* criar e iniciar execução da tarefa Tp */
  r = pthread_create(&trf_tp_id, &trf_atr, Tp, NULL);
  if( r ){
    /* erro */
    printf("Ocorreu um erro na criacao da tarefa!\n");
    exit(1);
  }

  /* esperar que a tarefa Tx termine */
  pthread_join(trf_tx_id, (void **) NULL);

  /* esperar que a(s) tarefa(s) Ty termine(m) */
  for(i=0; i<nt; i++){
    pthread_join(trf_ty_id[i], (void **) NULL);
  }

  /* esperar que a tarefa Tp termine */
  pthread_join(trf_tp_id, (void **) NULL);

  /* libertar recursos associados ao mutex */
  pthread_mutex_destroy(&mtx);

  /* imprimir relatorio */
  for(i=0; i<nt; i++){
    printf("A tarefa Ty[%d] calculou %d valor(es)\n", ty_arg[i].i, ty_arg[i].count);
  }
  
  return 0;
}

void *Tx(){
  int i, tx_check_n=0;
  float reais[n];

  /*inicializar semente */
  srand(223);

  /* gerar valores reais */
  for(i=0; i<n; i++){
    reais[i] = RANGE_MIN + ( (float)rand() / (float)RAND_MAX ) * (RANGE_MAX-RANGE_MIN);
  }

  while( tx_check_n < n ){
    /* aceder ao buffer_x */
    pthread_mutex_lock(&mtx);
    /* colocar valores no buffer_x */
    for(i=0; i<dimbuf; i++){
      if( tx_check_n==n )
        break;
      if( buffer_x[i]==EMPTY ){
        buffer_x[i] = reais[tx_check_n];
        tx_check_n++;
      }
    }
    pthread_mutex_unlock(&mtx);
    sched_yield();
  }

  return (void*) NULL;
}

void *Ty(void *arg){
  int i, k;
  float vetor[kmax];
  trf_arg_t *y;

  /* converter (void *) para o tipo certo */
  y = (trf_arg_t *) arg;

  /* inicializar vetor[] */
  for(i=0; i<kmax; i++){
    vetor[i] = EMPTY;
  }

  while( total_count_n<n ){

    /* aceder ao buffer_x */
    pthread_mutex_lock(&mtx);
    for(k=0; k<kmax; k++){
      for(i=0; i<dimbuf; i++){
        /* remover valores do buffer_x */
        if( buffer_x[i]!=EMPTY && vetor[k]==EMPTY ){
          vetor[k] = buffer_x[i];
          buffer_x[i] = EMPTY;
        }
      }
    }
    pthread_mutex_unlock(&mtx);

    /* calcular exponencial */
    for(i=0; i<kmax; i++){
      if( vetor[i]!=EMPTY && vetor[i]>=-1 && vetor[i]<=1 ){
        vetor[i] = expf(vetor[i]);
      }
    }

    /* aceder ao buffer_y */
    pthread_mutex_lock(&mtx);
    for(k=0; k<kmax; k++){
      for(i=0; i<dimbuf; i++){
        /* colocar valores no buffer_y */
        if( buffer_y[i]==EMPTY && vetor[k]!=EMPTY ){
          buffer_y[i] = vetor[k];
          vetor[k] = EMPTY;
          total_count_n++;
          y->count++;
        }
      }
    }
    pthread_mutex_unlock(&mtx);
    sched_yield();
  }

  return (void*) NULL;
}

void *Tp(){
  int i, tp_check_n=0;

  while( tp_check_n < n ){
    /* aceder ao buffer_y */
    pthread_mutex_lock(&mtx);
    for(i=0; i<dimbuf; i++){
      if( tp_check_n==n )
        break;
      /* imprimir resultados e esvaziar buffer_y */
      if( buffer_y[i]!=EMPTY ){
        printf("%.5f\n", buffer_y[i]);
        buffer_y[i] = EMPTY;
        tp_check_n++;
      }
    }
    pthread_mutex_unlock(&mtx);
    sched_yield();
  }

  return (void*) NULL;
}