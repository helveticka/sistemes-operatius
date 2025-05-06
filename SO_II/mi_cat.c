/**
 * @file mi_cat.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"
/**
 * @brief Función principal que lee un fichero
 * @param argc Cantidad de argumentos
 * @param argv Argumentos
 * @return EXITO si no hay errores, FALLO en caso contrario
 */
int main(int argc, char **argv) {
    struct STAT p_stat;
    int ninodo, leidos, total_leidos = 0, offset = 0, tambuffer = 1500;
    char *path, buffer_texto[tambuffer];
    // Comprobamos que el número de argumentos sea el correcto
    if (argc != 3) {
        fprintf(stderr, RED "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n" RESET);
        return FALLO;
    }
    char *ruta = argv[2];
    if (ruta[strlen(ruta) - 1] == '/') {
        fprintf(stderr, RED "ERROR: La entrada de ruta no es un fichero\n" RESET);
        return FALLO;
    }
    // Montamos el dispositivo virtual
    path = argv[1];
    if (bmount(path) == FALLO) {
        fprintf(stderr, RED "Error al montar el dispositivo virtual en ./mi_cat" RESET);
        return FALLO;
    }
    // Leemos el inodo
    memset(buffer_texto, 0, tambuffer);
    leidos = mi_read(ninodo, buffer_texto, offset, tambuffer);
    // Leemos el contenido del inodo
    while (leidos > 0) {
        write(1, buffer_texto, leidos);
        total_leidos += leidos;
        offset += tambuffer;
 
        memset(buffer_texto, 0, tambuffer);
        leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
        if (leidos == FALLO) {
            fprintf(stderr, RED "Error en mi_read_f en ./mi_cat" RESET);
            return FALLO;
        }
    }
#if DEBUGN9
    fprintf(stderr, "Total_leidos: %d\n", total_leidos);
#endif
    if (bumount() == FALLO){
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual en ./mi_cat" RESET);
        return FALLO;
    }
    return EXITO;
}