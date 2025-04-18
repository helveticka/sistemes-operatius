/**
 * @file leer_sf.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"
/**
 * @brief Función principal que muestra información sobre el SB 
 * y realiza pruebas de las funciones de ficheros_basico.c
 * @param argc Cantidad de argumentos
 * @param argv Argumentos
 * @return EXITO si no hay errores, FALLO en caso contrario
 */
int main(int argc, char *argv[]) {
    const char *nombre_dispositivo = argv[1];
    if (bmount(nombre_dispositivo) == FALLO) {
        fprintf(stderr, RED "Error al montar el dispositivo virtual en ./leer_sf" RESET);
        return FALLO;
    }
    struct superbloque SB;
    bread(0, &SB);
    // Información del superbloque
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
// Debug de la semana 2
#if DEBUGN2
    // Información de los bloques de metadatos
    printf("\nsizeof struct superbloque: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));
    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    // Struct que contiene los inodos de un bloque
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    // Posición del bloque del inodo en el array de inodos
    int posAI = SB.posPrimerBloqueAI+(SB.posPrimerInodoLibre)/(BLOCKSIZE/INODOSIZE);
    // Posición del inodo en el bloque (relativa)
    int posInodo = (SB.posPrimerInodoLibre)%(BLOCKSIZE/INODOSIZE);
    // Posición del siguiente bloque de inodos en el array de inodos
    int nextAI = 0;
    // Posición absoluta del próximo inodo
    int nextInodo = 0;
    // Lectura del primer bloque de inodos
    bread(posAI, &inodos);
    // Recorrido de la lista enlazada de inodos libres
    while (posAI <= SB.posUltimoBloqueAI && inodos[posInodo].punterosDirectos[0] != UINT_MAX) {
        printf("%d ", ((posAI - SB.posPrimerBloqueAI) * (BLOCKSIZE / INODOSIZE)) + posInodo + 1);
        nextInodo = inodos[posInodo].punterosDirectos[0];
        nextAI = SB.posPrimerBloqueAI + (nextInodo / (BLOCKSIZE / INODOSIZE));
        posInodo = nextInodo % (BLOCKSIZE / INODOSIZE);
        if (nextAI != posAI) {
            posAI = nextAI;
            bread(posAI, &inodos);
        }        
    }
    printf("-1 ");
#endif
// Debug de la semana 3
#if DEBUGN3
    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    int nBloque = reservar_bloque();
    // Comprobamos que se ha reservado un bloque
    if (nBloque == FALLO) {
        fprintf(stderr, RED "Error al reservar bloque en ./leer_sf" RESET);
        return FALLO;
    }
    printf("Se ha reservado el bloque físico nº %d que era el 1º libre indicado por el MB\n", nBloque);
    // Leemos el SB para comprobar que se ha actualizado la cantidad de bloques libres
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, RED "Error al leer bloque en ./leer_sf" RESET);
        return FALLO;
    }
    printf("SB.cantBloquesLibres: %d\n", SB.cantBloquesLibres);
    // Liberamos el bloque
    if (liberar_bloque(nBloque) == FALLO) {
        fprintf(stderr, RED "Error al liberar bloque en ./leer_sf" RESET);
        return FALLO;
    }
    // Leemos el SB para comprobar que se ha actualizado la cantidad de bloques libres
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, RED "Error al leer bloque en ./leer_sf" RESET);
        return FALLO;
    }
    // Imprimimos el mapa de bits para comprobar que se ha liberado el bloque
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
    // Leemos el inodo del directorio raíz
    if (leer_inodo(SB.posInodoRaiz, &inodo) == FALLO) {
        fprintf(stderr, RED "Error al leer inodo en ./leer_sf" RESET);
        return FALLO;
    }
    // Imprimimos los datos del inodo
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
// Debug de la semana 4
#if DEBUGN4
    struct inodo inodo;
    printf("\nINODO 1. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n");
    int nblogicos[5] = {8, 204, 30004, 400004, 468750};
    int ninodo = reservar_inodo('f', 6);
    // Comprobamos que se ha reservado un inodo
    if (ninodo == FALLO) {
        fprintf(stderr, RED "Error al reservar inodo en ./leer_sf" RESET);
        return FALLO;
    }
    // Leemos el inodo
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, RED "Error al leer inodo en ./leer_sf" RESET);
        return FALLO;
    }
    // Traducimos los bloques lógicos
    for (int i=0; i<sizeof(nblogicos)/sizeof(int); i++) {
        if (traducir_bloque_inodo(ninodo, nblogicos[i], 1) == FALLO) {
            fprintf(stderr, RED "Error al traducir el bloque del inodo en ./leer_sf" RESET);
            return FALLO;
        }
        printf("\n");
    }
    // Imprimimos los datos del inodo
    printf("\nDATOS DEL INODO RESERVADO 1\n");
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, RED "Error al leer inodo en ./leer_sf" RESET);
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
    // Leemos el SB para comprobar que se ha actualizado la cantidad de inodos libres
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, RED "Error al leer bloque en ./leer_sf" RESET);
        return FALLO;
    }
    printf("\nSB.posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
#endif

#if DEBUGN7
    //Mostrar creación directorios y errores
    mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
    mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
    mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
    mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
    mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);  
    //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
    mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
    mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2
#endif
    // Desmontamos el dispositivo virtual
    if (bumount()) {
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual en ./leer_sf" RESET);
        return FALLO;
    }
    return EXITO;
}

void mostrar_buscar_entrada(char *camino, char reservar){
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    printf("\ncamino: %s, reservar: %d\n", camino, reservar);
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
        mostrar_error_buscar_entrada(error);
    }
    printf("**********************************************************************\n");
    return;
}