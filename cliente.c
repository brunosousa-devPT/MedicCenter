#include "cliente.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <signal.h>
#include <pthread.h>

#define FCTB "FIFOcliente"
#define FB "FIFOespecialista"


char globalCan[8];

void encerra();

void *verificaBalcao() {
	while (1) {
		sleep(5);

		if (access(FCTB, F_OK) != 0){
			printf("\n\nBALCAO ENCERROU!!!\n\nAs melhoras.Resto de um bom dia\n");
			encerra();
		}
	}
}


void encerra() {
	unlink(globalCan);

	printf("\n");
	exit(EXIT_FAILURE);
}

void ctrlC(int s) {
	eBC envia;
	envia.pid = getpid();
	envia.pidCliente = -3; // significa que e ctrl c do cliente
	strcpy(envia.especialidade, "CTRLC");
	strcpy(envia.canal, "CTRLCCC");
	strcpy(envia.tipo, "CLI");

	int medEnvia = open(FB, O_WRONLY);
	
	int n = write(medEnvia, &envia, sizeof(eBC));		
	close(medEnvia);


	printf("--> CTRL C <-- \n");
	encerra();
}



int main (int argc, char *argv[]) {
		
	Crec dataReceived;	
	cliente dados;
	int cliente2balcao, balcao2cliente,n, clienteFicheiro, atendido=0;  // atendido -0 a espera, 1 especialista
	int Cliente2EspecialistaM;
	char str[20];	
	char especialistaCanal[8];
	pthread_t toCheck;


	if (argc < 2) {
		printf("\n./cliente nome\n");
		return -1;			
	}

	//select 
	fd_set fds;
	struct timeval tempo;
	int res;
	char mensagem2Esp[MAX];
	char sintomas[MAX];
	char nome[MAX];
	char FIFO_CLIENTE[8] = "CLI";
	char pidSTR[5];
	strcpy(dados.nome,argv[1]);
	dados.pid = getpid();
	sprintf(pidSTR,"%d",dados.pid);
	strcat(FIFO_CLIENTE, pidSTR);
	FIFO_CLIENTE[8] = '\0';
	strcpy(dados.canal, FIFO_CLIENTE);
	strcpy(dados.tipo,"cliente");
	if (access(FCTB, F_OK) != 0) {
		fprintf(stderr, "Balcao Indisponivel!\n");
		exit(EXIT_FAILURE);
	}

	if (access(dados.canal, F_OK) == 0) {
		fprintf(stderr, "Cliente ja existe");
		exit(2);
	}
	if (mkfifo(dados.canal, 0600) != 0) {
		fprintf(stderr, "Erro na criacao do fifo!");
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, ctrlC);


	strcpy(globalCan, dados.canal);
	clienteFicheiro = open(dados.canal, O_RDWR);
	cliente2balcao = open(FCTB, O_RDWR);
    if (cliente2balcao == -1) { //bloqueante
		fprintf(stderr, "Erro a abrir o FIFO: %d\n",dados.pid);
		exit(EXIT_FAILURE);
	}

	

	if (clienteFicheiro == -1) {
		fprintf(stderr, "Erro a abrir ficheiro CLIENTEFICHEIRO\n");
		exit(EXIT_FAILURE);
	}
	printf("\n::. CLIENTE iniciado! .::\n");

	printf("Por favor, insira os seus sintomas: ");
	fgets(dados.sintomas, sizeof(dados.sintomas), stdin);
	do {
	fflush(stdout);	
	n = write(cliente2balcao,&dados,sizeof(cliente));
	//printf("%d", n);
	if (n == sizeof(cliente)){
		//printf("\n%s\n", dados.tipo);
		break;	
	}
	}while(1);
	close(cliente2balcao);
	


	do {
		if (!atendido) {
			n = read(clienteFicheiro, &dataReceived, sizeof(dataReceived));
			if (n == sizeof(dataReceived)) {
				if (dataReceived.estado == -1) {
					printf("Por favor, volte mais tarde.\n");
					encerra();}
				printf("\nEspecialidade: %s || Prioridade: %d  \n", dataReceived.especialidade, dataReceived.prioridade);
				break;
			}

		}
	}while(1);

	if (pthread_create(&toCheck, NULL, verificaBalcao, NULL) != 0) {
		printf("ERRO NA THREAD");
	}


	do {
		printf("Aguarde...Irá ser observado brevemente.\n\n");
		n = read(clienteFicheiro, especialistaCanal, sizeof(especialistaCanal));

		if (n == sizeof(especialistaCanal)) {


			printf("Chegou a sua vez! Está a ser encaminhado para o consultorio...\n\n\n");
			Cliente2EspecialistaM = open(especialistaCanal, O_WRONLY);
			n = write(Cliente2EspecialistaM, dados.canal, sizeof(dados.canal));
			if (n != sizeof(dados.canal)) {
				printf("Erro na comunicacao, por favor tente outra vez!\n");
				close(Cliente2EspecialistaM);
				unlink(dados.canal);
				close(clienteFicheiro);
				exit(EXIT_FAILURE);
			}
			break;
		}


	}while(1);
		


	do {


		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(clienteFicheiro,&fds);

		tempo.tv_sec =5;
		tempo.tv_usec = 0;
		res = select(clienteFicheiro + 1, &fds, NULL, NULL, &tempo);

		if (res == 0) {
			if (access(especialistaCanal, F_OK) != 0) {
				printf("\n\nO medico morreu! Tenha o resto de um bom dia.\n\n");
				ctrlC(20);
			}

		}
		else if (res > 0 && FD_ISSET(0, &fds)) { 

			fgets(mensagem2Esp, sizeof(mensagem2Esp), stdin);

			//printf("\n");

			n = write(Cliente2EspecialistaM, mensagem2Esp, sizeof(mensagem2Esp));
			if (n != sizeof(mensagem2Esp)) {
				printf("Erro no contato\n");
				break;
			}

			if (!strcmp(mensagem2Esp, "Adeus\n") || (!strcmp("adeus\n",mensagem2Esp)) ) {
				close(Cliente2EspecialistaM);
				printf("\n\nDesejamos as melhoras! :)\n");
				break;
			}

		}
		else if (res > 0 && FD_ISSET(clienteFicheiro, &fds)) {
			n = read(clienteFicheiro, mensagem2Esp, sizeof(mensagem2Esp));

			if (!strcmp(mensagem2Esp,"Sair\n")) {
				close(Cliente2EspecialistaM);
				printf("\n\nMedico foi se embora! As melhoras.\n");
				ctrlC(0);
				break;
			}

			printf("\n::. MEDICO: %s\n", mensagem2Esp);


		}

	}while(1);



	close(clienteFicheiro);
	unlink(dados.canal);

}

