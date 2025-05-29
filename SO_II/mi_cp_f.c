#include "directorios.h"

int main(int argc, char **argv) {
    char *camino_origen, *camino_destino;

    // Comprobar sintaxis
    if (argc != 4) {
        fprintf(stderr, "Sintaxis: ./mi_cp_f <disco> </origen/nombre> </destino/>\n");
        return FALLO;
    }

    // Montar el disco
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, "Error al montar el disco\n");
        return FALLO;
    }

    camino_origen = argv[2];
    camino_destino = argv[3];

    // Comprobar que el origen es un fichero
    if (camino_origen[strlen(camino_origen) - 1] == '/') {
        fprintf(stderr, RED "Error: el origen debe ser un fichero (no terminar en '/')\n" RESET);
        return FALLO;
    }

    // Comprobar que el destino es un directorio
    if (camino_destino[strlen(camino_destino) - 1] != '/') {
        fprintf(stderr, RED "Error: el destino debe ser un directorio (terminar en '/')\n" RESET);
        bumount();
        return FALLO;
    }

    // Llamar a la función de copiar
    if (mi_cp_f(camino_origen, camino_destino) == FALLO) {
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
