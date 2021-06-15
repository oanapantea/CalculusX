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
#include <fcntl.h>
#include <sys/stat.h>

#define INET_PORT 4043
#define UNIX_PORT 4044

#define SOCKET_ERROR 1
#define BIND_ERROR 2
#define LISTEN_ERROR 3

//Fisierul prin care serverul si clientul de administrare vor comunica:
#define UNIX_FILE "/tmp/unixFile"

#define BUF_SIZE 8192
#define MAX_CLIENTS 30
#define MAX_BLOCKED 30

#define min(m,n) ((m) < (n) ? (m) : (n))

char pathname[255] = "/home/oanap/Documents/ecuatiiRecv.txt";
char filename[255] = "ecuatiiRecv.txt";

typedef struct
{
    int fd; // identificator de socket din interiorul programului
    char IP[16];
    unsigned short port;

} inetClient;

inetClient clientList[30]; //lista clientilor conectati
int mClient; // the last client

FILE * blockedCFile; //file cu lista clientilor blocati

char blockedIpC[MAX_CLIENTS][16];  //lista de clienti blocati
int totalBlocked = 0;

// functie procesare comanda client administrare UNIX

int processClientRequest(int numChars, char * buffer, char * msg)
{
    int cmdType = buffer[0];
    int numConnected = 0;
    printf("\n UNIX = comanda primita = %s, type: %c.\n", buffer, cmdType);
    bzero(msg, strlen(msg));  //msg este umplut cu strlen(msg) de zero
    char temp[8];
    int i, theFd, found;

    switch(cmdType)
    {
    case '1': //lista fd-uri clientilor
        strcpy(msg,"1");//seteaza tipul mesajului
        for (i = 0; i < mClient; i++)
        {
            if(clientList[i].fd > 0)
            {
                strcat(msg, ":");
                snprintf(temp, 3, "%d", clientList[i].fd);
                strcat(msg, temp);
                numConnected++;
            }
        }
        if(numConnected == 0)
            strcat(msg, " No INET client is connected.");
        break;
    case '2': //afisare informatii despre client
        strcpy(msg,"2");
        //gaseste clientul cu fd-ul cerut, afiseaza info
        theFd = atoi(buffer + 2);
        for(i=0; i<mClient; i++)
        {
            if(clientList[i].fd == theFd)
            {
                strcat(msg, clientList[i].IP);
                strcat(msg, ":");
                snprintf(temp, 5, "%d", clientList[i].port);
                strcat(msg, temp);
                break;
            }
        }
        break;
    case '3': //deconectare client
        strcpy(msg, "3");
        theFd = atoi(buffer + 2);
        found = 0;
        for (i = 0; i<mClient ; i++)
        {
            if (clientList[i].fd == theFd)
            {
                close(clientList[i].fd);
                found = 1;
                break;
            }
        }
        if(found)
            strcat(msg, ": client disconnected.");
        else
            strcat(msg, ": client not found.");
        break;

    default:
        //afisare mesaj eroare
        printf("Command is invalid, it will be ignored..\n");
        break;
    }
    printf("\nMessage for UNIX client: %s", msg);
    return strlen(msg);

    return 0;
}

void *unix_main(void *args)
{

    printf("UNIX server is running\n");

    int serverFd = -1, clientFd = -2;
    int error, nBytes, numSent;
    char cmdClient[512]; // comanda de la client
    char mesaj[512]; // mesaj catre client
    struct sockaddr_un serverAddr;

    // creeaza socket cu protocolul TCP
    serverFd = socket(AF_UNIX, SOCK_STREAM, 0);

    if(serverFd < 0)
    {
        perror("UNIX socket() error..");
    }

    memset(&serverAddr, 0, sizeof(serverAddr)); //initialize serverAddr
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, UNIX_FILE);

    //cu bind facem legatura dintre serverAddr si socket(serverFd)
    error = bind(serverFd, (struct sockaddr *) &serverAddr, SUN_LEN(&serverAddr));

    if(error < 0)
    {
        perror("UNIX bind() error..");
    }

    error = listen(serverFd, 10); // convertim socket-ul in modul ascultare, pot sa aibe maximum 10 conexiuni

    if(error < 0)
    {
        perror("UNIX listen() error");
    }
    int tries = 10;
    while(tries)
    {
        //ascultam cererile de conectare doar daca clientFd < 0
        if(clientFd < 0)
        {
            if((clientFd = accept(serverFd, NULL, NULL)) == -1)  // accept returneaza urmatoarea conexiune existenta
            {
                perror("UNIX accept() error\n");
                break;
            }
        }
        else   //primeste comenzi de la administrator
        {
            while((nBytes = read(clientFd, cmdClient, sizeof(cmdClient))) > 0)  // citeste mesajul de la client si il pune in buffer-ul cmd client
            {
                cmdClient[nBytes] = 0;
                printf("UNIX = read %d bytes: %s\n", nBytes, cmdClient);  // buffer-ul cmdClient continte acum mesajul de la admin

                int len = processClientRequest(nBytes, cmdClient, mesaj);

                numSent = send(clientFd, mesaj, len, 0); // trimite mesajul inapoi la client
                if( numSent < 0)
                {
                    perror("UNIX send() error.");
                    break;
                }
            }
            if(nBytes < 0)
            {
                perror("UNIX client read error, the connection will be closed.\n");
                close(clientFd);
                clientFd = -2;
                continue;
            }
            else if( nBytes == 0)
            {
                printf("UNIX ** The client closed the connection.\n");
                close(clientFd);
                clientFd = -2;
                continue;
            }
        }
        --tries;
    }

    if(serverFd > 0)
        close(serverFd);

    unlink(UNIX_FILE); //fisierul de comunicare este inchis
}


ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
    off_t orig;

    if (offset != NULL)
    {

        /* Save current file offset and set offset to value in '*offset' */

        orig = lseek(in_fd, 0, SEEK_CUR);
        if (orig == -1)
            return -1;
        if (lseek(in_fd, *offset, SEEK_SET) == -1)
            return -1;
    }

    size_t totSent = 0;

    while (count > 0)
    {
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

    if (offset != NULL)
    {

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

void mySendFunction(int socketFd)
{
    off_t* sendFileOffset = 0;
    ssize_t fileSize;
    int fileFd;
    struct stat fileStruct;

    //trimit numele fisierului
    send(socketFd, &filename, sizeof(filename), 0);
    fileFd = open(pathname, O_RDONLY);
    fstat(fileFd, &fileStruct);
    fprintf(stderr,"Fisierul %s are marimea = %ld ", filename, fileStruct.st_size);

    //trimit marimea fisierului
    send(socketFd, &fileStruct.st_size, sizeof(fileStruct.st_size), 0);

    //sendfile(unde scrii, de unde citesti, peste cati bytes sa sari, cati sa trimiti);
    //pt primire o sa am sendfile(fileFd, socketFd, ...);

    fileSize = sendfile(socketFd, fileFd, sendFileOffset, fileStruct.st_size);
    fprintf(stderr, "Fisierul a fost trimis, avand marimea %ld", fileSize);

    close(fileFd);

}

float A[20][20], x[10];
int n;

void resolveEcuation(int socketFd)
{

    int i,j,k;
    float c,sum=0.0;

    for(j=1; j<=n; j++) /* loop for the generation of upper triangular matrix*/
    {
        for(i=1; i<=n; i++)
        {
            if(i>j)
            {
                c=A[i][j]/A[j][j];
                for(k=1; k<=n+1; k++)
                {
                    A[i][k]=A[i][k]-c*A[j][k];
                }
            }
        }
    }
    x[n]=A[n][n+1]/A[n][n];
    /* this loop is for backward substitution*/
    for(i=n-1; i>=1; i--)
    {
        sum=0;
        for(j=i+1; j<=n; j++)
        {
            sum=sum+A[i][j]*x[j];
        }
        x[i]=(A[i][n+1]-sum)/A[i][i];
    }

    FILE *fileServ;
    fileServ = fopen(pathname, "w");

    fprintf(fileServ, "Solutia este: \n");
    for(i=1; i<=n; i++)
    {
        fprintf(fileServ, "\nx%d=%f\t",i,x[i]); /* x1, x2, x3... are the required solutions*/
    }
    fclose(fileServ);

    mySendFunction(socketFd);
}

void ecuation(int socketFd)
{

    int i, j, k, aux;
    char delim[] = " ";
    int B[100];
    int m = 0;

    char* n1;
    char line[80] = {0};
    FILE *file = fopen(pathname, "r");
    n1 = fgets(line, 80, file);
    n = atoi(n1);
    printf("%d", n);

    for(i=1; i<=n; i++)
    {
        n1 = fgets(line, 100, file);
        char *ptr = strtok(n1, delim);
        while(ptr != NULL)
        {
            //transform din char in int si il pun in vectorul aux B
            aux = atoi(ptr);
            B[++m] = aux;
            //trec la urmatorul nr
            ptr = strtok(NULL, delim);
        }
    }
    m = 0;
    for(i = 1; i <= n; i++)
    {
        for(j = 1; j <= n+1; j++)
        {
            A[i][j] = B[++m];
        }
    }
    fclose(file);

    resolveEcuation(socketFd);
}

void myRecvFunction(int socketFd)
{
    off_t* sendFileOffset = 0;
    ssize_t fileSize;
    int fileFd;
    struct stat fileStruct;
    char sentFileName[255];

    //trimit numele fisierului
    recv(socketFd, &sentFileName, sizeof(sentFileName), 0);
    fileFd = open(pathname, O_CREAT|O_WRONLY, 0600);
    recv(socketFd, &fileSize, sizeof(fileSize), 0);
    printf("Fisierul %s are marimea = %ld ", sentFileName, fileSize);

    //trimit marimea fisierului

    //sendfile(unde scrii, de unde citesti, peste cati bytes sa sari, cati sa trimiti);
    //pt primire o sa am sendfile(fileFd, socketFd, ...);

    fileSize = sendfile(fileFd, socketFd, sendFileOffset, fileSize);
    printf("Fisierul a fost primit, avand marimea %ld", fileSize);
    close(fileFd);
}

void *inet_main(void *args)
{

    printf("Acesta este pcd server\n\n");

    int serverFd;
    int clientFd;
    int fdReady;
    int maxFd;
    int result;
    int recvResult;

    int i;

    unsigned int addr_size;

    fd_set readSet; //o multime de fd pentru a citi de la clienti

    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;

    char* clientIP;

    //creare socket server
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverFd < 0)
    {
        fprintf(stderr, "\nEroare socket() = %d.\n", errno);
        exit(SOCKET_ERROR);
    }
    else
    {
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
    if(result != 0)
    {
        printf("\nEroare la bind().\n");
        exit(BIND_ERROR);
    }
    else
    {
        printf("\nServer pornit pe port = %d\n", INET_PORT);
    }

    //Serverul trece pe listen
    //int listen(int sockfd, int backlog);
    // backlog -> max de listenuri pe care le poate avea simultan
    if(listen(serverFd, 15) == 0)
    {
        printf("\nListening.\n");
    }
    else
    {
        fprintf(stderr, "Eroare la listen = %d", errno);
        exit(LISTEN_ERROR);
    }

    //lista cu clienti e initializata/per sesiune

    for(i = 0; i < MAX_CLIENTS; i++)
    {
        clientList[i].fd = 0;
    }


    FD_ZERO(&readSet); // clear socket set

    //void FD_SET(int fd, fd_set *set);
    FD_SET(serverFd, &readSet); //seteaza serverFd cu readSet care are bitii pe 0(initializare)
    maxFd = serverFd;
    addr_size = sizeof(clientAddr);

    while(1)
    {
        printf("\nServerul se poate conecta cu clientii\n");
        fdReady = select(maxFd + 1, &readSet, NULL, NULL, NULL);

        // FD_ISSET() tests to see if a file descriptor is part of the set; this is useful after select() returns.
        if(FD_ISSET(serverFd, &readSet))
        {
            //conexiune cu client

            // accept -> accept a connection on a socket
            clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &addr_size);
            //clientul face request si serverul da accept, prin accept pune informatia clientului in clientAddr

            //char *inet_ntoa(struct in_addr in);
            //function converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation.
            clientIP = inet_ntoa(clientAddr.sin_addr);
            printf("Conexiune reusita client inet; fd = %d, IP = %s\n", clientFd, clientIP);

            if(1)
            {

                //se adauga clientul la lista de clienti
                for(i = 0; i < MAX_CLIENTS; i++)
                {
                    if(clientList[i].fd == 0)
                    {
                        clientList[i].fd = clientFd;
                        strcpy(clientList[i].IP, clientIP);
                        clientList[i].port = clientAddr.sin_port;
                        printf("%d   %s    %d\n", clientList[i].fd, clientList[i].IP, clientList[i].port);

                        if(i >= mClient)
                        {
                            mClient = i+1;
                        }

                        break;
                    }

                }

                if(i == MAX_CLIENTS)
                {
                    printf("Numarul de clienti a atins limita maxima de %d", MAX_CLIENTS);
                    exit(1);
                }



                //pune noul clientFd in setul de readSet
                FD_SET(clientFd, &readSet);

                if(clientFd > maxFd)
                    maxFd = clientFd;

            }
            if(--fdReady <=0)
                continue; //se intoarce la select
        }

        //daca client-ul s-a conectat atunci realizam rezervarea
        for(i = 0; i < MAX_CLIENTS; i++)
        {
            if(FD_ISSET(clientList[i].fd, &readSet))
            {
                myRecvFunction(clientList[i].fd);
                ecuation(clientList[i].fd);
            }
        }
    }
}


int main()
{
    int inetPort;
    int unixPort;

    pthread_t inetThread;
    pthread_t unixThread;


    inetPort = INET_PORT; // portul pt inet
    unixPort = UNIX_PORT; // portul pt admin

    //int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg);

    pthread_create(&inetThread, NULL, inet_main, &inetPort);
    pthread_create(&unixThread, NULL, unix_main, &unixPort);

    pthread_join(inetThread, NULL);
    pthread_join(unixThread, NULL);

    return 0;
}
