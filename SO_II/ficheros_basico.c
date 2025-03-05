// Autores: Xavier Campos, Pedro Félix, Harpo Joan

#include "ficheros_basico.h"

/**
 * @brief Calcula el tamaño del mapa de bits
 * @param nbloques Número de bloques
 * @return Tamaño del mapa de bits
 */
int tamMB(unsigned int nbloques){
    int tamMB = (nbloques/8)/BLOCKSIZE;
    if(nbloques%BLOCKSIZE != 0){
        return tamMB+1;
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
        return tam+1; // Agregar un bloque extra si hay restos
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
        fprintf(stderr, RED"Error al leer el superbloque en initAI()\n"RESET);
        return FALLO;
    }

    contInodos = SB.posPrimerInodoLibre + 1;
    for(int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        if(bread(i, &inodos)==FALLO){
            fprintf(stderr, RED"Error al leer inodo en initAI\n"RESET);
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

/**
 * @brief Escribe un bit en el mapa de bits
 * @param nbloque Número de bloque
 * @param bit Bit a escribir
 */
int escribir_bit(unsigned int nbloque, unsigned int bit){
    int posbyte, posbit, nbloqueMB, nbloqueabs;

    // leer superbloque
    struct superbloque SB;
    if(bread(0, &SB) == FALLO){
        printf(RED "Error al leer el superbloque en escribir_bit()\n" RESET);
        return FALLO;
    }

    // calcular posición del byte y del bit
    posbyte = nbloque / 8;
    posbit = nbloque % 8;

    // determinar el bloque del MB en el que se encuentra el byte
    nbloqueMB = posbyte/BLOCKSIZE;

    // determinar la posición absoluta del bloque del MB
    nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    // leer el bloque del MB
    unsigned char bufferMB[BLOCKSIZE];
    bread(nbloqueabs, &bufferMB);

    // calcular la posición del byte dentro del bloque
    posbyte = posbyte % BLOCKSIZE;

    // calcular la máscara
    unsigned char mascara = 128; // 10000000
    mascara >>= posbit; // desplazamiento de bits a la derecha

    // escribir el bit
    if (bit == 0) {
        bufferMB[posbyte] &= ~mascara; // poner a 0 el bit correspondiente
    } else {
        bufferMB[posbyte] |= mascara; // poner a 1 el bit correspondiente
    }
    bwrite(nbloqueabs, &bufferMB); // escribir el bloque modificado
    
    return EXITO;
}
/**
 * @brief Lee un bit del mapa de bits
 * @param nbloque Número de bloque
 * @return Valor del bit leído
 */
char leer_bit(unsigned int nbloque){
    struct superbloque SB;
    int posbyte, posbit, nbloqueMB, nbloqueabs;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128;
    // Leer el superbloque
    if (bread(posSB, &SB) == FALLO) {
        printf(RED"Error en leer_bit()\n"RESET);
        return FALLO;
    }
    // Calcular la posición del byte y del bit
    posbyte = nbloque / 8;
    posbit = nbloque % 8;
    nbloqueMB = posbyte / BLOCKSIZE;
    nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
#if DEBUG_N3
    printf(GRAY "[leer_bit(%d)→ posbyte:%d, posbyte (ajustado): %d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\n" RESET, nbloque, posbyte, posbyte % BLOCKSIZE, posbit, nbloqueMB, nbloqueabs);
#endif
    // Leer el bloque del mapa de bits
    if (bread(nbloqueabs, bufferMB) == FALLO) {
        printf(RED"Error en leer_bit()\n"RESET);
        return FALLO;
    }
    // Cálculo de la máscara mediante desplazamiento de bits
    posbyte = posbyte % BLOCKSIZE;
    mascara >>= posbit;
    mascara &= bufferMB[posbyte];
    mascara >>= (7 - posbit);
    return mascara;
}

int reservar_bloque(){

}
/**
 * @brief Libera un bloque
 * @param nbloque Número de bloque
 * @return Número de bloque liberado
 */
int liberar_bloque(unsigned int nbloque){
    struct superbloque SB;
    // Escritura del bit en el mapa de bits
    if (escribir_bit(nbloque, 0) == FALLO) {
        printf(RED"Error al escribir bit en liberar_bloque()\n"RESET);
        return FALLO;
    }
    // Lectura del superbloque
    if (bread(posSB, &SB) == FALLO) {
        printf(RED"Error al leer bloque en liberar_bloque()\n"RESET);
        return FALLO;
    }
    // Actualización de la cantidad de bloques libres
    SB.cantBloquesLibres++;
    if (bwrite(posSB, &SB) == FALLO) {
        printf(RED"Error al aumentar SB.cantBloquesLibres en liberar_bloque()\n"RESET);
        return FALLO;
    }
    // Devolución del número de bloque liberado
    return nbloque;
}

/**
 * @brief Escribe un inodo en el array de inodos
 * @param ninodo Número de inodo
 * @param inodo Inodo a escribir
 */
int escribir_inodo(unsigned int ninodo, struct inodo *inodo){
    int nbloqueAI, nbloqueabs, posinodo;

    // leer el superbloque
    struct superbloque SB;
    if(bread(0, &SB) == FALLO){
        printf(RED "Error al leer el superbloque en escribir_inodo()\n" RESET);
        return FALLO;
    }

    // calcular el bloque del array de inodos
    nbloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;

    // calcular la posición absoluta del bloque del array de inodos
    nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;

    // leer el bloque del array de inodos
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    bread(nbloqueabs, &inodos);

    // calcular la posición del inodo dentro del bloque
    posinodo = ninodo % (BLOCKSIZE/INODOSIZE);

    // escribir el inodo
    inodos[posinodo] = *inodo;
    bwrite(nbloqueabs, &inodos);

    return EXITO;

}

int leer_inodo(unsigned int ninodo, struct inodo *inodo){

}

int reservar_inodo(unsigned char tipo, unsigned char permisos){
    int posInodoReservado, posAI, posInodo;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    struct inodo inodo;

    // Comprobar si hay inodos libres
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        printf(RED"Error al leer el superbloque en reservar_inodo()\n"RESET);
        return FALLO;
    }
    if (SB.cantInodosLibres == 0) {
        printf(RED"No hay inodos libres en reservar_inodo()\n"RESET);
        return FALLO;
    }

    // Guardar posición del primer inodo libre
    posInodoReservado = SB.posPrimerInodoLibre;

    // Actualizar el primer inodo libre del superbloque
    posAI = SB.posPrimerBloqueAI + posInodoReservado;
    posInodo = (posInodoReservado) / (BLOCKSIZE / INODOSIZE);
    bread(posAI, &inodos);
    SB.posPrimerInodoLibre = inodos[posInodo].punterosDirectos[0];

    // Inicializar el inodo
    inodo.tipo = tipo;
    inodo.permisos = permisos;    
    inodo.nlinks = 1;
    inodo.tamEnBytesLog = 0;
    inodo.btime = time(NULL);
    inodo.numBloquesOcupados = 0;
    for (int i = 0; i < 12; i++) {
        inodo.punterosDirectos[i] = 0;
    }
    for (int i = 0; i < 3; i++) {
        inodo.punterosIndirectos[i] = 0;
    }

    // Escribir el inodo en el array de inodos
    if (escribir_inodo(posInodoReservado, &inodo) == FALLO) {
        printf(RED"Error al escribir el inodo en reservar_inodo()\n"RESET);
        return FALLO;
    }

    // Actualizar la cantidad de inodos libres
    SB.cantInodosLibres--;

    return posInodoReservado;
}