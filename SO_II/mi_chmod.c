#include "directorios.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: ./mi_chmod <disco> <permisos> </ruta>\n");
        return 1;
    }

    // Parseo de permisos (número octal)
    int permisos = atoi(argv[2]);
    
    // Verificamos si el valor de los permisos es válido (entre 0 y 7)
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, "Error: Valor de permisos inválido. Debe estar entre 0 y 7.\n");
        return 1;
    }

    // Obtenemos la ruta del archivo/directorio
    const char *path = argv[3];

    // Llamamos a la función mi_chmod para cambiar los permisos
    int resultado = mi_chmod(path, permisos);
    
    if (resultado != 0) {
        fprintf(stderr, "Error: No se pudo cambiar los permisos de '%s'.\n", path);
        return FALLO;
    }

    printf("Los permisos de '%s' se cambiaron con éxito.\n", path);
    return EXITO;
}
