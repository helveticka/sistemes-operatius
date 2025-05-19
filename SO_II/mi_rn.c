#include "directorios.h"

/**
 * @file mi_rn.c
 * @brief Cambia el nombre de un fichero o directorio
 */
int main(int argc, char **argv)
{
    char *camino, *ruta, *nuevo;
    int error;
    
    if (argc != 4)
    {
        fprintf(stderr, RED"Sintaxis: ./mi_rn <disco> </ruta/antiguo> <nuevo>\n"RESET);
        return FALLO;
    }

    ruta = argv[2];
    nuevo = argv[3];

    camino = argv[1];
    if (bmount(camino) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_rn\n"RESET);
        return FALLO;
    }

    if ((error = mi_rename(ruta, nuevo)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        if (error == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_rn\n"RESET);
            return FALLO;
        }
    }

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_rn\n"RESET);
        return FALLO;
    }
    return EXITO;
}