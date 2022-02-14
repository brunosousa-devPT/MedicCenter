#include "medico.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <signal.h>
#include <pthread.h>


char globalCan[8];
int acabar = 1;

void encerra() {
	unlink(globalCan);

	printf("\n");
	exit(EXIT_FAILURE);
}

void *verificaBalcao() {
	while (1) {
		sleep(5);

		if (access(FETB, F_OK) != 0){
			printf("\n\nBALCAO ENCERROU!!!\n\nBoa viagem de regresso a casa\n");
			encerra();
		}
	}

}

void ctrlC(int s) {

	eBC envia;
	envia.pid = getpid();
	envia.pidCliente = -2; // significa que e ctrl c do especialista
	strcpy(envia.especialidade, "CTRLC");
	strcpy(envia.canal, "CTRLCCC");
	strcpy(envia.tipo, "ESP");

	int medEnvia = open(FB, O_WRONLY);
	
	int n = write(medEnvia, &envia, sizeof(eBC));		
	close(medEnvia);




	printf("--> CTRL C <-- \n");
	encerra();
}


void * trataPipes() {

	eBC envia;
	envia.pid = getpid();
	envia.pidCliente = -1; // significa que e ctrl c do especialista
	strcpy(envia.especialidade, "CTRLC");
	strcpy(envia.canal, "CTRLCCC");
	strcpy(envia.tipo, "ESP");

	while(1) {
		sleep(20);
		int medEnvia = open(FB, O_WRONLY);
	
		int n = write(medEnvia, &envia, sizeof(eBC));		
		close(medEnvia);
	}

	


}

int getPidCliente(char *canal) { //Obter pid atraves do canal do cliente 
	char pidS[4];
	int j = 0;
	for (int i = 3; i<7; i++,j++) {
		pidS[j] = canal[i];
	}
	j = atoi(pidS);
	return j;

}

int main(int argc, char * argv[]) {


	int pid_Cliente = 0 ;
	char str_input[256];
	char str_output[256];
	medico a;
	int fd,n,res,fd_retorno;
	char str[100];
	int medico2balcao, medEnvia;
	char canal[8], aux[8], aux_pid[4];
	eBC envioDados;
	pthread_t toSend, toCheck;
	
	/*
	SELECT
	*/
	fd_set fds;
	struct timeval tempo;


	if (argc < 3) {
		printf("./medico nome especialdiade\n");
		return 0;
	}
	
	
	strcpy(a.nome, argv[1]);
	strcpy(a.sintomas, argv[2]); //sintomas e a especialidade
	//printf("Esp: %s", a.sintomas);
	a.pid = getpid();
	//printf("Nome:%s Especialidade:%s PID:%d",a.nome,a.especialidade,a.pid);
	sprintf(aux_pid, "%d", a.pid);
	//printf("aux before: %s\n", aux_pid);
	strcpy(aux, "ESP");
	//printf("pid: %s\n", aux_pid);
	//printf("aux: %s\n",aux);
	for(int i = 0 ; i< 4; i++) aux[3+i] = aux_pid[i];
	//printf("Novo canal: %s", aux);
	aux[8] = '\0';
	strcpy(a.canal, aux);
	strcpy(a.tipo, "especil");
	envioDados.pid = a.pid;
	strcpy(envioDados.tipo,"esp");
	strcpy(envioDados.especialidade, a.sintomas);
	strcpy(envioDados.canal, a.canal);
	strcpy(globalCan, a.canal);

	if (access(FETB, F_OK) != 0) {
		fprintf(stderr,"Balcao Indisponivel!\n");
		exit(1);

	}

	if (access(a.canal, F_OK) == 0) {
		fprintf(stderr,"Erro, mesmo medico deu login. \n");
		exit(1);
	}

	if (mkfifo(a.canal,0600) != 0) {
		fprintf(stderr, "ERRO NO FIFO");
		exit(EXIT_FAILURE);
	}


	medico2balcao = open(FETB, O_WRONLY);
	if (medico2balcao == -1) {
		fprintf(stderr, "Erro a abrir FETB\n");
		exit(2);
	}
	fd = open(a.canal, O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "Erro a abrir a.canal!\n");
		exit(2);
	}

	printf("\n::. MEDICO iniciado! .::\n");

/*	medEnvia = open(FB, O_WRONLY);
	if (medEnvia == -1) {
		fprintf(stderr,"Erro a abrir FB");
		exit(3);
	}
*/

	if (access(FB, F_OK) != 0) {
		fprintf(stderr, "balcao indisponivel!\n");
		exit(1);
	}

	signal(SIGINT, ctrlC);

	/*
		Informar o balcao das caracteristicas do medico
	*/



	do {
		
		fflush(stdout);	
		n = write(medico2balcao,&a,sizeof(medico));
		//printf("%d", n);
		if (n == sizeof(medico)){
		
			break;	
		}
		break;	

	}while(1);


	if (pthread_create(&toSend,NULL,trataPipes,NULL) != 0) {
		printf("ERRO NA THREAD\n");
	} 

	if (pthread_create(&toCheck,NULL,verificaBalcao,NULL) != 0) {
		printf("ERRO NA THREAD\n");
	}

		do {
		if (pid_Cliente ==0) {
			printf("A espera de um utente... \n\n");
			n = read(fd, canal, sizeof(canal));
			//printf("%s", canal);
			if (canal[0] == 'E') {
					acabar = 0;
					encerra();
				}
			if (n == sizeof(canal)) {
				

				envioDados.pidCliente= getPidCliente(canal);
				//printf("canal:%s\n", canal);
				printf("O utente entrou no seu consultório!\n\n\n");

				pid_Cliente = 1;
			}
		}
		if (pid_Cliente == 1) {
			do {
				//printf("\n::. CLIENTE: ");

				FD_ZERO(&fds);
				FD_SET(0, &fds);
				FD_SET(fd,&fds);
				tempo.tv_sec = 8;
				tempo.tv_usec = 0;
				res = select(fd +1, &fds, NULL, NULL, &tempo);
				if (res == 0) {
					//printf("A espera... .:: \n");
					if (access(canal, F_OK) != 0){
						pid_Cliente = 2;
						break;
					}

				}
				else if (res > 0 && FD_ISSET(0, &fds)) { 
					
					fgets(str_input, sizeof(str_input), stdin);

					fd_retorno = open(canal, O_WRONLY);
					n = write(fd_retorno, str_input, sizeof(str_input));
					if (n != sizeof(str_input)) {
						fprintf(stderr, "Erro grave na comunicacao medico-cliente!\n");
						close(fd_retorno);
						close(fd);
						unlink(a.canal);
						exit(3);
					}
					if (!strcmp(str_input, "Sair\n")) {
						pid_Cliente = 2;
						ctrlC(2);
					}
					close(fd_retorno);
				}
				else if (res > 0 && FD_ISSET(fd, &fds)){

					n = read(fd, str_output, sizeof(str_output));

					printf("\n::. CLIENTE: ");

					puts(str_output);

					if (!strcmp(str_output, "Adeus\n") || !strcmp("Adeus", str_output)) {
						pid_Cliente = 2;
						break;
					}

					if (!strcmp(str_output, "Tesla2001")) {
						encerra();
					}

				}
			}while(!strcmp(str_input, "Sair\n") || !strcmp(str_output,"adeus"));
			

		}
		if (pid_Cliente == 2) {
				medEnvia = open(FB, O_WRONLY);
	
				n = write(medEnvia, &envioDados, sizeof(envioDados));
				close(medEnvia);
				pid_Cliente = 0;
				printf("\n\nSessão com o utente terminada. Balcão foi informado!\n");

				
		}
		

	}while(1);


	
	close(fd);
	unlink(a.canal);
	
	exit(0);

	}
	
	



