// Autores: Xavier Campos, Pedro Félix, Harpo Joan

#include "ficheros_basico.h"

int tamMB(unsigned int nbloques){
    int tamMB = (nbloques/8)/BLOCKSIZE;
    if(nbloques%8 != 0){
        tamMB++;
    }
    return tamMB;
}