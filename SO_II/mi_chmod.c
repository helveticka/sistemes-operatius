/**
 * @file mi_chmod.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/**
 * @brief Función principal que cambia los permisos de un fichero o directorio
 * @param argc Cantidad de argumentos
 * @param argv Argumentos
 * @return EXITO si no hay errores, FALLO en caso contrario
 */
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, RED "Sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>\n");
        return 1;
    }

    // Parseo de permisos (número octal)
    int permisos = atoi(argv[2]);

    
    // Verificamos si el valor de los permisos es válido (entre 0 y 7)
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, RED "Error: Valor de permisos inválido. Debe estar entre 0 y 7.\n");
        return FALLO;
    }
    
    // Montamos el dispositivo
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "Error: No se pudo montar el disco '%s'.\n", argv[1]);
        return FALLO;
    }

    // Obtenemos la ruta del archivo/directorio
    const char *path = argv[3];

    // Llamamos a la función mi_chmod para cambiar los permisos
    int resultado = mi_chmod(path, permisos);
    
    if (resultado != 0) {
        fprintf(stderr, RED "Error: No se pudo cambiar los permisos de '%s'.\n", path);
        return FALLO;
    }

    // Desmontamos el dispositivo
    if (bumount() == FALLO) {
        fprintf(stderr, RED "Error: No se pudo desmontar el disco '%s'.\n", argv[1]);
        return FALLO;
    }
    return EXITO;
}
