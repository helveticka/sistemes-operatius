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
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    for(int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; i++) {
        if(bread(i, &inodos)==FALLO){
            fprintf(stderr, RED"Error -/leer_sf\n"RESET);
            return FALLO;
        }
        for(int j = 0; j < BLOCKSIZE/INODOSIZE; j++) {
            inodos[j].tipo = 'l';   //'l' = libre

            if(((i-sb.posPrimerBloqueAI)*BLOCKSIZE/INODOSIZE+j+1)==sb.totInodos) {
                printf("-1 ");
            } else { // es el ultimo inodo
                printf("%d ", ((i-sb.posPrimerBloqueAI)*BLOCKSIZE/INODOSIZE+j+1));
            }
        }

        if (bwrite(i, &inodos) != BLOCKSIZE) return FALLO;
    }

    bumount();
    return EXIT_SUCCESS;
}

