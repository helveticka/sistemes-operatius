/**
 * @file mi_ls.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * @brief Función principal que lista el contenido de un directorio
 * @param argc Cantidad de argumentos
 * @param argv Argumentos
 * @return EXITO si no hay errores, FALLO en caso contrario
 */
int main(int argc, char **argv) {
    char buffer[TAMBUFFER];
    int nentradas;
    char flag = 0;
    char tipo;
    char *camino;
    char *disco;
    if (argc < 3 || argc > 4) {
        fprintf(stderr, RED "Sintaxis: ./mi_ls [-l] <disco> </ruta>\n");
        return FALLO;
    }
    if (argc == 4) {  // Con opción -l
        if (strcmp(argv[1], "-l") != 0) {
            fprintf(stderr, "Opción no válida: %s\n", argv[1]);
            return FALLO;
        }
        flag = 'l';  // 'l' indica el formato extendido
        disco = argv[2];
        camino = argv[3];
    } else{
        disco = argv[1];
        camino = argv[2];
    }
    // Determinar si la ruta corresponde a un directorio o fichero
    tipo = (camino[strlen(camino) - 1] == '/') ? 'd' : 'f';
    // Montar el disco
    if (bmount(disco) == FALLO) {
        fprintf(stderr, "Error en mi_ls: No se pudo montar el disco.\n");
        return FALLO;
    }
    // Limpiar el buffer
    memset(buffer, 0, TAMBUFFER);
    // Comprobar si el camino es válido
    if ((nentradas = mi_dir(camino, buffer, tipo, flag)) < 0) {
        mostrar_error_buscar_entrada(nentradas);
        if(nentradas == FALLO){
            bumount();
            return FALLO;
        }
    } else {
        // Mostrar el contenido del directorio
        printf("%s\n", buffer);
    }
    // Desmontar el disco
    if(bumount() == FALLO){
        fprintf(stderr, "Error en mi_ls: No se pudo desmontar el disco.\n");
        return FALLO;
    }

    return EXITO;
}