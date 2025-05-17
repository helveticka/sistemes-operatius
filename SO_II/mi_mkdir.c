/**
 * @file mi_mkdir.c
 * @brief Funciones para crear un directorio
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"
/**
 * @brief Función principal para crear un directorio
 * @param argc Número de argumentos
 * @param argv Array de argumentos
 * @return Éxito o fallo
 */
int main (int argc, char **argv) {
    // Comprobar argumentos
    if (argc != 4) {
        fprintf(stderr, RED "Sintaxis: ./mi_mkdir <nombre_dispositivo> <permisos> </ruta_directorio/>\n" RESET);
        return FALLO;
    }
    // Comprobar permisos
    int permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, RED "Error: modo inválido: <<%d>>\n" RESET, permisos);
        return FALLO;
    }
    // Comprobar ruta
    char *ruta = argv[3];
    if (ruta[strlen(ruta) - 1] != '/') {
        fprintf(stderr, RED "Error: Camino incorrecto.\n" RESET);
        return FALLO;
    }
    // Comprobar que el camino no contenga caracteres inválidos
    char *camino = argv[1];
    // Montar el dispositivo
    bmount(camino);
    // Comprobar que el dispositivo se haya montado correctamente
    mostrar_error_buscar_entrada(mi_creat(ruta, permisos));
    // Desmontar el dispositivo
    bumount();
    return EXITO;
}