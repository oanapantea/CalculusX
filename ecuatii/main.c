#include <stdio.h>
#include <stdlib.h>
#include <string.h>
  char userFile[512] = "../Documents/ecuatiiRecv";

  void function1(){

    char *valueC = (char*) calloc(10, sizeof(char));

    int val = 2;

    strcpy(userFile, "../Documents/ecuatiiRecv");

    snprintf(valueC, 12, "%d", 2);

    printf("%s\n", valueC);

    strcat(userFile, valueC);
    printf("%s\n", userFile);

    strcat(userFile, ".txt");
    printf("%s\n", userFile);

  }
int main()
{
    function1();
    printf("\n%s\n", userFile);

    return 0;
}
