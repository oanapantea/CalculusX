#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char path[255] = "/home/oanap/Documents/";

    printf("%s\n", path);

    char pathAux[255] = "\0";

    strcpy(pathAux, path);
    printf("%s\n", pathAux);

    char userFile[255] = "ecuatiiRecv";
    char valueC = 2 + "0";
    strcat(userFile, valueC);
    printf("%s\n", userFile);

    strcat(userFile, ".txt");
    printf("%s\n", userFile);

    return 0;
}
