#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>
#include"estruturas.h"

Aviao *avioes;
Aeroporto *aeroporto;
sem_t mecanico;
int pouso_forcado = 0;

void * torre_controle(void * id);
void *prepara_voo(void *id);
int main()
{
	printf("\tQTD_AVIOES : %d;\n \
	MAX_COMBUSTIVEL : %d;\n \
	CAPACIDADE_AEROPORTO %d;\n \
	CAPACIDADE_SOLO : %d;\n \
	TEMPO_VIAGEM_OFFSET : %d;\n \
	TEMPO_VIAGEM_BASE : %d;\n \
	TEMPO_ABASTECER : %d;\n \
	TEMPO_PREPARAR : %d;\n \
	TEMPO_DECOLAR : %d;\n \
	TEMPO_POUSAR : %d;\n \
	TEMPO_DESEMBARQUE_OFFSET : %d;\n \
	TEMPO_EMBARQUE_OFFSET : %d;\n \
	CHANCE_POUSO_FORCADO : %d;\n \
	COMBUSTIVEL_VIAGEM : %d.\n\n",
	QTD_AVIAO,
	MAX_COMBUSTIVEL,
	CAPACIDADE_AEROPORTO,
	CAPACIDADE_SOLO,
	TEMPO_VIAGEM_OFFSET,
	TEMPO_VIAGEM_BASE,
	TEMPO_ABASTECER,
	TEMPO_PREPARAR,
	TEMPO_DECOLAR,
	TEMPO_POUSAR,
	TEMPO_DESEMBARQUE_OFFSET,
	TEMPO_EMBARQUE_OFFSET,
	CHANCE_POUSO_FORCADO,
	COMBUSTIVEL_VIAGEM);
	getchar();

	pthread_t *aviaoThreads;
	// Alocação de memoria
	avioes = (Aviao*) malloc (QTD_AVIAO * sizeof(Aviao));
	aeroporto = (Aeroporto*) malloc (QTD_AEROPORTOS * sizeof(Aeroporto));
	aviaoThreads = (pthread_t*) malloc (QTD_AVIAO * sizeof(pthread_t));
	printf("Memoria alocada\n");

	// Inicialização basica
	aviaoinit(avioes);
	AeroportoInit(aeroporto);
	printf("Inicialização concluida\n");

	// Criacao das threads
	for(int i=0;i<QTD_AVIAO;i++)
	{
		pthread_create(&aviaoThreads[i], NULL, &torre_controle, (void*)(intptr_t)i);
	}
	for(int i=0;i< QTD_AVIAO;i++)
	{
		pthread_join(aviaoThreads[i],NULL);
	}

	// Liberando a memoria
	AviaoDestroy(avioes);
	AeroportoDestroy(aeroporto);
	free(aviaoThreads);

	pthread_exit(NULL);
	printf("Memória Liberada\n");
	return 0;
}

//Funcao que controla as acoes dos avioes
void *torre_controle(void *id)
{
	int meu_id = (intptr_t)id;
	unsigned int seed = time(NULL)+meu_id;
	int chance;
	int forcada;
	while(1)
	{
		switch(avioes[meu_id].estado)
		{
			//Caso do pouso forcado
			case ESTADO_POUSO_FORCADO:
				//Segura a pista para pouso
				pthread_mutex_lock(&aeroporto[avioes[meu_id].aeroporto_atual].pista);
				printf("\t\t|FORCADA| Aviao %d pousando no aeroporto %d----\n",avioes[meu_id].id,avioes[meu_id].aeroporto_atual);
				sleep(TEMPO_POUSAR);
				//Solta a pista depois que o pouso foi realizado
				pthread_mutex_unlock(&aeroporto[avioes[meu_id].aeroporto_atual].pista);				
				//Lock para garantir consistencia de dados
				pthread_mutex_lock(&aeroporto[avioes[meu_id].aeroporto_atual].atualiza);
				//Libera o aeroporto de pousos emergenciais
				aeroporto[avioes[meu_id].aeroporto_atual].emergencia = 0;	
				//Libera o lock de consistencia de dados
				pthread_mutex_unlock(&aeroporto[avioes[meu_id].aeroporto_atual].atualiza);
				//Avisa os outros avioes que pousos e decolagens podem voltar ao normal
				pthread_cond_broadcast(&aeroporto[avioes[meu_id].aeroporto_atual].pemerg);
				//Coloca o aviao em modo de desembarque
				avioes[meu_id].estado = ESTADO_DESEMBARQUE;
				break;
			//Caso do pouso normal
			case ESTADO_POUSANDO:
				//Segura a pista para pouso
				printf("\t----Aviao %d solicitando autorizacao para pousar----\n",avioes[meu_id].id);
				pthread_mutex_lock(&aeroporto[avioes[meu_id].aeroporto_atual].pista);
				//Segura os avioes que precisam de pouso normal caso exista um pouso emergencial, para tal utiliza uma variavel de condicao e uma variavel indicando se existe pouso emergencial no aeroporto. Libera a pista
				while(aeroporto[avioes[meu_id].aeroporto_atual].emergencia)
				{
					printf("\t\t|Aviao %d aguardando pouso emergencial no aeroporto %d|\n",avioes[meu_id].id, avioes[meu_id].aeroporto_atual);
					//Wait da variavel de condicao
					pthread_cond_wait(&aeroporto[avioes[meu_id].aeroporto_atual].pemerg,&aeroporto[avioes[meu_id].aeroporto_atual].pista);		
				}
				printf("\t\t|Aprovada| Aviao %d pousando no aeroporto %d----\n",avioes[meu_id].id,avioes[meu_id].aeroporto_atual);
				sleep(TEMPO_POUSAR);
				//Libera a pista para outros avioes
				pthread_mutex_unlock(&aeroporto[avioes[meu_id].aeroporto_atual].pista);
				//Segura o lock de consistencia de dados
				pthread_mutex_lock(&aeroporto[avioes[meu_id].aeroporto_atual].atualiza);
				//Libera o aeroporto de pousos normais
				aeroporto[avioes[meu_id].aeroporto_atual].normal=0;
				//Solta o lock de consistencai de dados
				pthread_mutex_unlock(&aeroporto[avioes[meu_id].aeroporto_atual].atualiza);
				//Avisa aos outros avioes que a decolagem esta liberada
				pthread_cond_broadcast(&aeroporto[avioes[meu_id].aeroporto_atual].pnormal);
				//Coloca o aviao em modo desembarque
				avioes[meu_id].estado = ESTADO_DESEMBARQUE;
				break;
			//Caso da decolagem
			case ESTADO_DECOLANDO:
				//Segura a pista para decolagem
				printf("\t----Aviao %d solicitando autorizacao para decolar ----\n",avioes[meu_id].id);
				pthread_mutex_lock(&aeroporto[avioes[meu_id].aeroporto_atual].pista);
				//Segura os avioes que precisam decolar caso exista alguem com pouso emergencial, para tal utiliza a variavel de condicao e uma variavel indicando se existe pouso emergencial no aeroporto. Libera a pista
				while(aeroporto[avioes[meu_id].aeroporto_atual].emergencia)
				{
					printf("\t\t|Aviao %d aguardando pouso emergencial no aeroporto %d|\n",avioes[meu_id].id, avioes[meu_id].aeroporto_atual);
					//Wait da variavel de condicao
					pthread_cond_wait(&aeroporto[avioes[meu_id].aeroporto_atual].pemerg,&aeroporto[avioes[meu_id].aeroporto_atual].pista);		
				}
				//Se nao existe aviao em emergencia, segura a decolagem caso exista um aviao em pouso normal. Afim de gastar menos combustivel. Libera a pista
				while(aeroporto[avioes[meu_id].aeroporto_atual].normal)
				{
					printf("\t|Aviao %d aguardando pouso para decolar no aeroporto %d|\n",avioes[meu_id].id,avioes[meu_id].aeroporto_atual);
					//Wait da variavel de condicao
					pthread_cond_wait(&aeroporto[avioes[meu_id].aeroporto_atual].pnormal,&aeroporto[avioes[meu_id].aeroporto_atual].pista);
				}
				//Se chegar aqui pode decolar
				printf("\t\t|Aprovada| Aviao %d decolando no aeroporto %d----\n",avioes[meu_id].id,avioes[meu_id].aeroporto_atual);
				sleep(TEMPO_DECOLAR);
				//Libera a pista apos decolagem
				pthread_mutex_unlock(&aeroporto[avioes[meu_id].aeroporto_atual].pista);
				//Coloca o aviao em modo voo
				avioes[meu_id].estado = ESTADO_VOANDO;
				break;
			//Caso de garagem
			case ESTADO_GARAGEM:
				//Estado base do aviao, apenas para funcionamento inicial, pois o aviao deve estar em algum modo na primeira vez
				printf("\t----Aviao %d esta no portao----\n",avioes[meu_id].id);
				//Aguarda limpeza do aviao
				pthread_mutex_lock(&avioes[meu_id].apenas_limpa);
				while(!avioes[meu_id].limpo)
				{
					printf("Aviao %d sendo preparado para voo", avioes[meu_id].id);
					//Wait da condicao de limpeza
					pthread_cond_wait(&avioes[meu_id].espera_limpeza,&avioes[meu_id].apenas_limpa);
				}
				//Se chegar aqui o aviao foi preparado para voo
				printf("\tAviao %d preparado para voo\n",avioes[meu_id].id);
				//Coloca o aviao em modo embarque
				avioes[meu_id].estado = ESTADO_EMBARQUE;
				pthread_mutex_unlock(&avioes[meu_id].apenas_limpa);
				break;
			//Caso de desembarque
			case ESTADO_DESEMBARQUE:
				//Aguarda um portao ser liberado
				printf("----Aviao %d aguardando entrar no portao----\n",avioes[meu_id].id);
				//Segura o portao para desembarque do aviao
				sem_wait(&aeroporto[avioes[meu_id].aeroporto_atual].portoes);
				//Gera uma nova thread para preparacao do aviao
				pthread_t prepara;
				pthread_create(&prepara,NULL,&prepara_voo,(void*)(intptr_t)meu_id);
				chance = TEMPO_DESEMBARQUE_OFFSET + rand_r(&seed)%TEMPO_DESEMBARQUE_OFFSET;
				sleep(chance);
				//Coloca o aviao em modo garagem
				avioes[meu_id].estado = ESTADO_GARAGEM;
				break;
			//Caso de embarque
			case ESTADO_EMBARQUE:
				chance = TEMPO_EMBARQUE_OFFSET + rand_r(&seed)%TEMPO_EMBARQUE_BASE;
				sleep(chance);
				//Libera o portao do aviao
				if(!avioes[meu_id].first)
					sem_post(&aeroporto[avioes[meu_id].aeroporto_atual].portoes);
				else
					avioes[meu_id].first=0;
				//Coloca o aviao em modo decolagem
				avioes[meu_id].estado = ESTADO_DECOLANDO;
				break;
			//Caso de voo
			case ESTADO_VOANDO:
				printf("----Aviao %d esta em viagem----\n",avioes[meu_id].id);
				chance  = TEMPO_VIAGEM_OFFSET + rand_r(&seed)%TEMPO_VIAGEM_BASE;
				sleep(chance/2);
				avioes[meu_id].combustivel-=COMBUSTIVEL_VIAGEM/2;
				//Checa se havera pouso forcado
				forcada = rand_r(&seed)%100;
				if(forcada < CHANCE_POUSO_FORCADO)
				{
					//Coloca o aviao em modo pouso forcado
					avioes[meu_id].estado = ESTADO_POUSO_FORCADO;
					//Segura o lock de consistencia de dados
					pthread_mutex_lock(&aeroporto[avioes[meu_id].aeroporto_atual].atualiza);
					//Ativa modo emergencia no aeroporto
					aeroporto[avioes[meu_id].aeroporto_atual].emergencia = 1;
					//Solta o lock de consistencia de dados
					pthread_mutex_unlock(&aeroporto[avioes[meu_id].aeroporto_atual].atualiza);
					printf("\t|AVIAO %d NECESSITA POUSO FORCADO|\n",avioes[meu_id].id);
					//Avanca para o proximo loop
					continue;
				}
				sleep(chance/2);
				avioes[meu_id].combustivel-=COMBUSTIVEL_VIAGEM/2;
				//Avanca o aviao para o proximo aeroporto
				avioes[meu_id].aeroporto_atual = (avioes[meu_id].aeroporto_atual + 1)%QTD_AEROPORTOS;
				//Marca o aviao para limpeza
				avioes[meu_id].limpo = 0;
				//Segura o lock de consistencia de dados
				pthread_mutex_lock(&aeroporto[avioes[meu_id].aeroporto_atual].atualiza);
				//Ativa o modo pouso normal no aeroporto
				aeroporto[avioes[meu_id].aeroporto_atual].normal = 1;
				//Solta o lock de consitencia de dados
				pthread_mutex_unlock(&aeroporto[avioes[meu_id].aeroporto_atual].atualiza);
				//Coloca o aviao em modo pouso
				avioes[meu_id].estado = ESTADO_POUSANDO;
				break;
		}
	}
	return 0;
}


//Funcao para preparar o aviao para o proximo voo
void *prepara_voo(void *id)
{
	int meu_id = (intptr_t)id;
	//Aguarda uma equipe terrestre para preparacao do aviao
	printf("---Aviao %d aguardando preparação para voo---\n",avioes[meu_id].id);
	sem_wait(&aeroporto[avioes[meu_id].aeroporto_atual].equipe_solo);
	//Checa necessidade de reabastecimento
	if(avioes[meu_id].combustivel < MAX_COMBUSTIVEL/4)
	{
		sleep(TEMPO_ABASTECER);
		printf("\t\t---|AVIAO %d ABASTECIDO|---\n", avioes[meu_id].id);
		avioes[meu_id].combustivel = MAX_COMBUSTIVEL;
	}
	sleep(TEMPO_PREPARAR);
	//Marca o aviao como limpo
	avioes[meu_id].limpo = 1;
	//Avisa que esta limpo
	pthread_cond_signal(&avioes[meu_id].espera_limpeza);
	//Libera uma equipe terrestre
	sem_post(&aeroporto[avioes[meu_id].aeroporto_atual].equipe_solo);
	return 0;
}

