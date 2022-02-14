#ifndef CLIENTES_H
#define CLIENTES_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define MAX 256
#define BALCAOFIFO "balcaoFifo"



typedef struct client cliente;
typedef struct clientRec Crec;

struct client {
	int pid;	
	char nome[MAX];
	char canal[8];		
	char sintomas[MAX];
	char tipo[8];
};


struct clientRec {
	char especialidade[MAX];
	int prioridade;
	int estado;
};


typedef struct especialistBalcaoContact {
	int pid;
	int pidCliente;
	char tipo[3];
	char especialidade[256];
	char canal[8];
}eBC;

#endif //CLIENTES_H
