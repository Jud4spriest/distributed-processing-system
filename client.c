#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

//menset(&x,y,sizeof(x)): Seta o valor de x para y.

int main(int argc, char *argv[]){
	
	int n = 0;
	//init sockets
	int sockserver = 0;
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	
	//init buffer
	char recvBuffer[1024];
	char sendBuffer[1024];
	memset(recvBuffer, 0, sizeof(recvBuffer));
	memset(sendBuffer, 0, sizeof(sendBuffer));
	
	//argumentos terminal
	if(argc < 2){
		printf("\n Como usar: %s <ip do servidor> \n",argv[0]);
        return 1;
	}
	
	//cria socket
	if((sockserver = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Error creating socket");
		return 1;
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(3365);
	// server_addr.sin_port = htons(3364);
	
	
	//connect servidor
	if(connect(sockserver, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		perror("Error estabilish connection");
		return 1;
	}
	// -------------------- Conectado ------------------------
	

	// char op[30];
	char op[30];
	float num1;
	float num2;
	// char *op;
	// const char* code = "client";
	// const char* format = "%.20s\r\0";
	const char* code = "1";
	const char* format = "%.30s\0";
	snprintf(sendBuffer, sizeof(sendBuffer), format, code);
	// printf("Msg: %s\n", code); //debug line
	send(sockserver, sendBuffer, strlen(sendBuffer)+1, 0); //envia identificação de cliente.
	
	while((n = recv(sockserver, recvBuffer, sizeof(recvBuffer)-1, 0))>0){ //recebe Ok do servidor.
		// printf("%d",n);
		recvBuffer[n] = '\0';
		if(fputs(recvBuffer, stdout) == EOF) perror("fputs");
		else break;
	} 
	printf("Digite a operação que deseja realizar no formato: \"operação\" \"num1\" \"num2\"\n");
	printf("Operações possíveis:\n1:\t\"add\" \t\t- Adição\n2:\t\"subtract\" \t- Subtração\n3:\t\"multiply\" \t- Multiplicação\n4:\t\"divide\" \t- Divisão\n");
	printf("Comando: ");
	
	if (argc < 4){
		char op[30];
		scanf("%s %.3f %.3f", op, &num1, &num2);
	}else{
		char *op = argv[2];
		num1 = strtof(argv[3], NULL);
		num2 = strtof(argv[4], NULL);
	}
	printf("%s %.3f %.3f", op, num1, num2);

	snprintf(sendBuffer, sizeof(sendBuffer), "%.12s %.2f %.2f\0", op, &num1, &num2);
	send(sockserver, sendBuffer, strlen(sendBuffer)+1, 0); //envia operação pro servidor.
	
	printf("\nAguardando Resultado....\n");
	// memset(recvBuffer, 0, sizeof(recvBuffer));
	while((n = recv(sockserver, recvBuffer, sizeof(recvBuffer)-1, 0))>0){
		// printf("O que ta sendo retornado aki é %s : %d",recvBuffer,n);
		recvBuffer[n] = '\0';
		if(n < 0) perror("fputs");
		else if (n == 0) perror("Conexão fechada pelo servidor!");
		else break;
	}

	printf("%s\n", recvBuffer);
	
	
	
	
	
	// -------------------- End connect ----------------------
	return 0;
}