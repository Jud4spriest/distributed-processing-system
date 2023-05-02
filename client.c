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

// #define PORT 3362
#define PORT 3365



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
		printf("\n Como usar: %s <ip do servidor> \nOu para execução em linhas de comando, após o ip coloque \'Operacao\' \'num1\' \'num2'\n",argv[0]);
        return 1;
	}
	
	//cria socket
	if((sockserver = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Error creating socket");
		return 1;
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	
	
	//connect servidor
	if(connect(sockserver, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		perror("Error estabilish connection");
		return 1;
	}
	// -------------------- Conectado ------------------------

	float num1;
	float num2;
	const char* code = "1";
	const char* format = "%.30s\0";
	snprintf(sendBuffer, sizeof(sendBuffer), format, code);
	send(sockserver, sendBuffer, strlen(sendBuffer)+1, 0);  //envia identificação de cliente.
	
	//recebe Ok do servidor.
	while((n = recv(sockserver, recvBuffer, sizeof(recvBuffer)-1, 0))>0){ 
		recvBuffer[n] = '\0';
		if(fputs(recvBuffer, stdout) == EOF) perror("fputs");
		else break;
	} 

	//Recebe operação por linha de comando ou digitado pelo usuario e envia operacao para o server.
	if (argc < 4){ 
		printf("Digite a operação que deseja realizar no formato: \"operação\" \"num1\" \"num2\"\n");
		printf("Operações possíveis:\n1:\t\"add\" \t\t- Adição\n2:\t\"subtract\" \t- Subtração\n3:\t\"multiply\" \t- Multiplicação\n4:\t\"divide\" \t- Divisão\n");
		printf("Comando: ");
		char op1[30];
		scanf("%s %2f %2f", op1, &num1, &num2);
		printf("%s %.3f %.3f", op1, num1, num2);
		snprintf(sendBuffer, sizeof(sendBuffer), "%.12s %.2f %.2f\0", op1, &num1, &num2);
		send(sockserver, sendBuffer, strlen(sendBuffer)+1, 0); 
	}else{
		const char *op2 = argv[2];
		num1 = strtof(argv[3], NULL);
		num2 = strtof(argv[4], NULL);
		printf("%s %.3f %.3f", op2, num1, num2);
		snprintf(sendBuffer, sizeof(sendBuffer), "%.12s %.2f %.2f\0", op2, &num1, &num2);
		send(sockserver, sendBuffer, strlen(sendBuffer)+1, 0); 
	}

	//retorno dos dados
	printf("\nClient: Aguardando Resultado...\n"); 
	while((n = recv(sockserver, recvBuffer, sizeof(recvBuffer)-1, 0))>0){
		recvBuffer[n] = '\0';
		if(n < 0) perror("fputs");
		else if (n == 0) perror("Conexão fechada pelo servidor!");
		else break;
	}
	printf("Client: Resultado = %s\n", recvBuffer);
	
	// -------------------- End connect ----------------------
	return 0;
}