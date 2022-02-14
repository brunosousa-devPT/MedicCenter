#ifndef MEDICO_H
#define MEDICO_H



#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define FIFO_CON "consultorio" //criar fifo no consultorio
#define FETB "FIFOcliente"
#define FB "FIFOespecialista"


typedef struct medic medico;
typedef char string[256];

struct medic {
	
	int pid;
	char nome[256];
	char canal[8];
	char sintomas[256];
	char tipo[8];


};
typedef struct especialistBalcaoContact {
	int pid;
	int pidCliente;
	char tipo[3];
	char especialidade[256];
	char canal[8];
}eBC;


#endif
