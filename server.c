#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <pthread.h>
#include <semaphore.h>


#define MAX_WORKERS 5
int workers_list[MAX_WORKERS];
int idle_worker[MAX_WORKERS] = {1};
int workers_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t workers_busy;


struct entity_data{
    int sock_entity;
    struct sockaddr_in *entity_addr;
};

// struct worker_data{
// 	int socket;
// 	struct sockaddr_in *worker_addr;
// 	int idle;
// }

// cd = (struct entity_data *)malloc(sizeof(struct entity_data));
//         cd->entity_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
//         addrlen = sizeof(struct sockaddr_in);


int addWorkerList(int sockworker){
	if (workers_count < MAX_WORKERS){
		workers_list[workers_count] = sockworker;
		idle_worker[workers_count] = 1;
		workers_count++;
		sem_post(&workers_busy);
		return workers_count;
	}
	return 0;
}

float callWorker(int worker, char* str){
	int n;
	float res;
	char recvBuffer[1024];
	send(workers_list[worker], str, strlen(str)+1, 0);

	while((n = recv(workers_list[worker], recvBuffer, sizeof(recvBuffer)-1, 0))>0){
		printf("Worker: buffer size:%d\n",n);
		recvBuffer[n] = '\0';
		if(n < 0) perror("fputs");
		else break;
	}
	printf("Worker: %.2lf\n",recvBuffer);
	res = strtof(recvBuffer, NULL);
	return res;
}

int verifyIdleWorker(){
	for (int i=0; i < workers_count; i++){
		if(idle_worker[i]){
			idle_worker[MAX_WORKERS] = 0;
			return i;
		}
	}
	return -1;
}

// float isAliveWorker(int worker){
// 	int n;
//     char sendBuffer[1024];
// 	char recvBuffer[1024];
//  	workers_list
// 	while((n = recv(workers_list[worker], recvBuffer, sizeof(recvBuffer)-1, 0))>0){
// 		printf("Thread Worker: buffer size:%d\n",n);
// 		recvBuffer[n] = '\0';
// 		if(n < 0) perror("fputs");
// 		if(n == 0) break;
// 	}

// }


void *clientHandle(void* cd){
	// srand(time(NULL));
    struct entity_data *client = (struct entity_data *)cd;
    char sendBuffer[1024];
	char recvBuffer[1024];
	int wait_time = rand() % 5 + 1;

	// const char* format = "%.3f\0";
	const char* format = "%.40s\0";
    int n;
    memset(sendBuffer, 0, sizeof(sendBuffer)); 
	memset(workers_list, 0, sizeof(workers_list));


    /* Imprime IP e porta do cliente. */
    printf("Thread: Received connection from %s:%d\n", inet_ntoa(client->entity_addr->sin_addr), ntohs(client->entity_addr->sin_port));
    fflush(stdout);
	
	int s;
	while((n = recv(client->sock_entity, recvBuffer, sizeof(recvBuffer)-1, 0))>0){
		printf("Thread: buffer size:%d\n",n);
		recvBuffer[n] = '\0';
		if(n < 0) perror("fputs");
		else break;
	}
	printf("Thread: Buffer msg: %s\n", recvBuffer);
    char *msg;
	sleep(1);
	float res;
	int w;
	if ( (s = sem_trywait(&workers_busy)) == 0){
		w = verifyIdleWorker();
		if(w < 0) perror("This shoudn't occurrs! Unexpected Error!");
		res = callWorker(w, recvBuffer);

		msg = "Thread: O resultado da operação é: \n";
		snprintf(sendBuffer, sizeof(sendBuffer), "%.40s: %.3f\0", msg, res);
		sleep(wait_time);
		// pthread_mutex_unlock(&mutex);
		idle_worker[w] = 1;
		sem_post(&workers_busy);
	}else{
		msg = "Thread: Worker ocupado ... Tente novamente mais tarde\n";
		snprintf(sendBuffer, sizeof(sendBuffer), "%.60s\0", msg);
	}
	printf("Thread: Semaforo: %d %d\n", s, workers_count);
	for (int i=0; i<workers_count; i++){
		printf("Thread: idle workers:\n worker %d: %d\n", i, idle_worker[i]);
	}
	send(client->sock_entity, sendBuffer, strlen(sendBuffer)+1, 0);
	 //Verifica se worker disponivel.
	 	//sim: Chama worker
			// retorna resultado
		//no: worker ocupado

	//while(1);




    close(client->sock_entity);
    free(client->entity_addr);
    free(client);
    return NULL;
}




int main(int argc, char* argv[]){
	
	//init
	sem_init(&workers_busy, 0, 0);
	int serv_listen = 0;
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	//
	//
	char recvBuffer[1024];
	char sendBuffer[1024];
	memset(&recvBuffer, 0, sizeof(recvBuffer));
	memset(&sendBuffer, 0, sizeof(sendBuffer));
	//
	struct entity_data *cd;
	int addrlen, n=0;
	//
	pthread_t thread;
	
	
	// ----------- Create Socket and Server Config -----------
	if((serv_listen = socket(AF_INET, SOCK_STREAM, 0))<0){
		perror("Fail create socket");
		return 1;
	}
	
	//config server
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(3365);
	// server_addr.sin_port = htons(3364);
	
	if(bind(serv_listen, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
		perror("Error binding sockets");
		return 1;
	}
	
	// ------------------ Wait Connection -------------------
	int c = 0;
	int opt;
	int workers;
	listen(serv_listen, 10);
	while(1){
		// opt = 0;
		memset(&sendBuffer, 0, sizeof(sendBuffer));
		cd = (struct entity_data *)malloc(sizeof(struct entity_data));
        cd->entity_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
        addrlen = sizeof(struct sockaddr_in);
		printf("Main: %x\n",cd);
		//connect:
		cd->sock_entity = accept(serv_listen, (struct sockaddr*)cd->entity_addr, (socklen_t*)&addrlen);
	// --------------------- Connected ----------------------
	// printf("Main: counter:%d\n",c);
	printf("Main: Socket Connected: %d\n", cd->sock_entity);
	while((n = recv(cd->sock_entity, recvBuffer, sizeof(recvBuffer)-1, 0))>0){
		printf("Main: buffer size:%d\n",n);
		recvBuffer[n] = '\0';
		if(n < 0) perror("fputs");
		else break;
	}
	
	// if (message != "client"){
	// 	printf("Entrou !!!\n");
	// 	// message = "Client Connected!";
	// // 		pthread_create(&thread, NULL, client_handle, (void *)cd);
	// // 		pthread_detach(thread);
	// }

	printf("Main: Buffer msg: %s\n",recvBuffer);
	opt = atoi(recvBuffer);
    char* message = recvBuffer;
	// printf("Main: counter2:%d\n",c);
	switch(opt){
		case 1:
			message = "Server: Client Connected !\n";
			printf("%s", message);
			send(cd->sock_entity, message, strlen(message), 0);
			pthread_create(&thread, NULL, clientHandle, (void *)cd);
			pthread_detach(thread);
		break;
		case 2:
			if( (workers = addWorkerList(cd->sock_entity)) ) message = "Server: Worker Connected !";
			else message = "Server: Max Worker Over Capacity!!";
			printf("%s || Workers:%d\n", message, workers);
		break;
		default:
			printf("Opt Erro !\n");
		break;
	}
	// send(cd->sock_entity, message, strlen(message), 0);
	// printf("Main: counter3:%d\n",c);
	// c++;
	
	
	// -------------------- End Server ---------------------
	}
	sem_destroy(&workers_busy);
	return 0;
}