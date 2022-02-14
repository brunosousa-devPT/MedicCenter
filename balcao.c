
#include "balcao.h"
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>




void adicionarEspecialista(estadoEspecialista** root, especialista b) {
	estadoEspecialista * novo_especialista = malloc(sizeof(estadoEspecialista));
	if(novo_especialista == NULL)
		exit(1);

	strcpy(novo_especialista->a.especialidade,b.especialidade);
	strcpy(novo_especialista->a.nome,b.nome);
	novo_especialista->a.estado = b.estado;
	novo_especialista->a.pid = b.pid;

	novo_especialista->next = NULL;
	novo_especialista->a.begin = time(NULL); //tempo para check
	estadoEspecialista * curr = * root;
	if(*root == NULL){
		*root = novo_especialista;
		return;
	}
	while(curr->next != NULL){
		curr = curr->next;
	}

	curr->next = novo_especialista;
}


void adicionarCliente(estadoClientes** root, cliente *b) {
	estadoClientes * new_client= malloc(sizeof(estadoClientes));
	if (new_client == NULL) {
		printf("Erro a adicionar cliente!");
		exit(1);
	}

	strcpy(new_client->a.nome,b->nome);
	strcpy(new_client->a.especialidade,b->especialidade);
	new_client->a.pid = b->pid;
	new_client->a.prioridade = b->prioridade;
	new_client->a.estado = b->estado;
	strcpy(new_client->a.canal, b->canal); //FLAG (1)
	strcpy(new_client->a.sintomas, b->sintomas);

	
	new_client->next = NULL;

	
	if(*root == NULL) {
		*root = new_client;
	
		return;
	}

	estadoClientes*  curr = *root;
	while(curr->next != NULL) {
		curr = curr-> next;
		
	}
	curr->next = new_client;
	

}
void listarUtentes(estadoClientes** root) {
	if (*root == NULL) printf("Nenhum cliente registado!\n");
	estadoClientes* curr = * root;
	while(curr != NULL) {
		printf("Nome do Cliente: %s\nDados{ PID: %d || Estado: %d || Especialidade: %s || Prioridade: %d }\n", curr->a.nome, curr->a.pid, curr->a.estado, curr->a.especialidade, curr->a.prioridade);
		curr = curr->next;
	}


}


void listaEspecialistas(estadoEspecialista ** root) {
	if (*root == NULL) printf("Nenhum especialista registado!\n");

	estadoEspecialista * curr = *root;
	while(curr != NULL) {
		printf("Nome do Especialista: %s\nDados{ PID: %d || Estado: %d || Especialidade: %s }\n", curr->a.nome, curr->a.pid, curr->a.estado, curr->a.especialidade);
		curr = curr->next;
	}
}

void removerUtente(estadoClientes** root, int pid) {
	if(*root == NULL) {
		return;
	}
	if((*root)->a.pid == pid) {
		estadoClientes * to_remove = *root;
		*root = (*root)->next;
		free(to_remove);
		return;
	}
	for(estadoClientes * curr = *root; curr->next != NULL; curr = curr->next) {
		if(curr->next->a.pid == pid) {
			estadoClientes * to_remove = curr->next;
			curr->next = curr->next->next;
			free(to_remove);
			return;
		}
	}
	printf("\n\nNão existe nenhum cliente com esse PID: %d\n\n", pid);

}

void removerEspecialista(estadoEspecialista** root, int pid) {
	if (*root == NULL) {
		return;
	}
	if((*root)->a.pid == pid) {
		estadoEspecialista *to_remove = *root;
		*root = (*root)->next;
		free(to_remove);
		return;
	}
	for(estadoEspecialista * curr = *root; curr->next != NULL; curr = curr->next) {
		if(curr->next->a.pid == pid) {
			estadoEspecialista * to_remove = curr->next;
			curr->next = curr->next->next;
			free(to_remove);
			return;
		}
	}
	printf("\n\nNão existe nenhum especialista com esse PID: %d\n\n", pid);
	

}

void libertarEspaço(estadoClientes** rootC, estadoEspecialista ** rootE) {
	estadoClientes* currClientes = *rootC;
	estadoEspecialista* currEsp =  *rootE;

	while(currClientes != NULL) {
		estadoClientes * aux1 = currClientes;
		currClientes = currClientes->next;
		free(aux1);
	}
	*rootC = NULL;

	while(currEsp != NULL) {
		estadoEspecialista * aux2 = currEsp;
		currEsp = currEsp->next;
		free(aux2);
	}
	*rootE = NULL;


}


void  Balcao2Classificador(char *sintomas, char* veredicto) {
	setbuf(stdin,NULL);
	//variavel para o fork
	int res;
	int iterador=0; //loop para buff2 
	int a =0;
	//pipes
	int baltoclf[2]; //balcao para o classificador
	int clftobal[2]; //classificador para o balcao
	
	char buff[256]; //escrita
	char buff2[256]; //leitura
	char comando[20];
	int n_bytes;
	strcpy(comando,"classificador");
	
	if(pipe(baltoclf) == -1) {
		printf("Erro no pipe baltoclf\n");
		return ;
	}
	if(pipe(clftobal) == -1) {
		printf("Erro no pipe clftobal\n");
		return ;
	}
	
	res = fork();

	if(res < 0) {
		perror("Erro no fork!\n");
		return;
	}

	else if (res == 0) {
				
		close(STDIN_FILENO);
		dup(baltoclf[0]);
		close(STDOUT_FILENO);
		dup(clftobal[1]);


		close(clftobal[0]);
		close(clftobal[1]);
		close(baltoclf[0]);
		close(baltoclf[1]);
		execl(comando,comando,NULL);
		fprintf(stderr, "Erro na execucao\n");
		exit(123);

		
	}
	
	else {
		close(baltoclf[0]);
		close(clftobal[1]);
		a = 0;	
		do{	
			if (a == 1) {
				char fim[8] = "#fim";
				write(baltoclf[1], fim, strlen(fim));
				close(baltoclf[1]);
				close(clftobal[0]);
				break;
			}

			
			
			if (write(baltoclf[1], sintomas,strlen(sintomas)) == -1) {
				printf("Erro write.\n");
				exit(2);
			}
			
			
			
	
		
			if (read(clftobal[0],veredicto,SIZEBUFFER) == -1) { //SIZEBUFFER = 255
				printf("Erro read.\n");

				exit(3);
			}
	
			for(iterador = 0; veredicto[iterador] != '\n'; iterador++); 
			veredicto[iterador +1] = '\0';

			
			
		
			a++;
			
		}while(1);			
		
	}
}

//Funcao que vai buscar as variaveis ambientes MAXMEDICOS e MAXCLIENTES
void variaveisAmbiente(varenv *var) {
	char *lixo;
	char *lixo1;
	var->MAXCLIENTES = strtol(getenv("MAXCLIENTES"), &lixo, 10);
	var->MAXMEDICOS = strtol(getenv("MAXMEDICOS"),&lixo1,10);
}
	



int countClientsOnHold(estadoClientes **root) {
	int a = 0;
	if (root == NULL)
		return 0;
	estadoClientes* curr = * root;
	while(curr != NULL) {
		//printf("Nome do Cliente: %s\n Dados {Estado: %d || Especialidade: %s || Prioridade: %d}", curr->a.nome, curr->a.estado, curr->a.especialidade, curr->a.prioridade);
		if (curr->a.estado == 0)
			a++;
		curr = curr->next;

	}
	return a;
}
int countEspecialistas(estadoEspecialista **root) {
	int a = 0;
	estadoEspecialista* curr = * root;
	while(curr != NULL) {
		//printf("Nome do Especialista: %s\n Dados {Estado: %d || Especialidade: %s }", curr->a.nome, curr->a.estado, curr->a.especialidade);
		curr = curr->next;
		a++;
	}
	return a;
}



char* encontraCliente(estadoClientes ** root,  char *especialidade, char *FIFO_C) {

	
	char pidSTR[5];

	estadoClientes* aux = NULL;
	estadoClientes* curr = * root;
	while(curr != NULL) {
		
		if (!strcmp(curr->a.especialidade,especialidade)) {
			if (curr->a.estado == 0) {
				if (aux == NULL) {
					aux = curr;

				}
				else{
					if (aux->a.prioridade > curr->a.prioridade) {
						aux = curr;
					}
				}
			}
			
		}
		
		curr = curr->next;

	}
	sprintf(pidSTR,"%d",aux->a.pid);
	strcat(FIFO_C, pidSTR);
	FIFO_C[8] = '\0';
	return FIFO_C;
}

char* encontraEspecialista(estadoEspecialista ** root,  int pid) {


	estadoEspecialista* curr = *root;
	while(curr != NULL) {
		
		if (curr->a.pid == pid) {
			return curr->a.especialidade;
		}
			
		
		curr = curr->next;

	}
	return NULL;
}

void mudaEstadoCliente(estadoClientes ** root,  int pid) {

	estadoClientes * curr = *root;
	//estadoClientes * curr = *root;
	while(curr != NULL) {
		if (curr->a.pid == pid) {
			if (curr->a.estado == 1) curr->a.estado = 0;
			if (curr->a.estado == 0) curr->a.estado = 1;


			break;
		}
		curr = curr->next;
	}

}


void mudaEstadoEspecialista(estadoEspecialista ** root,  int pid, int estado) {

	estadoEspecialista * curr = *root;
	//estadoClientes * curr = *root;
	while(curr != NULL) {
		if (curr->a.pid == pid) {
			curr->a.estado = estado;


			break;
		}
		curr = curr->next;
	}

}

char* getCanalEsp(int pid, char *FIFO_CLIENTE) {
	
	char pidSTR[5];
	sprintf(pidSTR,"%d",pid);
	strcat(FIFO_CLIENTE, pidSTR);
	FIFO_CLIENTE[8] = '\0';
	return FIFO_CLIENTE;
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

int max(int a, int b) {
	return (a>b) ? a: b;
}



void encerra() {
	unlink(FCTB);
	unlink(FETB);
	printf("\n");
	exit(EXIT_FAILURE);
}

void ctrlC(int s) {
	printf("--> CTRL C <-- \n");
	encerra();
}

int compStrings(char *string1, char *string2) {
	int bytes;
	if (strlen(string1) >= strlen(string2)) {
		bytes = strlen(string2);
	}
	else {
		bytes = strlen(string1);
	}
	
	int bol = 1;
	for (int i = 0; i< bytes; i++) {
		if (string1[i] != string2[i]) {
			return 0;
		}
	}

	return bol;
}

int procuraCliente(estadoClientes ** root, char *especialidade) {
	if (*root == NULL) {
		return 0;
	}

	estadoClientes * aux = *root;

	estadoClientes * aux2 = NULL;


	while(aux != NULL) {
		if (compStrings(aux->a.especialidade, especialidade) && aux->a.estado != 1) {
			if (aux2 == NULL) {
				aux2 = aux;
			}
			else {
				if (aux2->a.prioridade > aux->a.prioridade) {
					
					aux2 = aux;
				}
			}

		}
		aux = aux->next;

	}

	if (aux2 == NULL)
		return 0;

	return aux2->a.pid;
}


int procuraLivre(estadoEspecialista ** rootEs, estadoClientes**rootCl, int *p) {


	estadoEspecialista * currEs = * rootEs;

	estadoClientes * curr = *rootCl;
	estadoClientes * aux = NULL;

	while (currEs != NULL) {
	
		if (currEs->a.estado == 0) {
			while(curr != NULL) {
			
				if (curr->a.estado == 0 && !strcmp(curr->a.especialidade, currEs->a.especialidade)) {
					if (aux == NULL) {
						aux = curr;
				
					}
					else {
						if (aux->a.prioridade > curr->a.prioridade) aux = curr;
						
					}
				}
			curr = curr->next;
			}
		}
		if (aux != NULL) {
			break;
		}

		currEs = currEs->next;
	}
	if (aux != NULL) {
		
		*p = aux->a.pid;
		return currEs->a.pid;
	}

	return 0;
}


void filasDeEspera(estadoClientes **root) {

	if (*root == NULL) {
		return ;
	}

	

		for (int i = 0; i< 5; i++) filas[i] = 0;
		estadoClientes * curr = * root;
		while (curr != NULL) {
			if (!strcmp(curr->a.especialidade, "geral") && curr->a.estado == 0) {
				filas[0] = filas[0] + 1;

			}
			if (!strcmp(curr->a.especialidade, "ortopedia") && curr->a.estado == 0) {
			filas[1] = filas[1] + 1;
			
		}
		if (!strcmp(curr->a.especialidade, "oftalmologia") && curr->a.estado == 0) {
			filas[2] = filas[2] + 1;
			
		}
		if (!strcmp(curr->a.especialidade, "estomatologia") && curr->a.estado == 0) {
			filas[3] = filas[3] + 1;
			
		}
		if (!strcmp(curr->a.especialidade, "neurologia") && curr->a.estado == 0) {
			filas[4] = filas[4] + 1;
			
		}

		curr = curr->next;
		}
	
	

}

void *alarm_handler() {

	while (1){
		sleep(times);

		printf("\nFila de Espera Geral: %d\n", filas[0]);
		printf("Fila de Espera Ortopedia: %d\n", filas[1]);
		printf("Fila de Espera Oftalmologia: %d\n", filas[2]);
		printf("Fila de Espera Estomatologia: %d\n", filas[3]);
		printf("Fila de Espera Neurologia: %d\n", filas[4]);
	}
	

}
void atualizaTempo(estadoEspecialista ** root, int pid) {

	if (*root == NULL) return;

	estadoEspecialista * curr = *root;
	while(curr != NULL) {
		if (curr->a.pid == pid) {
			curr->a.begin = time(NULL);
	
		}
		curr= curr->next;
	}

}	

int verificaTempo(estadoEspecialista**root) {

	if (*root == NULL) return 0;

	time_t tmp= time(NULL);
	estadoEspecialista * curr = *root;

	while(curr != NULL) {
		if (tmp - curr->a.begin > 20) {
			return curr->a.pid;
	
		}
		curr= curr->next;
	}


	return 0;
}

int verificaNumeroEmHoldClientes(estadoClientes ** root, char * especialidade) {
	int a = 0;

	if (*root == NULL) return 0;

	estadoClientes * curr = *root;

	while(curr!= NULL) {
		if (!strcmp(curr->a.especialidade, especialidade) && curr->a.estado == 0)
			a++;
		curr = curr->next;
	}

	return a;
}


int verificaEstadoUtente(estadoClientes ** root, int pid) {
	
	int a;
	
	if (*root == NULL)
		return 0;

	estadoClientes * curr = *root;
	
	while(curr != NULL){
		if (curr->a.pid == pid)
			return curr->a.estado;

		curr = curr->next;
	}

	return 0;
}


int verificaEstadoEspecialista(estadoEspecialista ** root, int pid) {
	
	int a;
	
	if (*root == NULL)
		return 0;

	estadoEspecialista * curr = *root;
	
	while(curr != NULL){
		if (curr->a.pid == pid)
			return curr->a.estado;

		curr = curr->next;
	}

	return 0;
}


int main(int argc, char * argv[], char* envq) {
	
	varenv var;
	variaveisAmbiente(&var);
	int n, receiveData, res;
	int fd_retorno; // BALCAO -> ESPECIALISTA
	int fd_balEsp; //novo pipe criado pelo balcao que permite comunicaçao a posteriori entre especialista  e balcao
	fd_set fds;
	struct timeval tempo;
	char comando[256];
	struct especialist especialista;
	auxCli clie;
	auxCli clie2; //reset
	char veredicto[240];
	cliente Clien;
	estadoClientes *rootCl = NULL;
	estadoEspecialista * rootESP = NULL;
	sClient enviarCliente;
	eBC auxESPBAL;
	waitStruct auxDataCopy;
	int cpp;

	pthread_t check;

	/*ACESSO RAPIDO VARINT*/
	printf("\n»»» BALCAO inicializado «««\n");
	//teste();


	if (access(FCTB, F_OK) == 0) {
		fprintf(stderr,"Erro no acess\n");
		exit(EXIT_FAILURE);
	}
	if (access(FETB, F_OK) == 0) {
		fprintf(stderr, "Erro no acess FETB\n");
		exit(EXIT_FAILURE);
	}
	

	if (mkfifo(FCTB,0600) != 0) {
		fprintf(stderr,"Erro no FIFO\n");
		exit(EXIT_FAILURE);
	}
	if (mkfifo(FETB,0600) != 0) {
		fprintf(stderr,"Erro no FIFO\n");
		exit(EXIT_FAILURE);
	}


	receiveData= open(FCTB,O_RDWR);
	if (receiveData == -1) {
		fprintf(stderr, "Erro a abrir o fifo: %d\n", getpid());
		exit(EXIT_FAILURE);
	}
	fd_balEsp = open(FETB, O_RDWR);
	if (fd_balEsp == -1) {
		fprintf(stderr, "Erro a abrir o fifo: %d\n", getpid());
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, ctrlC);
//	signal(SIGALRM, alarm_handler);
/*
	printf("\n::. BALCAO: ");
	fflush(stdout);
*/

	if (pthread_create(&check, NULL, alarm_handler, NULL) != 0) {
		printf("ERRO NA THREAD");
	}

	
	do {
		filasDeEspera(&rootCl);
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(receiveData,&fds);
		FD_SET(fd_balEsp,&fds);
	
		tempo.tv_sec =times;
		tempo.tv_usec = 0;
		res = select(max(receiveData,fd_balEsp)+1, &fds, NULL, NULL, &tempo);
		if (res == 0) {
			filasDeEspera(&rootCl);
//			alarm_handler();
			

			/*Verifica tempos*/
			int espPIDax = verificaTempo(&rootESP);

			if (espPIDax != 0) {
				removerEspecialista(&rootESP,  espPIDax);
			}

			int a = 0;
			//printf("A espera:: \n");
			fflush(stdin);
			if (rootCl != NULL && rootESP != NULL) {
				while(1) {
					cpp = procuraLivre(&rootESP,&rootCl, &a); //pid medico
				//	printf("%d", cpp);
					if (cpp != 0) {
						char ESPCAN[8]="ESP";
						char CLICAN[8] = "CLI";
						char sPIDE[5];
						char sPIDC[5];

						sprintf(sPIDE, "%d", cpp);
						sprintf(sPIDC,"%d", a);
							
						strcat(ESPCAN, sPIDE);
						strcat(CLICAN, sPIDC);

						fd_retorno = open(CLICAN, O_RDWR);
						n = write(fd_retorno, ESPCAN, sizeof(ESPCAN));
						close(fd_retorno);
						mudaEstadoEspecialista(&rootESP, cpp,1);
						mudaEstadoCliente(&rootCl,a);
					}
					else {
						
						break;
					}
				}	
				

			}
				

		}
		else if (res > 0 && FD_ISSET(0, &fds)) {
			filasDeEspera(&rootCl);
			comando[0] = '\0';
			//scanf(" %199[^\n]",comando);
			fgets(comando, sizeof(comando), stdin);
			//puts(comando);
				

			if (!strcmp(comando, "encerra\n")) {
				close(receiveData);
				encerra();

				break;
			}
			else if (!strcmp(comando, "utentes\n")){
				listarUtentes(&rootCl);
			}
			else if (!strcmp(comando, "especialistas\n")) {
				listaEspecialistas(&rootESP);
			}
				else if (strstr(comando, "delut")) {
					int auxPID; 
					char ptr[5];

					int estado;

					//char canal[8] = "CLI";
					
					int j = 0;
					for (int i = 6; i< 11; i++) {
						ptr[j] = comando[i];
						j++;
					}

					//strcat(canal, ptr);
					//canal[8] = '\0';

					//puts(canal);

					auxPID = atoi(ptr);

					estado = verificaEstadoUtente(&rootCl, auxPID);

					if (auxPID >= 1000 && estado == 0){
						removerUtente(&rootCl, auxPID);
						/*
						fd_retorno = open(canal, O_WRONLY);
						write(fd_retorno, canal, sizeof(canal));
						close(fd_retorno);
						*/
						
					}

				}
				else if (strstr(comando, "delesp")) {
					int auxPID; 
					char ptr[5];

					int estado;

					//char canal[8] = "ESP";
					
					int j = 0;
					for (int i = 7; i< 12; i++) {
						ptr[j] = comando[i];
						j++;
					}
					
					//strcat(canal, ptr);
					//canal[8] = '\0';

					//puts(canal);

					auxPID = atoi(ptr);

					estado = verificaEstadoEspecialista(&rootESP, auxPID);

					if (auxPID >= 1000 && estado == 0) {
						removerEspecialista(&rootESP, auxPID);
						/*
						fd_retorno = open(canal, O_WRONLY);
						write(fd_retorno, canal, sizeof(canal));
						close(fd_retorno);
						*/
						
					}
				
				}
				else if (strstr(comando, "freq")) {
					int len = strlen(comando);
					unsigned int auxPID; 
					char ptr[len-5];
					int j = 0;
					for (int i = 5; i< len; i++) {
						ptr[j] = comando[i];
						j++;
					}
					auxPID = atoi(ptr);
					printf("SetTime = %d\n",auxPID);
					times = auxPID;
				}
				
				

		}
		else if (res > 0 && FD_ISSET(receiveData, &fds)){
			clie = clie2;
			
			n = read(receiveData, &clie, sizeof(auxCli));
				
			
				
			if (n == sizeof(auxCli)) {

				if (!strcmp(clie.tipo, "cliente")) {
					//printf("Novo cliente: %s\nDados : PID: %d  Sintomas: %s\n",clie.nome,clie.pid,clie.sintomas);
					if (countClientsOnHold(&rootCl) < var.MAXCLIENTES) {
						//printf("%d Clie\n",countClientsOnHold(&rootCl));
						Balcao2Classificador(clie.sintomas, veredicto);					
						strcpy(Clien.nome, clie.nome);
						Clien.pid  = clie.pid;	
						Clien.estado = 0;
						int a = 0;
						for (int i = 0; i< strlen(veredicto) || veredicto[i] != ' '; i++) {
							Clien.especialidade[i] = veredicto[i];
								
							a++;
							if (veredicto[i] == ' '){
								Clien.especialidade[i] = '\0';
								break;
							}
						}

						if (verificaNumeroEmHoldClientes(&rootCl,Clien.especialidade) + 1 > 5) {
							fd_retorno = open(clie.canal, O_WRONLY);
							enviarCliente.estado = -1;
							enviarCliente.prioridade = -1;
							strcpy(enviarCliente.especialidade, "Impossibilidade na atribuicao.");
							n = write(fd_retorno, &enviarCliente, sizeof(sClient));
							
							close(fd_retorno);

						}
						else {
								
							Clien.prioridade = veredicto[a] - '\0' -48;
							printf("\nNovo cliente: %s\nDados: PID = %d, Sintoma = %s",clie.nome,clie.pid,clie.sintomas);
							printf("Resultado do classificador --> Prioridade: %d / Especialidade: %s\n", Clien.prioridade,Clien.especialidade);
										
							adicionarCliente(&rootCl, &Clien);
							enviarCliente.estado = Clien.estado;
							enviarCliente.prioridade = Clien.prioridade;
							strcpy(enviarCliente.especialidade, Clien.especialidade);
							fd_retorno = open(clie.canal, O_WRONLY);
							n = write(fd_retorno, &enviarCliente, sizeof(sClient));
							close(fd_retorno);
								

						}
					
							
						


					}
					else {
						fd_retorno = open(clie.canal, O_WRONLY);
						enviarCliente.estado = -1;
						enviarCliente.prioridade = -1;
						strcpy(enviarCliente.especialidade, "Impossibilidade na atribuicao.");
						n = write(fd_retorno, &enviarCliente, sizeof(sClient));
						
						close(fd_retorno);
					}

				}
				if (!strcmp(clie.tipo, "especil")) {
					
					if (countEspecialistas(&rootESP) < var.MAXMEDICOS) {
						printf("\nNovo Especialista: %s\nDados: PID = %d, Especialidade = %s\n",clie.nome,clie.pid,clie.sintomas);
						strcpy(especialista.nome, clie.nome);
						strcpy(especialista.especialidade, clie.sintomas);

						especialista.pid = clie.pid;
						especialista.estado = 0;
						strcpy(especialista.canal, clie.canal);
						adicionarEspecialista(&rootESP, especialista); 
						int pid = procuraCliente(&rootCl , especialista.especialidade);
						if (pid != 0) {
							char canalC[8]="CLI";						
							char sPid[5];
							sprintf(sPid,"%d",pid);
								
							strcat(canalC,sPid);
							fd_retorno = open(canalC, O_RDWR);
							n = write(fd_retorno, especialista.canal, sizeof(especialista.canal));
							mudaEstadoCliente(&rootCl,pid);
							mudaEstadoEspecialista(&rootESP, especialista.pid,1);
							close(fd_retorno);

						}
					
					}
					else {
						char canal[8];
						strcpy(canal,"ERROOOO");
						fd_retorno = open(clie.canal, O_WRONLY);
						n = write(fd_retorno, canal, sizeof(canal));
						close(fd_retorno);
					}
						
				}
					
			}
		}
		else if (res > 0 && FD_ISSET(fd_balEsp, &fds))
		{
			
			n = read(fd_balEsp, &auxESPBAL, sizeof(auxESPBAL));


			if (n == sizeof(eBC)) {
				if (auxESPBAL.pidCliente ==-1) {
					atualizaTempo(&rootESP, auxESPBAL.pid);
				}
				if (auxESPBAL.pidCliente ==-2) {
					removerEspecialista(&rootESP, auxESPBAL.pid);
				}
				if (auxESPBAL.pidCliente ==-3) {
					removerUtente(&rootCl, auxESPBAL.pid);
				}


				if (auxESPBAL.pidCliente >= 0) {
					removerUtente(&rootCl,auxESPBAL.pidCliente);
					mudaEstadoEspecialista(&rootESP, auxESPBAL.pid,0);
				//	printf("Estado: %d", rootESP->a.estado);
					int pid = 0;
					if (rootCl != NULL)
						pid = procuraCliente(&rootCl , auxESPBAL.especialidade);
					
					if (pid !=0) {
						char auxC[8]= "CLI";
						char sPid[5];
					//	printf("\nPIDCL:%d",pid);
						sprintf(sPid,"%d",pid);
						
						strcat(auxC,sPid);
					//	printf("%s\n", auxC);
							
						if (auxC != NULL){ // mudar para cliente // FIX **FIX**
															
							fd_retorno = open(auxC, O_WRONLY);
							n = write(fd_retorno, auxESPBAL.canal, sizeof(auxESPBAL.canal));
							close(fd_retorno);
							mudaEstadoCliente(&rootCl,pid);
							mudaEstadoEspecialista(&rootESP, auxESPBAL.pid,1);

						}
					
												
					}
				}
			}
		}
			
	
	}while(1);

	

			
	fflush(stdout);	
	
	printf("\n\nResto de um bom dia!");
	exit(0);

}
