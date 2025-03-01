#include "ficheros_basico.h"

int main(int argc, char *argv[]) {

    const char *nombre_dispositivo = argv[1];
    bmount(nombre_dispositivo);
    struct superbloque sb;
    bread(0, &sb);

    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %d\n", sb.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %d\n", sb.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n", sb.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n", sb.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n", sb.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n", sb.posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n", sb.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n", sb.posPrimerInodoLibre);
    printf("cantBloquesLibres = %d\n", sb.cantBloquesLibres);
    printf("cantInodosLibres = %d\n", sb.cantInodosLibres);
    printf("totBloques = %d\n", sb.totBloques);
    printf("totInodos = %d\n", sb.totInodos);
    
    printf("\nsizeof struct superbloque: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));

    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    // struct que contiene los inodos de un bloque
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    // posición del bloque del inodo en el array de inodos
    int posAI = sb.posPrimerBloqueAI+(sb.posPrimerInodoLibre)/(BLOCKSIZE/INODOSIZE);
    // posición del inodo en el bloque (relativa)
    int posInodo = (sb.posPrimerInodoLibre)%(BLOCKSIZE/INODOSIZE);
    // posición del siguiente bloque de inodos en el array de inodos
    int nextAI = 0;
    // posición absoluta del próximo inodo
    int nextInodo = 0;

    bread(posAI, &inodos);

    // RECORRIDO DE LA LISTA ENLAZADA DE INODOS LIBRES
    while(posAI <= sb.posUltimoBloqueAI && inodos[posInodo].punterosDirectos[0] != UINT_MAX) {
        printf("%d ",((posAI-sb.posPrimerBloqueAI)*(BLOCKSIZE/INODOSIZE))+posInodo+1);
        nextInodo = inodos[posInodo].punterosDirectos[0];
        nextAI = sb.posPrimerBloqueAI+(nextInodo/(BLOCKSIZE/INODOSIZE));
        posInodo = nextInodo%(BLOCKSIZE/INODOSIZE);
        if(nextAI != posAI) {
            posAI = nextAI;
            bread(posAI, &inodos);
        }        
    }
    printf("-1 ");

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

    bumount();
    return EXIT_SUCCESS;
}

