/**
 * @file mi_link.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"
#include <stdio.h>
#include <string.h>
/**
 * @brief Función principal que crea un enlace a un fichero
 * @param argc Cantidad de argumentos
 * @param argv Argumentos
 * @return EXITO si no hay errores, FALLO en caso contrario
 */
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
