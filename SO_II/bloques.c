// Autores: Xavier Campos, Pedro Félix, Harpo Joan

#include "bloques.h"

int bmount(const char *camino){
    descriptor = open(camino,O_RDWR|O_CREAT,0666);
    if(descriptor == -1){
        return -1;
    }
    return descriptor;
}

int bumount(){
    if(close(descriptor) == -1){
        return -1;
    }
    return 0;
}

int bwrite(unsigned int nbloque, const void *buf){
    lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET);
    if(write(descriptor,buf,BLOCKSIZE) == -1){
        return -1;
    }
    return BLOCKSIZE;
}

int bread(unsigned int nbloque, void *buf){

}