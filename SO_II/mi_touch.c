/**
 * @file mi_touch.c
 * @brief Funciones para crear un fichero
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"
/**
 * @brief Función principal para crear un fichero
 * @param argc Número de argumentos
 * @param argv Array de argumentos
 * @return Éxito o fallo
 */
int main (int argc, char **argv) {
    char *camino, *ruta;
    int permisos, error;

    // Comprobar argumentos
    if (argc != 4) {
        fprintf(stderr, RED "Sintaxis: ./mi_touch <nombre_dispositivo> <permisos> </ruta/>\n" RESET);
        return FALLO;
    }
    // Comprobar permisos
    permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, RED "Error: Permisos incorrectos\n" RESET);
        return FALLO;
    }
    // Comprobar ruta
    ruta = argv[3];
    if (ruta[strlen(ruta) - 1] == '/') {
        fprintf(stderr, RED "Error: Camino incorrecto.\n" RESET);
        return FALLO;
    }
    // Comprobar que el camino no contenga caracteres inválidos
    camino = argv[1];
    // Montar el dispositivo
    if(bmount(camino) == FALLO){
        fprintf(stderr, RED "Error en ./mi_touch: No se pudo montar el disco '%s'.\n", camino);
        return FALLO;
    }
    // Comprobar que el dispositivo se haya montado correctamente
    error = mi_creat(ruta, permisos);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        bumount();
        return FALLO;
    }
    // Desmontar el dispositivo
    if(bumount() == FALLO){
        fprintf(stderr, RED "Error en ./mi_touch: No se pudo desmontar el disco '%s'.\n", camino);
        return FALLO;
    }
    return EXITO;
}