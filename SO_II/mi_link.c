#include "directorios.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Sintaxis: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n");
        return FALLO;
    }

    // Montar el sistema de ficheros
    if (bmount(argv[1]) == FALLO) {
        perror("Error en bmount");
        return FALLO;
    }

    // Llamar a mi_link (de la capa de directorios)
    if (mi_link(argv[2], argv[3]) < 0) {
        fprintf(stderr, "Error: No se pudo crear el enlace.\n");
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
