/**
 * @file mi_mkfs
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"
/**
 * @brief Crea un fichero de bloques con todos los bloques a 0
 * @param argc Número de argumentos
 * @param argv Argumentos pasados por la terminal
 */
int main(int argc, char**argv) {
    int nbloques = atoi(argv[2]);
    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);

#if MMAP
    // Bloque para mmap
    unsigned int finDV = atoi(argv[2]) * BLOCKSIZE - 1;
    FILE *fp = fopen(argv[1], "w");
    fseek(fp, finDV, SEEK_SET);
    fputc('\0', fp);
    fclose(fp);
#endif

    // Montamos el dispositivo virtual 
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "Error al montar el dispositivo virtual en ./mi_mkfs\n");
        return FALLO;
    }
    // Inicializamos a 0 todos los bytes del disco virtual
    for (int i = 0; i < nbloques; i++) {
        if (bwrite(i, buffer) == FALLO) {
            fprintf(stderr, RED "Error en bwrite() en ./mi_mkfs\n");
            return FALLO;
        }
    }
    initSB(nbloques, nbloques/4);
    initMB();
    initAI();
    reservar_inodo ('d', 7);
    // Desmontamos el dispositivo virtual
    if (bumount() == FALLO) {
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual en ./mi_mkfs\n");
        return FALLO;
    }
}