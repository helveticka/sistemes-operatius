#include "directorios.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    char buffer[TAMBUFFER];
    int nentradas;
    char flag = 0;
    char tipo;
    char *camino;
    char *disco;

    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Sintaxis: ./mi_ls [-l] <disco> </ruta>\n");
        return FALLO;
    }

    if (argc == 4) {  // Con opción -l
        if (strcmp(argv[1], "-l") != 0) {
            fprintf(stderr, "Opción no válida: %s\n", argv[1]);
            return FALLO;
        }
        flag = 1;  // 'l' indica el formato extendido
        disco = argv[2];
        camino = argv[3];
    } else {  // Sin -l
        disco = argv[1];
        camino = argv[2];
    }

    // Determinar si la ruta corresponde a un directorio o fichero
    tipo = (camino[strlen(camino) - 1] == '/') ? 'd' : 'f';

    // Montar el disco
    if (bmount(disco) == -1) {
        fprintf(stderr, "Error: No se pudo montar el disco.\n");
        return FALLO;
    }

    // Limpiar el buffer
    memset(buffer, 0, sizeof(buffer));

    // Llamar a mi_dir para listar el contenido del directorio o mostrar información sobre un fichero
    nentradas = mi_dir(camino, buffer, tipo, flag);

    // Comprobar errores de mi_dir
    if (nentradas < 0) {
        fprintf(stderr, "Error al listar contenido: %d\n", nentradas);
    } else {
        // Solo imprimimos el buffer que contiene la salida generada por mi_dir
        printf("%s\n", buffer);
    }

    // Desmontar el disco
    bumount();

    return EXITO;
}
