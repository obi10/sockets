#include <stdio.h>

#define BUFFERSIZE 100

int main (int argc, char *argv[])
{
    char buffer[BUFFERSIZE];
    fgets(buffer, BUFFERSIZE , stdin);
    printf("Read: %s", buffer);
    return 0;
}
