#ifndef BALCAO_H
#define BALCAO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>


#define FIFO_BALCAO "balcaoFifo"
#define SIZEBUFFER 255
#define MAXLISTAESPERA 5 // maximo de 5 clientes na lista de espera por especialista
#define FCTB "FIFOcliente"
#define FETB "FIFOespecialista"



typedef struct varAmbiente varenv;
typedef struct client cliente;
typedef struct ClientesAtendidos ClienteAtendidos;
typedef struct listaDeEsperaClientes listaClientes;
typedef struct especialist especialista;


int filas[5] = {0,0,0,0,0}; // geral, ortopedia, oftamologia, estamatologia, neurologia
int times = 11;
struct varAmbiente {

	int MAXCLIENTES; 
	int MAXMEDICOS;

};



struct client {
	char nome[256];
	char especialidade[256];
	int pid;
	int prioridade;
	int estado; //0 lista de espera , 1 a ser atendido
	char sintomas[256];
	char canal[8];
};

struct client_Send2Bal {
	char nome[256];
	int pid;
	char sintomas[256];



};

typedef struct auxCliente{
	int pid;
	char nome[256];
	char canal[8];
	char sintomas[256];
	char tipo[8];
}auxCli;

typedef struct sendClient{
	char especialidade[256];
	int prioridade;
	int estado;
}sClient;

typedef struct estadoClientes {
	cliente a;
	struct estadoClientes* next;

}estadoClientes;

struct especialist {
	char nome[256];
	char especialidade[256];
	int pid;
	int estado;
	char canal[8];
	time_t begin;
};
//especialista informacao mandada ao balcao
typedef struct especialistBalcaoContact {
	int pid;
	int pidCliente;
	char tipo[3];
	char especialidade[256];
	char canal[8];
}eBC;

typedef struct waitStruct {
	char canalMedico[8];
	int pid_Cliente;
	int pid_Medico;
	char canalCliente[8];

}waitStruct;


//clientes a serem atendidos
typedef struct estadoEspecialista {

	especialista a;
	struct estadoEspecialista* next;

}estadoEspecialista;


void adicionarEspecialista(estadoEspecialista** root, especialista b);
void adicionarCliente(estadoClientes** root, cliente *b);
void listarUtentes(estadoClientes** root);
void listaEspecialistas(estadoEspecialista** root);
void removerUtente(estadoClientes** root, int pid);
void removerEspecialista(estadoEspecialista** root, int pid);
void libertarEspaço(estadoClientes** rootC, estadoEspecialista** rootE);
void Balcao2Classificador();
void variaveisAmbiente(varenv *var);
int countClientsOnHold(estadoClientes **root);
int countEspecialistas(estadoEspecialista **root);
char* encontraCliente(estadoClientes ** root,  char *especialidade, char *FIFO_C);

#endif //BALCAO_H
