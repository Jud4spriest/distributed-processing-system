
all:
	gcc -o server server.c -Wall -lpthread
	gcc -o client client.c -Wall  
	gcc -o worker worker.c -Wall

clean:
	rm -f server client worker


