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

/**
 * @brief Escribe el mapa de bits en el dispositivo virtual
 * @param nbloque Número de bloque
 * @param bit Bit a escribir
 * @return FALLO si ha habido error al escribir el mapa de bits, EXITO en caso contrario
 */
int initMB() {
    struct superbloque SB;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned int size, blocks, bytes, sizeBits;
    
    //comprobamos que existe el superbloque
    if(bread(posSB, &SB) == FALLO) {
        perror(RED "Error InitMB()"RESET);
        return FALLO;
    }

    blocks = tamMB(SB.totBloques);
    bytes = tamAI(SB.totInodos);
    size = tamSB+blocks+bytes;
    sizeBits = size/8;

    if(sizeBits/BLOCKSIZE > 1) {
        for(int i =0; i<(sizeBits/BLOCKSIZE); i++) {
            memset(bufferMB, 255, BLOCKSIZE);
            if(bwrite(SB.posPrimerBloqueMB+i,bufferMB)==FALLO) {
                perror(RED "Error initSB()");
                printf(RESET);
                return FALLO;
            }
        }
    } else {
        for(int i=0; i<sizeBits;i++) {
            bufferMB[i]=255;
        }
    }
    if(size%8 != 0) {
        char ultimoByte=0;
        for(int i =0; i<(size%8);i++) {
            ultimoByte |= (1 << (7-i));
        }
        bufferMB[sizeBits]=ultimoByte;
        sizeBits++;
    }

    for(int i = sizeBits;i<BLOCKSIZE;i++) bufferMB[i]=0;

    if(bwrite(SB.posPrimerBloqueMB, bufferMB)==FALLO) {
        perror(RED "Error initSB()");
        printf(RESET);
        return FALLO;
    }

    SB.cantBloquesLibres -= size;

    if (bwrite(posSB, &SB) == FALLO) {
        perror(RED"Error in initMB()"RESET);
        return FALLO;
    }

    return EXITO;
}


/**
 * @brief Inicializa el array de inodos en el dispositivo virtual
 * @param nbloque Número de bloque
 * @param bit Bit a escribir
 * @return FALLO si ha habido error al escribir el array de inodos, EXITO en caso contrario
 */
int initAI() {
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    int contInodos;

    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, RED"Nonexistan Superblock in initAI()\n"RESET);
        return FALLO;
    }

    contInodos = SB.posPrimerInodoLibre + 1;
    for(int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        if(bread(i, &inodos)==FALLO){
            fprintf(stderr, RED"Nonexistan Inodo in initAI\n"RESET);
            return FALLO;
        }
        for(int j = 0; j < BLOCKSIZE/INODOSIZE; j++) {
            inodos[j].tipo = 'l';   //'l' = libre

            if(contInodos < SB.totInodos) {
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            } else { // es el ultimo inodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }

        if (bwrite(i, &inodos) != BLOCKSIZE) return FALLO;
    }
    
    if(bwrite(posSB, &SB)==FALLO) return FALLO;

    return EXITO;
}