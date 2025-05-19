#include "directorios.h"


int main(int argc, char **argv)
{
    char *camino, *ruta;
    int error;
    
    // Comprobar la sintaxis
    if (argc != 3){
        fprintf(stderr, RED"Sintaxis: ./mi_rm <disco> </ruta_fichero>\n"RESET);
        return FALLO;
    }

    // Comprobar que la ruta es un fichero
    ruta = argv[2];
    if (ruta[strlen(ruta)-1] == '/'){
        fprintf(stderr,RED"ERROR: La entrada no es un fichero\n"RESET);
        return FALLO;
    }

    camino = argv[1];

    // Montar el sistema de archivos
    if (bmount(camino) == FALLO){
        return FALLO;
    }

    // Borrar el fichero
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