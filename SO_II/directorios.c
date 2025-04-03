/**
 * @file directorios.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {

}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
    struct entrada entrada;
    struct inodo inodo_dir;
    struct superbloque SB;
    unsigned char inicial[sizeof(entrada.nombre)];
    unsigned char final[strlen(camino_parcial)];
    unsigned char tipo;
    unsigned cant_entradas_inodo, num_entrada_inodo;
    
    if (camino_parcial == '/') {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return 0;
    }
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO) {
        fprintf(stderr, RED "Error al extraer el camino en buscar_entrada\n" RESET);
        return ERROR_CAMINO_INCORRECTO;
    }
    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO) {
        fprintf(stderr, RED "El inodo no tiene permisos de lectura\n" RESET);
        return ERROR_PERMISO_LECTURA;
    }
    struct entrada *buffer_lecturas = malloc(BLOCKSIZE);
    memset(buffer_lecturas, 0, BLOCKSIZE);
    //calcular cant_entradas_inodo
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0;
    if (cant_entradas_inodo > 0) {
        ///leer entrada (mejora)
    }
}

void mostrar_error_buscar_entrada(int error) {
    // fprintf(stderr, "Error: %d\n", error);
    switch (error) {
    case -2: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
    case -3: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
    case -4: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
    case -5: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
    case -6: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
    case -7: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
    case -8: fprintf(stderr, "Error: No es un directorio.\n"); break;
    }
 }
 