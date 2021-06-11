//pcd project - inet client


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MAX_MSG 512

#define MAXCHAR 128


int main(int argc, char * argv[])
{
    printf("Acesta este pcd client inet\n\n");

    if (argc < 3)
    {
        printf("\nNumar de argumente insuficiente, trebuiesc 3 (ex. -localhost 4043 Salut)\n");
        exit(1);
    }

    int clientFd;
    int result;
    int len;
    int optiune;

    struct hostent *host;
    struct sockaddr_in serverAddr; //tine socket address, informatii despre socket

    //creare socket client
    //int socket(int domain, int type, int protocol); returns a file descriptor that refers to that endpoint
    //AF_INET      IPv4 Internet protocols

    clientFd = socket(AF_INET, SOCK_STREAM, 0); //protocolul 0 il face pe SOCK_STREAM sa foloseasca default

    if(clientFd < 0){
        fprintf(stderr, "\nEroare socket() = %d.\n", errno);
        exit(1);
    }
    else{
        printf("\nS-a creat client socketul\n");
    }


    //verificare host (localhost) -> daca nu exista
    host = gethostbyname(argv[1]);

    if ( host == (struct hostent*)NULL )
    {
        printf("\nEroare gethostbyname().\n");
        exit(1);
    }

    // name the socket as agreed with server. ????
    serverAddr.sin_family = AF_INET;

    memcpy( (char*)&serverAddr.sin_addr, (char*)host->h_addr, host->h_length);

    serverAddr.sin_port = htons((u_short)atoi(argv[2]));

    // conectare la server
    result = connect(clientFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    //tratare result
    if(result != 0){
        fprintf(stderr, "\nEroare %d, conectarea cu serverul a esuat.\n", errno);
        exit(1);
    }
    else {
        printf("\nConectare reusita. :D \n");
    }

    int nr_ecuatii;

    //afisare meniu pt client
    while(1){
        printf("\nMeniu CalculusX :) \n");
        printf("\n1. Trimite ecuatia spre rezolvare\n");
        printf("\n2. Deconecteaza-te\n");
        printf("\nOptiunea ta (1/2): ");
        scanf("%d", &optiune);

        //tratam optiunea
        while(optiune < 0 || optiune > 2){
            printf("\nAlege intre 1 si 2 :) \n");
            scanf("%d", &optiune);
        }

        //procesam optiunea de la client
        switch(optiune){
            case 1:
                printf("\nAi ales sa trimiti ecuatia spre rezolvare\n");
                printf("\nCate ecuatii are sistemul? n = ");
                scanf("%d", &nr_ecuatii);
                break;
            case 2:
                printf("\nLa revedere! :) \n");
                exit(0);
                break;
        }

        //avem nevoie de o variabila care trimite fisierul
        sendMsg = send(clientFd, )
    }


    return 0;
}
