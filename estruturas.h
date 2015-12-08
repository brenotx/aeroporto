#ifndef EST_H
#define EST_H
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define ESTADO_VOANDO 0
#define ESTADO_QUEBRADO 1
#define ESTADO_GARAGEM 2
#define ESTADO_DECOLANDO 3
#define ESTADO_POUSANDO 4
#define ESTADO_ABASTECENDO 5
#define ESTADO_DESEMBARQUE 6
#define ESTADO_EMBARQUE 7
#define ESTADO_POUSO_FORCADO 8
#define QTD_AVIAO 100
#define QTD_AEROPORTOS 2
#define MAX_COMBUSTIVEL 100
#define CAPACIDADE_AEROPORTO 20
#define CAPACIDADE_SOLO 10
#define TEMPO_VIAGEM_OFFSET 10
#define TEMPO_VIAGEM_BASE 20
#define TEMPO_ABASTECER 5
#define TEMPO_PREPARAR 8
#define TEMPO_DECOLAR 2
#define TEMPO_POUSAR 2
#define TEMPO_DESEMBARQUE_OFFSET 2
#define TEMPO_DESEMBARQUE_BASE 2
#define TEMPO_EMBARQUE_OFFSET 3
#define TEMPO_EMBARQUE_BASE 3 
#define CHANCE_POUSO_FORCADO 21
#define COMBUSTIVEL_VIAGEM 25
typedef struct _aviao
{
	int id;
	int estado;
	int combustivel;
	int aeroporto_atual;
	int limpo;
	int first;
	pthread_mutex_t apenas_limpa;
	pthread_cond_t espera_limpeza;
}Aviao;

void aviaoinit(Aviao *aviao);
void AviaoDestroy(Aviao *aviao);

typedef struct _aeroporto
{
	pthread_mutex_t pista;
	pthread_mutex_t atualiza;
	sem_t portoes;
	sem_t equipe_solo;
	pthread_cond_t pemerg;
	pthread_cond_t pnormal;
	int emergencia;
	int normal;
}Aeroporto;

void AeroportoInit(Aeroporto* aeroporto);
void AeroportoDestroy(Aeroporto * aeroporto);
#endif
