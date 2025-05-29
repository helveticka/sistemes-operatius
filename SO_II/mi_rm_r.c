#include "directorios.h"

int main(int argc, char **argv) {
    char *camino;

    // Comprobar sintaxis
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_rm_r  <disco> </ruta/directorio/>\n");
        return FALLO;
    }

    // Montar el disco
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, "Error al montar el disco\n");
        return FALLO;
    }

    camino = argv[2];

    // Comprobar que el destino es un directorio
    if (camino[strlen(camino) - 1] != '/') {
        fprintf(stderr, RED "Error: el destino debe ser un directorio (terminar en '/')\n" RESET);
        bumount();
        return FALLO;
    }

    // Llamar a la función de copiar
    if (mi_rm_r(camino) == FALLO) {
        fprintf(stderr, "Error: no se pudo copiar el fichero\n");
        bumount();
        return FALLO;
    }

    // Desmontar el disco
    if(bumount() == FALLO) {
        fprintf(stderr, "Error al desmontar el disco\n");
        return FALLO;
    }
    return EXITO;
}