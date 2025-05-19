/**
 * @file mi_escribir.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"

int main (int argc, char **argv) {
    int offset, nbytes, bytes_escritos;
    char *nombre_dispositivo, *ruta, *texto;

    // Comprobar la sintaxis
    if (argc != 5) {
        fprintf(stderr, RED "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n" RESET);
        return FALLO;
    }

    // Comprobar que la ruta es un fichero
    ruta = argv[2];
    if(ruta[strlen(ruta)-1] == '/') {
        fprintf(stderr, RED "Error en mi_escribir: la ruta no es un fichero\n" RESET);
        return FALLO;
    }

    // Comprobar que el texto a escribir no es vacío
    texto = argv[3];
    nbytes = strlen(texto);
    if (nbytes <= 0) {
        fprintf(stderr, RED "Error en mi_escribir: el texto a escribir no es válido\n" RESET);
        return FALLO;
    }
#if DEBUGN9
    fprintf(stderr, "longitud texto: %d\n", nbytes);
#endif

    // Comprobar que el offset es válido
    offset = atoi(argv[4]);
    if (offset < 0) {
        fprintf(stderr, RED "Error en mi_escribir: el offset no es válido\n" RESET);
        return FALLO;
    }

    nombre_dispositivo = argv[1];

    // Montar el sistema de archivos
    if (bmount(nombre_dispositivo) == FALLO) {
        fprintf(stderr, RED"Error en mi_escribir: montando el dispositivo virtual en ./mi_escribir"RESET);
        return FALLO;
    }

    if ((bytes_escritos = mi_write(ruta, texto, offset, nbytes)) < 0) {
        mostrar_error_buscar_entrada(bytes_escritos);
        if (bytes_escritos == FALLO){
            bytes_escritos = 0;
        }
        
    }

#if DEBUGN9
    fprintf(stderr, "Bytes escritos: %d\n", bytes_escritos);
#endif
    // Desmontar el sistema de archivos
    if (bumount() == FALLO) {
        fprintf(stderr, RED"Error desmontando el dispositivo\n"RESET);
        return FALLO;
    }
    return EXITO;
}