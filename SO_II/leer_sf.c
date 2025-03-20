#include "ficheros_basico.h"

int main(int argc, char *argv[]) {

    const char *nombre_dispositivo = argv[1];
    bmount(nombre_dispositivo);
    struct superbloque SB;
    bread(0, &SB);

    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %d\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %d\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres = %d\n", SB.cantBloquesLibres);
    printf("cantInodosLibres = %d\n", SB.cantInodosLibres);
    printf("totBloques = %d\n", SB.totBloques);
    printf("totInodos = %d\n", SB.totInodos);
#if DEBUGN2
    
    printf("\nsizeof struct superbloque: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));

    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    // struct que contiene los inodos de un bloque
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    // posición del bloque del inodo en el array de inodos
    int posAI = SB.posPrimerBloqueAI+(SB.posPrimerInodoLibre)/(BLOCKSIZE/INODOSIZE);
    // posición del inodo en el bloque (relativa)
    int posInodo = (SB.posPrimerInodoLibre)%(BLOCKSIZE/INODOSIZE);
    // posición del siguiente bloque de inodos en el array de inodos
    int nextAI = 0;
    // posición absoluta del próximo inodo
    int nextInodo = 0;

    bread(posAI, &inodos);

    // RECORRIDO DE LA LISTA ENLAZADA DE INODOS LIBRES
    while(posAI <= SB.posUltimoBloqueAI && inodos[posInodo].punterosDirectos[0] != UINT_MAX) {
        printf("%d ",((posAI-SB.posPrimerBloqueAI)*(BLOCKSIZE/INODOSIZE))+posInodo+1);
        nextInodo = inodos[posInodo].punterosDirectos[0];
        nextAI = SB.posPrimerBloqueAI+(nextInodo/(BLOCKSIZE/INODOSIZE));
        posInodo = nextInodo%(BLOCKSIZE/INODOSIZE);
        if(nextAI != posAI) {
            posAI = nextAI;
            bread(posAI, &inodos);
        }        
    }
    printf("-1 ");
#endif

    // RECORRIDO DE TODOS LOS INODOS
    //struct inodo inodos[BLOCKSIZE/INODOSIZE];
    //for(int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; i++) {
    //    if(bread(i, &inodos)==FALLO){
    //        fprintf(stderr, RED"Error -/leer_sf\n"RESET);
    //        return FALLO;
    //    }
    //    for(int j = 0; j < BLOCKSIZE/INODOSIZE; j++) {
    //        inodos[j].tipo = 'l';   //'l' = libre
    //
    //        if(((i-sb.posPrimerBloqueAI)*BLOCKSIZE/INODOSIZE+j+1)==sb.totInodos) {
    //            printf("-1 ");
    //        } else { // es el ultimo inodo
    //            printf("%d ", ((i-sb.posPrimerBloqueAI)*BLOCKSIZE/INODOSIZE+j+1));
    //        }
    //    }
    //
    //    if (bwrite(i, &inodos) != BLOCKSIZE) return FALLO;
    //}

#if DEBUGN3
    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    int nBloque = reservar_bloque();

    if (nBloque == FALLO) {
        perror(RED "Error ./leer_sf()");
        printf(RESET);
        return FALLO;
    }

    printf("Se ha reservado el bloque físico nº %d que era el 1º libre indicado por el MB\n", nBloque);
    
    if (bread(posSB, &SB) == FALLO) {
        perror(RED "Error ./leer_sf()");
        printf(RESET);
        return FALLO;
    }

    printf("SB.cantBloquesLibres: %d\n", SB.cantBloquesLibres);
    
    if (liberar_bloque(nBloque) == FALLO) {
        perror(RED "Error ./leer_sf()");
        printf(RESET);
        return FALLO;
    }

    if (bread(posSB, &SB) == FALLO) {
            perror(RED "Error ./leer_sf()");
            printf(RESET);
            return FALLO;
        }

    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %d\n", SB.cantBloquesLibres);

    printf("\n\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    printf(GRAY"[leer_bit(%d) -> posbyte:%d, posbyte(ajustado): %d, posbit:%d, nBloqueMB:%d, nbloqueabs:%d]\n" RESET, posSB, (posSB/8), ((posSB/8) % BLOCKSIZE), (posSB%8), (posSB/8/1024), (posSB/8/1024)+1);
    printf("posSB: %d → leer_bit(%d) = %d\n", posSB, posSB, leer_bit(posSB));
    printf(GRAY"[leer_bit(%d) -> posbyte:%d, posbyte(ajustado): %d, posbit:%d, nBloqueMB:%d, nbloqueabs:%d]\n" RESET, SB.posPrimerBloqueMB, (SB.posPrimerBloqueMB/8), ((SB.posPrimerBloqueMB/8) % BLOCKSIZE), (SB.posPrimerBloqueMB%8), (SB.posPrimerBloqueMB/8/1024), (SB.posPrimerBloqueMB/8/1024)+1);
    printf("SB.posPrimerBloqueMB: %d → leer_bit(%d) = %d\n", SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, leer_bit(SB.posPrimerBloqueMB));
    printf(GRAY"[leer_bit(%d) -> posbyte:%d, posbyte(ajustado): %d, posbit:%d, nBloqueMB:%d, nbloqueabs:%d]\n" RESET, SB.posUltimoBloqueMB, (SB.posUltimoBloqueMB/8), ((SB.posUltimoBloqueMB/8) % BLOCKSIZE), (SB.posUltimoBloqueMB%8), (SB.posUltimoBloqueMB/8/1024), (SB.posUltimoBloqueMB/8/1024)+1);
    printf("SB.posUltimoBloqueMB: %d → leer_bit(%d) = %d\n", SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, leer_bit(SB.posUltimoBloqueMB));
    printf(GRAY"[leer_bit(%d) -> posbyte:%d, posbyte(ajustado): %d, posbit:%d, nBloqueMB:%d, nbloqueabs:%d]\n" RESET, SB.posPrimerBloqueAI, (SB.posPrimerBloqueAI/8), ((SB.posPrimerBloqueAI/8) % BLOCKSIZE), (SB.posPrimerBloqueAI%8), (SB.posPrimerBloqueAI/8/1024), (SB.posPrimerBloqueAI/8/1024)+1);
    printf("SB.posPrimerBloqueAI: %d → leer_bit(%d) = %d\n", SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, leer_bit(SB.posPrimerBloqueAI));
    printf(GRAY"[leer_bit(%d) -> posbyte:%d, posbyte(ajustado): %d, posbit:%d, nBloqueMB:%d, nbloqueabs:%d]\n" RESET, SB.posUltimoBloqueAI, (SB.posUltimoBloqueAI/8), ((SB.posUltimoBloqueAI/8) % BLOCKSIZE), (SB.posUltimoBloqueAI%8), (SB.posUltimoBloqueAI/8/1024), (SB.posUltimoBloqueAI/8/1024)+1);
    printf("SB.posUltimoBloqueAI: %d → leer_bit(%d) = %d\n", SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, leer_bit(SB.posUltimoBloqueAI));
    printf(GRAY"[leer_bit(%d) -> posbyte:%d, posbyte(ajustado): %d, posbit:%d, nBloqueMB:%d, nbloqueabs:%d]\n" RESET, SB.posPrimerBloqueDatos, (SB.posPrimerBloqueDatos/8), ((SB.posPrimerBloqueDatos/8) % BLOCKSIZE), (SB.posPrimerBloqueDatos%8), (SB.posPrimerBloqueDatos/8/1024), (SB.posPrimerBloqueDatos/8/1024)+1);
    printf("SB.posPrimerBloqueDatos: %d → leer_bit(%d) = %d\n", SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, leer_bit(SB.posPrimerBloqueDatos));
    printf(GRAY"[leer_bit(%d) -> posbyte:%d, posbyte(ajustado): %d, posbit:%d, nBloqueMB:%d, nbloqueabs:%d]\n" RESET, SB.posUltimoBloqueDatos, (SB.posUltimoBloqueDatos/8), ((SB.posUltimoBloqueDatos/8) % BLOCKSIZE), (SB.posUltimoBloqueDatos%8), (SB.posUltimoBloqueDatos/8/1024), (SB.posUltimoBloqueDatos/8/1024)+1);
    printf("SB.posUltimoBloqueDatos: %d → leer_bit(%d) = %d\n", SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));
    
    printf("\n\nDATOS DEL DIRECTORIO RAIZ\n");
    struct inodo inodo;
    char atime[80];
    char ctime[80];
    char mtime[80];

    if (leer_inodo(SB.posInodoRaiz, &inodo) == FALLO) {
        perror(RED "Error ./leer_sf()");
        printf(RESET);
        return FALLO;
    }

    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %d\n", inodo.permisos);
    
    
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", localtime(&inodo.atime));
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", localtime(&inodo.ctime));
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", localtime(&inodo.mtime));
    
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);

    printf("nlinks: %d\n", inodo.nlinks);
    printf("tamEnBytesLog: %d\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", inodo.numBloquesOcupados);
#endif

#if DEBUGN4
    struct inodo inodo;
    
    printf("\nINODO 1. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n");
    int nblogicos[5] = {8, 204, 30004, 400004, 468750};
    int ninodo = reservar_inodo('f', 6);
    if (ninodo == FALLO) {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }

    if (leer_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }
    
    for (int i=0; i<sizeof(nblogicos)/sizeof(int); i++) {
        if (traducir_bloque_inodo(ninodo, nblogicos[i], 1) == FALLO) {
            fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
            return FALLO;
        }
        printf("\n");
    }

    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }
    printf("\nDATOS DEL INODO RESERVADO 1\n");
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }

    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %d\n", inodo.permisos);
    
    char atime[80];
    char ctime[80];
    char mtime[80];
    
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", localtime(&inodo.atime));
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", localtime(&inodo.ctime));
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", localtime(&inodo.mtime));
    
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);

    printf("nlinks: %d\n", inodo.nlinks);
    printf("tamEnBytesLog: %d\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", inodo.numBloquesOcupados);

    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }
    printf("\nSB.posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
#endif

    if (bumount()) {
        perror(RED "Error en bumount() al ./leer_sf()");
        printf(RESET);
        return FALLO;
    }
    
    return EXIT_SUCCESS;
}


