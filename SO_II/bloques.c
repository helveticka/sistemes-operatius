// Autores: Xavier Campos, Pedro Félix, Harpo Joan

#include "bloques.h"

int bmount(const char *camino){
    descriptor = open(camino,O_RDWR|O_CREAT,0666);
    if(descriptor == FALLO){
        return FALLO;
    }
    return descriptor;
}

int bumount(){
    if(close(descriptor) == FALLO){
        return FALLO;
    }
    return 0;
}

int bwrite(unsigned int nbloque, const void *buf){
    lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET);
    if(write(descriptor,buf,BLOCKSIZE) == -1){
        return FALLO;
    }
    return BLOCKSIZE;
}

int bread(unsigned int nbloque, void *buf){
    lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET);
    size_t bytes_leidos = read(descriptor, buf, BLOCKSIZE);
    if(bytes_leidos==-1){
        return FALLO;
    }
    return bytes_leidos;
}