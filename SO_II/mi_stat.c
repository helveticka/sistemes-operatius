/**
 * @file mi_stat.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "directorios.h"
/**
 * @brief Función principal que muestra el estado de un fichero o directorio
 * @param argc Cantidad de argumentos
 * @param argv Argumentos
 * @return EXITO si no hay errores, FALLO en caso contrario
 */
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
    printf("tipo: %c\n", stat.tipo);
    printf("permisos: %d\n", stat.permisos);
    printf("atime: %s", ctime(&stat.atime));
    printf("mtime: %s", ctime(&stat.mtime));
    printf("ctime: %s", ctime(&stat.ctime));
    printf("btime: %s", ctime(&stat.btime));
    printf("nlinks: %d\n", stat.nlinks);
    printf("tamEnBytesLog: %d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", stat.numBloquesOcupados);

    bumount();
    return EXIT_SUCCESS;
}
