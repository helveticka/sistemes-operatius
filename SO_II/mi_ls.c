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
    printf("debug 1: mi_ls()\n");
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Sintaxis: ./mi_ls [-l] <disco> </ruta>\n");
        return FALLO;
    }
    printf("debug 2: mi_ls()\n");
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
    printf("debug 3: mi_ls()\n");
    // Determinar si la ruta corresponde a un directorio o fichero
    tipo = (camino[strlen(camino) - 1] == '/') ? 'd' : 'f';
    printf("debug 4: mi_ls()\n");
    // Montar el disco
    if (bmount(disco) == -1) {
        fprintf(stderr, "Error: No se pudo montar el disco.\n");
        return FALLO;
    }
    printf("debug 5: mi_ls()\n");
    // Limpiar el buffer
    memset(buffer, 0, TAMBUFFER);
    printf("debug 6: mi_ls()\n");
    // Llamar a mi_dir para listar el contenido del directorio o mostrar información sobre un fichero
    nentradas = mi_dir(camino, buffer, tipo, flag);
    printf("debug 7: mi_ls()\n");
    // Comprobar errores de mi_dir
    if (nentradas < 0) {
        fprintf(stderr, "Error al listar contenido: %d\n", nentradas);
    } else {
        // Solo imprimimos el buffer que contiene la salida generada por mi_dir
        printf("%s\n", buffer);
    }
    printf("debug 8: mi_ls()\n");
    // Desmontar el disco
    bumount();

    return EXITO;
}
