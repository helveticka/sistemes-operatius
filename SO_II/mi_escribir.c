/**
 * @file mi_escribir.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"

int main (int argc, char **argv) {
    int aux, offset;
    if (argc != 5) {
        fprintf(stderr, RED "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n" RESET);
        return FALLO;
    }
    char *nombre_dispositivo = argv[1];
    char *ruta = argv[2];
    char *texto = argv[3];
    offset = atoi(argv[4]);
    int nbytes = strlen(texto);
#if DEBUGN_9
    fprintf(stderr, "longitud texto: %d\n", nbytes);
#endif
    int diferentes_inodos = atoi(argv[3]);
    unsigned int ninodo;
    int bytes_escritos, total_bytes = strlen(texto);
    printf("longitud texto: %d\n", total_bytes);
    // Montar el sistema de archivos
    if (bmount(nombre_dispositivo) == FALLO) {
        fprintf(stderr, RED"Error montando el dispositivo virtual en ./mi_escribir"RESET);
        return FALLO;
    }
    if ((aux = mi_write_f(ninodo, texto, offset, total_bytes)) < 0) {
        mostrar_error_buscar_entrada(aux);
        aux = 0;
    }
#if DEBUGN_9
    fprintf(stderr, "Bytes escritos: %d\n", aux);
#endif
    // Desmontar el sistema de archivos
    if (bumount() == FALLO) {
        fprintf(stderr, RED"Error desmontando el dispositivo\n"RESET);
        return FALLO;
    }
    return EXITO;
}