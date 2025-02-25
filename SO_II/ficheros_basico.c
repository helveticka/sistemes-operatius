// Autores: Xavier Campos, Pedro Félix, Harpo Joan

#include "ficheros_basico.h"

/**
 * @brief Calcula el tamaño del mapa de bits
 * @param nbloques Número de bloques
 * @return Tamaño del mapa de bits
 */
int tamMB(unsigned int nbloques){
    int tamMB = (nbloques/8)/BLOCKSIZE;
    if(nbloques%8 != 0){
        tamMB++;
    }
    return tamMB;
}
/**
 * @brief Calcula el tamaño en bloques del array de inodos.
 * @param ninodos Número de inodos
 * @return Tamaño en bloques del array de inodos
 */
int tamAI(unsigned int ninodos){
    int tam = ninodos / (BLOCKSIZE / INODOSIZE);
    if (ninodos % (BLOCKSIZE / INODOSIZE) != 0) {
        tam++; // Agregar un bloque extra si hay restos
    }
    return tam;
}
/**
 *  @brief Inicializa los datos del superbloque
 *  @param nbloques Número de bloques
 *  @param ninodos Número de inodos
 *  @return FALLO si ha habido error al escribir la estructura, EXITO en caso contrario
 */
int initSB(unsigned int nbloques, unsigned int ninodos){
    struct superbloque SB;

    // Inicialización de las posiciones del superbloque
    SB.posPrimerBloqueMB = posSB + tamSB; // posSB = 0, tamSB = 1
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;

    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;

    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;

    // Inicialización de información sobre inodos
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantInodosLibres = ninodos;

    // Inicialización de información sobre bloques
    SB.cantBloquesLibres = nbloques;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    // Escribir el superbloque en el bloque 0
    if (bwrite(0, &SB) == FALLO) {
        return FALLO; // Error en la escritura
    }

    return EXITO; // Éxito
}
