#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "directorios.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, RED "Sintaxis: %s <disco> </ruta>\n" RESET, argv[0]);
        return EXIT_FAILURE;
    }

    // Montamos el dispositivo
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, "Error: No se pudo montar el disco.\n");
        return EXIT_FAILURE;
    }

    struct STAT stat;
    int resultado = mi_stat(argv[2], &stat);
    if (resultado < 0) {
        mostrar_error_buscar_entrada(resultado);
        bumount();
        return EXIT_FAILURE;
    }

    // Mostramos la información del inodo
    printf("Tipo: %c\n", stat.tipo);
    printf("Permisos: %d\n", stat.permisos);
    printf("atime: %s", ctime(&stat.atime));
    printf("mtime: %s", ctime(&stat.mtime));
    printf("ctime: %s", ctime(&stat.ctime));
    printf("nlinks: %d\n", stat.nlinks);
    printf("Tamaño en bytes lógicos: %d\n", stat.tamEnBytesLog);
    printf("Bloques ocupados: %d\n", stat.numBloquesOcupados);

    bumount();
    return EXIT_SUCCESS;
}
