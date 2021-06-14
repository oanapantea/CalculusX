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
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_MSG 512
#define MAXCHAR 128
#define BUF_SIZE 8192

#define SOCKET_ERROR 1
#define HOST_ERROR 2
#define SERVER_CONN_ERROR 3
#define SEND_FILE_ERROR 4

#define min(m,n) ((m) < (n) ? (m) : (n))

char pathname[255] = "/home/oanap/Documents/ecuatii.txt";
char filename[255] = "ecuatii.txt";

ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
    off_t orig;

    if (offset != NULL) {

        /* Save current file offset and set offset to value in '*offset' */

        orig = lseek(in_fd, 0, SEEK_CUR);
        if (orig == -1)
            return -1;
        if (lseek(in_fd, *offset, SEEK_SET) == -1)
            return -1;
    }

    size_t totSent = 0;

    while (count > 0) {
        size_t toRead = min(BUF_SIZE, count);

        char buf[BUF_SIZE];
        ssize_t numRead = read(in_fd, buf, toRead);
        if (numRead == -1)
            return -1;
        if (numRead == 0)
            break;                      /* EOF */

        ssize_t numSent = write(out_fd, buf, numRead);
        if (numSent == -1)
            return -1;
        count -= numSent;
        totSent += numSent;
    }

    if (offset != NULL) {

        /* Return updated file offset in '*offset', and reset the file offset
           to the value it had when we were called. */

        *offset = lseek(in_fd, 0, SEEK_CUR);
        if (*offset == -1)
            return -1;
        if (lseek(in_fd, orig, SEEK_SET) == -1)
            return -1;
    }

    return totSent;
}

void mySendFunction(int socketFd){
    off_t* sendFileOffset = 0;
    ssize_t fileSize;
    int fileFd;
    struct stat fileStruct;

     //trimit numele fisierului
    send(socketFd, &filename, sizeof(filename), 0);
    fileFd = open(pathname, O_RDONLY); // if filedd !=0 => error, else => not error
    fstat(fileFd, &fileStruct);
    fprintf(stderr, "Fisierul %s are marimea = %ld \n", filename, fileStruct.st_size);

    //trimit marimea fisierului
    send(socketFd, &fileStruct.st_size, sizeof(fileStruct.st_size), 0);

    //sendfile(unde scrii, de unde citesti, peste cati bytes sa sari, cati sa trimiti);
    //pt primire o sa am sendfile(fileFd, socketFd, ...);

    fileSize = sendfile(socketFd, fileFd, sendFileOffset, fileStruct.st_size);
    fprintf(stderr, "Fisierul a fost trimis, avand marimea %ld\n", fileSize);

    close(fileFd);

}

void myRecvFunction(int socketFd){
    off_t* sendFileOffset = 0;
    ssize_t fileSize;
    int fileFd;
    struct stat fileStruct;
    char sentFileName[255];

     //trimit numele fisierului
    recv(socketFd, &sentFileName, sizeof(sentFileName), 0);
    fileFd = open("/home/oanap/Documents/ecuatii1.txt", O_CREAT|O_WRONLY, 0600);
    recv(socketFd, &fileSize, sizeof(fileSize), 0);
    fprintf(stderr, "Fisierul %s are marimea = %ld \n", sentFileName, fileSize);

    //trimit marimea fisierului

    //sendfile(unde scrii, de unde citesti, peste cati bytes sa sari, cati sa trimiti);
    //pt primire o sa am sendfile(fileFd, socketFd, ...);

    fileSize = sendfile(fileFd, socketFd, sendFileOffset, fileSize);
    fprintf(stderr, "Fisierul a fost primit, avand marimea %ld\n", fileSize);
    close(fileFd);
}

int main(int argc, char * argv[])
{
    printf("Acesta este pcd client inet\n\n");

    if (argc < 2)
    {
        printf("\nNumar de argumente insuficiente, trebuiesc 2 (ex. localhost 4043)\n");
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
        exit(SOCKET_ERROR);
    }
    else{
        printf("\nS-a creat client socketul\n");
    }


    //verificare host (localhost) -> daca nu exista
    host = gethostbyname(argv[1]);

    if ( host == (struct hostent*)NULL )
    {
        printf("\nEroare gethostbyname().\n");
        exit(HOST_ERROR);
    }

    // name the socket as agreed with server. ????
    serverAddr.sin_family = AF_INET;

    memcpy( (char*)&serverAddr.sin_addr, (char*)host->h_addr, host->h_length);

    serverAddr.sin_port = htons((u_short)atoi(argv[2]));

    // conectare la server
    result = connect(clientFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    //tratare result
    if(result != 0){
        fprintf(stderr, "\nEroare (%d) la conectarea cu serverul a esuat.\n", errno);
        exit(SERVER_CONN_ERROR);
    }
    else {
        printf("\nConectare reusita. :D \n");
    }

    int nr_ecuatii;
    int i;
    int sendF;
    char ecuatie[255];
    FILE *fileCl;

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
                fileCl = fopen(pathname, "w");
                printf("\nAi ales sa trimiti ecuatia spre rezolvare\n");
                printf("\nCate ecuatii are sistemul? n = ");
                scanf("%d", &nr_ecuatii);
                fprintf(fileCl, "%d\n", nr_ecuatii);
                getchar();
                for(i = 1; i <= nr_ecuatii; i++){
                    printf("\nDati parametrii necunoscutelor (despartite printr-un SPACE) si rezultatul ecuatiei cu numarul %d = ", i);
                    fgets(ecuatie, 255, stdin);
                    fprintf(fileCl, "%s", ecuatie);
                }
                fclose(fileCl);
                //trimite fisierul
                mySendFunction(clientFd);
                fprintf(stderr, "Fisierul a fost trimis cu succes\n");
                myRecvFunction(clientFd);
                fprintf(stderr, "Fisierul a fost primit cu succes\n");
                break;
            case 2:
                printf("\nLa revedere! :) \n");
                exit(0);
                break;
        }
    }

    return 0;
}
