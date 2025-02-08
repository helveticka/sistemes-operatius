//contador.c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main (int argc, char **argv) {
    if (!argv[1]) {
        fprintf(stderr, "Error de sintaxis. Uso: contador <cantidad_segundos>\n");
        exit(-1);
    }
    for (int i=0; i<atoi(argv[1]); i++) {
        fprintf(stderr, "%d\n",i);
        sleep(1);
    }
    return 0;
}
