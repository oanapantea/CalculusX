//pcd project - server

#include <sys/un.h>
#include <sys/socket.h>
#include <stddef.h>
#include <errno.h>
#include <pthread.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#define INET_PORT 4043
#define SOCKET_ERROR 1
#define BIND_ERROR 2
#define LISTEN_ERROR 3

typedef struct
{
    int fd; // identificator de socket din interiorul programului
    char IP[16];
    unsigned short port;

} inetClient;


void *inet_main(void *args){

    printf("Acesta este pcd server\n\n");

    int serverFd;
    int clientFd;
    int fdReady;
    int maxFd;
    int result;

    unsigned int addr_size;

    fd_set readSet; //o multime de fd pentru a citi de la clienti

    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;

    char* clientIP;

    //creare socket server
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverFd < 0){
        fprintf(stderr, "\nEroare socket() = %d.\n", errno);
        exit(SOCKET_ERROR);
    }
    else{
        printf("\nS-a creat server socketul\n");
    }

    //memset -> seteaza o bucata de memorie pe ceva; "\0" pentru ca vrem sa fie goala bucata aceea de memorie
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(INET_PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //deschidem serverul
    result = bind(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    //tratam result ul
    if(result != 0){
        printf("\nEroare la bind().\n");
        exit(BIND_ERROR);
    }
    else{
        printf("\nServer pornit pe port = %d\n", INET_PORT);
    }

    //Serverul trece pe listen
    //int listen(int sockfd, int backlog);
    // backlog -> max de listenuri pe care le poate avea simultan
    if(listen(serverFd, 15) == 0){
        printf("\nListening.\n");
    }
    else{
        fprintf(stderr, "Eroare la listen = %d", errno);
        exit(LISTEN_ERROR);
    }

    //???????????????????
    FD_ZERO(&readSet); // creeaza un set gol de fd uri

    //void FD_SET(int fd, fd_set *set);
    FD_SET(serverFd, &readSet); //seteaza serverFd cu readSet care are bitii pe 0(initializare)
    maxFd = serverFd + 1;
    addr_size = sizeof(clientAddr);
    //????????????????????

    while(1){
        printf("\nServerul se poate conecta cu clientii\n");
        fdReady = select(maxFd, &readSet, NULL, NULL, NULL);

        // FD_ISSET() tests to see if a file descriptor is part of the set; this is useful after select() returns.
        if(FD_ISSET(serverFd, &readSet)){
            //conexiune cu client

            // accept -> accept a connection on a socket
            clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &addr_size);
            //clientul face request si serverul da accept, prin accept pune informatia clientului in clientAddr

            //char *inet_ntoa(struct in_addr in);
            //function converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation.
            clientIP = inet_ntoa(clientAddr.sin_addr);

            printf("Conexiune reusita client inet; fd = %d, IP = %s\n", clientFd, clientIP);
            sleep(4);
        }

    }
}



int main()
{
    int inetPort;

    pthread_t inetThread;

    inetPort = INET_PORT; // portul pt inet

    //int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg);

    pthread_create(&inetThread, NULL, inet_main, &inetPort);

    pthread_join(inetThread, NULL);

    return 0;
}
