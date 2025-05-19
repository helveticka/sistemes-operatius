#include "directorios.h"


int main(int argc, char **argv)
{
    char *camino, *ruta;
    int error;
    
    // Comprobar la sintaxis
    if (argc != 3){
        fprintf(stderr, RED"Sintaxis: ./mi_rmdir <disco> </ruta_directorio>\n"RESET);
        return FALLO;
    }

    // Comprobar que la ruta es un directorio
    ruta = argv[2];
    if (ruta[strlen(ruta)-1] != '/'){
        fprintf(stderr,RED"ERROR: La entrada de '</ruta_directorio>' no es un directorio\n"RESET);
        return FALLO;
    }

    camino = argv[1];

    // Montar el sistema de archivos
    if (bmount(camino) == FALLO){
        return FALLO;
    }

    // Borrar el directorio
    if ((error = mi_unlink(ruta)) < 0){
        mostrar_error_buscar_entrada(error);
        if (error == FALLO){
            return FALLO;
        }
    }

    // Desmontar el sistema de archivos
    if (bumount() == FALLO){
        return FALLO;
    }
    return EXITO;
}