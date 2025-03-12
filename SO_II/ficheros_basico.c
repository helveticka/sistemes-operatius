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

    // Calcular el tamaño del mapa de bits
    blocks = tamMB(SB.totBloques);
    bytes = tamAI(SB.totInodos);
    size = tamSB+blocks+bytes; // Tamaño total de los bloques ocupados por el superbloque, mapa de bits y array de inodos
    sizeBits = size/8; // Tamaño en bytes del mapa de bits
    
    if(sizeBits/BLOCKSIZE > 1) { // Si el tamaño del mapa de bits es mayor que un bloque
        // Rellenar los bloques de mapa de bits con 1
        for(int i =0; i<(sizeBits/BLOCKSIZE); i++) {
            memset(bufferMB, 255, BLOCKSIZE);
            if(bwrite(SB.posPrimerBloqueMB+i,bufferMB)==FALLO) {
                perror(RED "Error initSB()");
                printf(RESET);
                return FALLO;
            }
        }
    } else { // Si el tamaño del mapa de bits es menor que un bloque
        // Rellenar el bloque con la cantidad de 1 correspondientes
        for(int i=0; i<sizeBits;i++) {
            bufferMB[i]=255;
        }
    }

    // Rellenar el último byte con los 1 correspondientes
    if(size%8 != 0) {
        char ultimoByte=0;
        for(int i =0; i<(size%8);i++) {
            ultimoByte |= (1 << (7-i));
        }
        bufferMB[sizeBits]=ultimoByte;
        sizeBits++;
    }

    // Rellenar el resto del bloque con 0
    for(int i = sizeBits;i<BLOCKSIZE;i++) bufferMB[i]=0;

    // Escribir el mapa de bits en el dispositivo virtual
    if(bwrite(SB.posPrimerBloqueMB, bufferMB)==FALLO) {
        perror(RED "Error initSB()");
        printf(RESET);
        return FALLO;
    }

    // Actualizar la cantidad de bloques libres
    SB.cantBloquesLibres -= size;

    // Actualizar el superbloque
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
/** 
 * @brief Encuentra el primer bloque libre, lo ocupa y devuelve su posición
 * @return La posición del primer bolque libre, FALLO si ha habido algún error
 */
int reservar_bloque(){
    struct superbloque SB;

    // Leer el superbloque
    if (bread(0, &SB) == FALLO) {
        return FALLO; // Error al leer el superbloque
    }
    // Comprobar si quedan bloques libres
    if (SB.cantBloquesLibres == 0) {
        return FALLO; // No hay bloques disponibles
    }

    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    memset(bufferAux, 255, BLOCKSIZE); // Llenar bufferAux con bits a 1

    int nbloqueMB, posbyte, posbit, nbloque;

    // Buscar el primer bloque del MB con al menos un bit a 0
    for (nbloqueMB = 0; nbloqueMB < tamMB(SB.totBloques); nbloqueMB++) {
        if (bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB) == -1) {
            return FALLO; // Error al leer el MB
        }

        if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0) {
            // Encontramos un bloque con al menos un bit a 0
            break;
        }
    }

    // Buscar el primer byte con un bit a 0 dentro del bloque encontrado
    for (posbyte = 0; posbyte < BLOCKSIZE; posbyte++) {
        if (bufferMB[posbyte] != 255) {
            // Encontramos un byte con al menos un bit a 0
            break;
        }
    }

    // Buscar el primer bit a 0 dentro del byte encontrado
    unsigned char mascara = 128; // 10000000
    posbit = 0;
    while (bufferMB[posbyte] & mascara) { // AND binario
        bufferMB[posbyte] <<= 1; // Desplazamiento de bits a la izquierda
        posbit++;
    }

    // Calcular el número de bloque a reservar
    nbloque = (nbloqueMB * BLOCKSIZE * 8) + (posbyte * 8) + posbit;

    // Reservar el bloque (poner el bit a 1)
    if (escribir_bit(nbloque, 1) == FALLO) {
        return FALLO; // Error al escribir el bit
    }

    // Actualizar el superbloque
    SB.cantBloquesLibres--;

    if (bwrite(0, &SB) == FALLO) {
        return FALLO; // Error al escribir el superbloque actualizado
    }

    // Limpiar el bloque de datos con ceros
    unsigned char bufferCero[BLOCKSIZE];
    memset(bufferCero, 0, BLOCKSIZE);

    if (bwrite(nbloque, bufferCero) == FALLO) {
        return FALLO; // Error al limpiar el bloque de datos
    }

    return nbloque; // Devolver el número del bloque reservado
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
/**
 * @brief Lee un determinado inodo del array de inodos para volcarlo en una variable de tipo struct inodo pasada por referencia.
 * @param ninodo Número de inodo
 * @param inodo Inodo a escribir
 * @return EXITO si se ha leido correctamente, FALLO en caso contrario
 */
int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    struct superbloque SB;

    // Leer el superbloque para obtener la posición del array de inodos
    if (bread(0, &SB) == FALLO) {
        return FALLO; // Error al leer el superbloque
    }

    // Calcular el número de bloque del array de inodos donde está el inodo
    unsigned int nbloqueAI = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));

    // Buffer para leer un bloque de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // Leer el bloque de inodos correspondiente
    if (bread(nbloqueAI, inodos) == FALLO) {
        return FALLO; // Error al leer el bloque de inodos
    }

    // Copiar el inodo solicitado en la variable pasada por referencia
    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];

    return EXITO; // Éxito
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
    posAI = (SB.posPrimerBloqueAI) + ((posInodoReservado*INODOSIZE)/BLOCKSIZE);
    posInodo = (posInodoReservado) % (BLOCKSIZE / INODOSIZE);
    bread(posAI, &inodos);
    SB.posPrimerInodoLibre = inodos[posInodo].punterosDirectos[0];
    bwrite(posSB, &SB);

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

int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar){
    unsigned int ptr, ptr_ant, salvar_inodo;
    int nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];
    struct inodo inodo;

    ptr = 0;
    ptr_ant = 0;
    salvar_inodo = 0;
    indice = 0;

    leer_inodo(ninodo, &inodo);
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); //0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros = nRangoBL;
    while (nivel_punteros > 0) { // Iterar sobre los punteros indirectos
        if (ptr == 0) { // No cuelgan bloques de punteros
            if (reservar == 0) { // No existe bloque
                return FALLO;
            } else {
                // Reservar bloques de punteros
                ptr = reservar_bloque(); // de punteros
                inodo.numBloquesOcupados++;
                inodo.ctime = time(NULL); // fecha actual
                salvar_inodo = 1;

                if(nivel_punteros == nRangoBL) { // el bloque cuelga directamente del inodo
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr;
                } else { // el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr; // salvar el puntero del bloque reservado
                    bwrite(ptr_ant, buffer); // salvar el bloque de punteros modificado
                }
                memset(buffer, 0, BLOCKSIZE); // llenar de ceros el buffer de punteros
            }
        } else {
            // Leer el bloque de punteros
            bread(ptr, buffer);
        }
        
        // Calcular el índice
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr; // el puntero actual pasa a ser el puntero anterior
        ptr = buffer[indice]; // siguiente puntero
        nivel_punteros--;
    }
    if(ptr == 0) { // No existe bloque de datos
        if(reservar == 0) {
            return FALLO;
        } else {
            // Reservar bloque de datos
            ptr = reservar_bloque();
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            salvar_inodo = 1;

            if(nRangoBL == 0) { // si era un puntero directo
                inodo.punterosDirectos[nblogico] = ptr; // actualizar puntero directo
            } else {
                buffer[indice] = ptr; // guardamos puntero al bloque de datos
                bwrite(ptr_ant, buffer); // escribimos el bloque de punteros
            }
        }
    }
    // Escribir el inodo si se ha modificado
    if(salvar_inodo == 1) {
        escribir_inodo(ninodo, &inodo);
    }
    return ptr; // devolver el número de bloque de datos lógico
}
/**
 * @brief Obtiene el rango de bloques lógicos
 * @param inodo Inodo
 * @param nblogico Número de bloque lógico
 * @param ptr Puntero
 * @return Rango de bloques lógicos
 */
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr){
    if(nblogico < DIRECTOS) {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    } else if(nblogico < INDIRECTOS0) {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    } else if(nblogico < INDIRECTOS1) {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    } else if(nblogico < INDIRECTOS2) {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    } else {
        *ptr = 0;
        fprintf(stderr, RED"Bloque lógico fuera de rango\n"RESET);
        return FALLO;
    }
}
/**
 * @brief Obtiene el índice del bloque de punteros
 * @param nblogico Número de bloque lógico
 * @param nivel_punteros Nivel en el que se encuentra el puntero
 * @return Índice del bloque de punteros, FALLO en caso de error
 */
int obtener_indice(unsigned int nblogico, int nivel_punteros){
    if (nblogico < DIRECTOS) {
        return nblogico;
    } else if (nblogico < INDIRECTOS0) {
        return nblogico - DIRECTOS;
    } else if (nblogico < INDIRECTOS1) {
        if (nivel_punteros == 2) {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        } else if (nivel_punteros == 1) {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    } else if (nblogico < INDIRECTOS2) {
        if (nivel_punteros == 3) {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        } else if (nivel_punteros == 2) {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        } else if (nivel_punteros == 1) {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
    return FALLO; // Caso de error o fuera de rango
}