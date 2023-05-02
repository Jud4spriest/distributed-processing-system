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


// #define PORT 3362
#define PORT 3365


#define MAX_WORKERS 5
int workers_list[MAX_WORKERS];
int idle_worker[MAX_WORKERS] = {1};
int workers_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t workers_busy;

struct client_data{
    int sock_entity;
    struct sockaddr_in *entity_addr;
};

struct worker_data{
	int socket;
	struct sockaddr_in *worker_addr;
	int identifier;
};


int addWorkerList(struct worker_data *wd){
	if (workers_count < MAX_WORKERS){
		workers_list[workers_count] = wd->socket;
		idle_worker[workers_count] = 1;
		wd->identifier = workers_count;
		workers_count++;
		sem_post(&workers_busy);
		return workers_count;
	}
	return 0;
}


float callWorker(int worker, char* str){
	int n;
	float res = 0;
	char recvBuffer[1024];
	memset(recvBuffer, 0, sizeof(recvBuffer));

	printf("Server: Calling Worker...\n");
	// printf("Param buffer %s\n", str);
	printf("Server: Worker from list: %d. Sending operation ..\n", workers_list[worker]);
	send(workers_list[worker], str, strlen(str)+1, 0);
	// printf("Send operation to worker...\n");
	while((n = recv(workers_list[worker], recvBuffer, sizeof(recvBuffer)-1, 0))>0){
		// printf("Worker: buffer size:%d\n",n);
		recvBuffer[n] = '\0';
		if(n < 0) perror("fputs");
		else break;
	}
	res = strtof(recvBuffer, NULL);
	printf("Server: Worker result operation: %.2lf\n",res);
	return res;
}

int verifyIdleWorker(){
	for (int i=0; i < workers_count; i++){
		if(idle_worker[i]){
			
			idle_worker[MAX_WORKERS] = 0;
			//printf("idle worker: now i busy!\n");
			return i;
		}
	}
	return -1;
}

float isAliveWorker(void* wd){
	struct worker_data *worker = (struct worker_data *)wd;
	int n;
 	int workers_on;
	char recvBuffer[1024];
	memset(recvBuffer, 0, sizeof(recvBuffer));
	
	pthread_mutex_lock(&mutex);
	workers_on = addWorkerList(worker);
	
	pthread_mutex_unlock(&mutex);
	if (workers_on){
		printf("Server: Workers On: %d\n",workers_on);
		while(worker->socket){
			printf("Server: Info workers: Socket - %x | Address - %d | Idle - %d\n", worker->socket, worker->worker_addr, worker->identifier);
			sleep(10);
			if((n = recv(worker->socket, recvBuffer, sizeof(recvBuffer)-1, 0))>0){
				break;
			}
		}
	}else{
		printf("Server: Max Worker Over Capacity!!");
	}

	idle_worker[worker->identifier] = 0;
	workers_list[worker->identifier] = 0;
	workers_count--;
	sem_wait(&workers_busy);
	printf("Worker Desconectado!\n");
	return 0;
}


void *clientHandle(void* cd){
    struct client_data *client = (struct client_data *)cd;
    char sendBuffer[1024];
	char recvBuffer[1024];
	int wait_time = rand() % 5 + 1;
    int n;
	int sem;
	int w;
	float res;
	char *msg;

    memset(sendBuffer, 0, sizeof(sendBuffer));
	memset(recvBuffer, 0, sizeof(recvBuffer));

	//thread status no servidor.
    printf("Thread: Received connection from %s:%d\n", inet_ntoa(client->entity_addr->sin_addr), ntohs(client->entity_addr->sin_port));
    fflush(stdout);
	
	
	while((n = recv(client->sock_entity, recvBuffer, sizeof(recvBuffer)-1, 0))>0){
		// printf("Thread: buffer size:%d\n",n);
		recvBuffer[n] = '\0';
		if(n < 0) perror("fputs");
		else break;
	}
	printf("Thread: Buffer msg: %s\n", recvBuffer);
	sleep(3);
	
	if ( (sem = sem_trywait(&workers_busy)) >= 0){
		printf("Thread: Idle Worker! Go to work!\n");
		w = verifyIdleWorker();
		printf("Thread: Worker selected: %d\n",w);
		if(w < 0) perror("This shoudn't occurrs! Unexpected Error!");
		printf("Thread: Buffer: %s\n", recvBuffer);
		res = callWorker(w, recvBuffer);
		// printf("worker list3: %d\n", workers_list[w]);
		printf("Thread: res = %d\n",res);

		msg = "Server: O resultado da operação é: \n";
		snprintf(sendBuffer, sizeof(sendBuffer), "%.40s: %.3f\0", msg, res);
		idle_worker[w] = 1;
		sem_post(&workers_busy);
		// printf("Thread: sem_workers: %d Conectados: %d\n", sem, workers_count);
	}else{
		if(workers_count == 0) printf("Thread: No workers connected!\n");
		msg = "Thread: Worker ocupado ... Tente novamente mais tarde\n";
		snprintf(sendBuffer, sizeof(sendBuffer), "%.60s\0", msg);
	}
	for (int i=0; i<workers_count; i++)
		printf("Thread: Idle workers:\n\t- worker %d: %d\n", i, idle_worker[i]);

	send(client->sock_entity, sendBuffer, strlen(sendBuffer)+1, 0);

    close(client->sock_entity);
    free(client->entity_addr);
    free(client);
    return NULL;
}


// -------------------- Main --------------------------------

int main(int argc, char* argv[]){
	
	//init
	sem_init(&workers_busy, 0, 0);
	int server_listen = 0;
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	//
	//
	char recvBuffer[1024];
	char sendBuffer[1024];
	memset(&recvBuffer, 0, sizeof(recvBuffer));
	memset(&sendBuffer, 0, sizeof(sendBuffer));
	//
	struct client_data *cd;
	struct worker_data *wd;
	int addrlen, n=0;
	//
	pthread_t thread;
	
	
	// ----------- Create Socket and Server Config -----------
	if((server_listen = socket(AF_INET, SOCK_STREAM, 0))<0){
		perror("Fail create socket");
		return 1;
	}
	
	//config server
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);
	
	if(bind(server_listen, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
		perror("Error binding sockets");
		return 1;
	}
	
	// ------------------ Wait Connection -------------------
	int opt;

	listen(server_listen, 10);
	while(1){
		cd = (struct client_data *)malloc(sizeof(struct client_data));
		wd = (struct worker_data *)malloc(sizeof(struct worker_data));
        cd->entity_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
		wd->worker_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
        addrlen = sizeof(struct sockaddr_in);
		printf("Main: %x\n",cd);
		//connect:
		cd->sock_entity = accept(server_listen, (struct sockaddr*)cd->entity_addr, (socklen_t*)&addrlen);

	// --------------------- Connected ----------------------

	printf("Main: Socket Connected: %d\n", cd->sock_entity);
	while((n = recv(cd->sock_entity, recvBuffer, sizeof(recvBuffer)-1, 0))>0){
		// printf("Main: buffer size:%d\n",n);
		recvBuffer[n] = '\0';
		if(n < 0) perror("fputs");
		else break;
	}
	
	// printf("Main: Buffer msg: %s\n",recvBuffer);
	// char* message = recvBuffer;
	char* message;
	opt = atoi(recvBuffer);
	
	//Worker ou cliente ?
	switch(opt){
		case 1: //Responde cliente conectado, e dispara thread.
			message = "Server: Client Connected !\n";
			printf("%s", message);
			send(cd->sock_entity, message, strlen(message), 0); 
			pthread_create(&thread, NULL, clientHandle, (void *)cd);
			pthread_detach(thread);
		break;
		case 2: //Responde worker conectado, e dispara thread.
			wd->socket = cd->sock_entity;
			wd->worker_addr = cd->entity_addr;
			// wd->idle = 1;
			pthread_create(&thread, NULL, isAliveWorker, (void *)wd);
			pthread_detach(thread);

			// if( (workers = addWorkerList(cd->sock_entity)) ){ //colocar isto dentro da thread.
			// 	wd->socket = cd->sock_entity;
			// 	wd->worker_addr = cd->entity_addr;
			// 	wd->idle = 1;
			// 	printf("Server: Worker Connected !\n\t- Workers:%d\n", workers);
			// 	pthread_create(&thread, NULL, isAliveWorker, (void *)wd);
			// 	pthread_detach(thread);
			// } 
			// else message = "Server: Max Worker Over Capacity!!";
			
		break;
		default:
			printf("Opt Erro !\n");
		break;
	}
	
	// -------------------- End Server ---------------------
	}
	sem_destroy(&workers_busy);
	return 0;
}