#include "directorios.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    char *camino1, *camino2;

    if (argc != 4) {
        fprintf(stderr, "Sintaxis: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n");
        return FALLO;
    }

    // Montar el sistema de ficheros
    if (bmount(argv[1]) == FALLO) {
        perror("Error en bmount");
        return FALLO;
    }


    camino1 = argv[2];
    camino2 = argv[3];

    // Llamar a mi_link (de la capa de directorios)
    if (mi_link(camino1, camino2) < 0) {
        bumount();
        return FALLO;
    }

    // Desmontar
    if (bumount() == FALLO) {
        perror("Error en bumount");
        return FALLO;
    }

    return EXITO;
}
