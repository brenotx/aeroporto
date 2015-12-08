#include"estruturas.h"
//--------------Aviao--------------------
void aviaoinit(Aviao *aviao)
{
	unsigned int seed = time(NULL);	
	for(int i=0;i<QTD_AVIAO;i++)
	{
		aviao[i].id = i;
		aviao[i].estado = ESTADO_GARAGEM;
		aviao[i].combustivel = MAX_COMBUSTIVEL;
		aviao[i].aeroporto_atual = rand_r(&seed)%QTD_AEROPORTOS;
		aviao[i].limpo = 1;
		aviao[i].first = 1;
		pthread_mutex_init(&aviao[i].apenas_limpa,NULL);
		pthread_cond_init(&aviao[i].espera_limpeza,NULL);
	}
}
void AviaoDestroy(Aviao *aviao)
{
	for(int i=0;i<QTD_AVIAO;i++)
	{
		pthread_mutex_destroy(&aviao[i].apenas_limpa);
		pthread_cond_destroy(&aviao[i].espera_limpeza);
	}
	free(aviao);	
}
//---------------------------------------
//--------------Aeroporto------------------
void AeroportoInit(Aeroporto* aeroporto)
{
	for(int i=0;i<QTD_AEROPORTOS;i++)
	{
		pthread_mutex_init(&aeroporto[i].pista,NULL);
		pthread_mutex_init(&aeroporto[i].atualiza,NULL);
		sem_init(&aeroporto[i].portoes,0,CAPACIDADE_AEROPORTO);
		sem_init(&aeroporto[i].equipe_solo,0,CAPACIDADE_SOLO);
		aeroporto[i].emergencia=0;
		aeroporto[i].normal=0;
		pthread_cond_init(&aeroporto[i].pemerg,NULL);
		pthread_cond_init(&aeroporto[i].pnormal,NULL);
	}
}

void AeroportoDestroy(Aeroporto * aeroporto)
{
	for(int i=0;i<QTD_AEROPORTOS;i++)
	{
		pthread_mutex_destroy(&aeroporto[i].pista);
		pthread_mutex_destroy(&aeroporto[i].atualiza);
		sem_destroy(&aeroporto[i].portoes);
		sem_destroy(&aeroporto[i].equipe_solo);
		pthread_cond_destroy(&aeroporto[i].pemerg);
		pthread_cond_destroy(&aeroporto[i].pnormal);
	}
	free(aeroporto);
}
//---------------------------------------
