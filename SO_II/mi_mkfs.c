// Autores: Xavier Campos, Pedro Félix, Harpo Joan
#include "ficheros_basico.h"
/**
 * @brief Crea un fichero de bloques con todos los bloques a 0
 * @param argc Número de argumentos
 * @param argv Argumentos pasados por la terminal
 */
int main(int argc, char**argv) {
    int nbloques = atoi(argv[2]);
    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);
    // Montamos el dispositivo virtual 
    if (bmount(argv[1]) == FALLO) {
        perror(RED "Error en bmount()\n");
        printf(RESET);
        return FALLO;
    }
    // Inicializamos a 0 todos los bytes del disco virtual
    for (int i = 0; i < nbloques; i++) {
        if (bwrite(i, buffer) == FALLO) {
            perror(RED "Error en bwrite()\n");
            printf(RESET);
            return FALLO;
        }
    }
    initSB();
    initMB();
    initAI();
    // Desmontamos el dispositivo virtual
    if (bumount() == FALLO) {
        perror(RED "Error en bumount()\n");
        printf(RESET);
        return FALLO;
    }
    
}