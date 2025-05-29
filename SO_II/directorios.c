/**
 * @file directorios.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"

// Definición de la estructura de caché
#if USARCACHE == 1
    #define CACHE_SIZE 2 // Cantidad de entradas en la caché
    static struct ultimaEntrada cache[CACHE_SIZE];
    static int CACHE_LIBRE = CACHE_SIZE; // Número de entradas libres en la caché
#endif

#if (USARCACHE == 2 || USARCACHE == 3)
    #define CACHE_SIZE 3 // Cantidad de entradas en la caché
    static struct ultimaEntrada cache[CACHE_SIZE];
    static int CACHE_LIBRE = CACHE_SIZE; // Número de entradas libres en la caché
#endif

#if USARCACHE == 1 || USARCACHE == 2
    static int ultima_entrada_mod = 0; // Última entrada de la caché actualizada
#endif
/**
 * @brief Extrae el camino de un directorio o fichero
 * @param camino Cadena de caracteres que representa el camino
 * @param inicial Cadena de caracteres donde se almacenará la parte inicial del camino
 * @param final Cadena de caracteres donde se almacenará la parte final del camino
 * @param tipo Tipo de entrada ('d' para directorio, 'f' para fichero)
 * @return 0 si se ha extraído correctamente, -1 si el camino es incorrecto
 */
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    // Validar que comience por '/'
    if (camino[0] != '/') {
        return FALLO;
    }

    const char *segundo_slash = strchr(camino + 1, '/');

    if (segundo_slash == NULL) {
        // No hay segundo '/', es un fichero
        strcpy(inicial, camino + 1);  // omitimos el primer '/'
        final[0] = '\0';
        *tipo = 'f';
    } else {
        // Hay al menos un segundo '/', es un directorio
        size_t len = segundo_slash - (camino + 1); // longitud de inicial
        strncpy(inicial, camino + 1, len);
        inicial[len] = '\0';  // terminador nulo
        strcpy(final, segundo_slash); // desde el segundo slash incluido
        *tipo = 'd';
    }

    return 0;
}

/**
 * @brief Busca una entrada en un directorio
 * @param camino_parcial Camino parcial a buscar
 * @param p_inodo_dir Puntero al inodo del directorio
 * @param p_inodo Puntero al inodo de la entrada
 * @param p_entrada Puntero a la entrada
 * @param reservar Indica si se debe reservar un inodo
 * @param permisos Permisos del inodo
 * @return EXITO si se encuentra la entrada o los distintos ERRORES tipificados
 */
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
    struct entrada entrada;
    struct inodo inodo_dir;
    struct superbloque SB;
    char inicial [TAMNOMBRE];
    char final[strlen(camino_parcial)+1];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo, offset;

    // Caso especial para la raíz
    if (strcmp(camino_parcial, "/") == 0) {
        // Leer el superbloque
        if (bread(posSB, &SB) == FALLO) {
            fprintf(stderr, "Error al leer el superbloque\n");
            return FALLO;
        }
        if (reservar == 1) {
            return FALLO;
        }
        *p_inodo = SB.posInodoRaiz; // El inodo de la raíz
        *p_entrada = 0;
        return EXITO;
    }

    memset(inicial, 0, sizeof(inicial));
    memset(final, 0, sizeof(final));

    // Extraer el camino
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO) {
        return ERROR_CAMINO_INCORRECTO;
    }

#if DEBUGN7 || DEBUGN8 || DEBUGN9 || DEBUGN10
    fprintf(stderr, GRAY "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n"RESET, inicial, final, reservar);
#endif

    // Leer el inodo del directorio
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if ((inodo_dir.permisos & 4) != 4) {
#if DEBUGN7 || DEBUGN8 || DEBUGN9 || DEBUGN10
        fprintf(stderr, GRAY "[buscar_entrada()→ El inodo %d no tiene permisos de lectura]\n" RESET, *p_inodo_dir);
#endif
        return ERROR_PERMISO_LECTURA;
    }

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / TAMENTRADA;
    num_entrada_inodo = 0;
    offset = 0;
    
    // Inicializar el buffer de lectura
    struct entrada entradas [BLOCKSIZE / TAMENTRADA];
    memset(&entrada, 0, TAMENTRADA);
    memset(entradas, 0, sizeof(entradas)); // Inicializar el buffer de entradas

    if (cant_entradas_inodo > 0) {
        if (mi_read_f(*p_inodo_dir, entradas, offset, BLOCKSIZE) < 0) {
            return FALLO;
        }
        // Simulación de la lectura de entradas
        while (num_entrada_inodo < cant_entradas_inodo && (strcmp(inicial, entradas[num_entrada_inodo%(BLOCKSIZE/TAMENTRADA)].nombre) != 0)) {
            num_entrada_inodo++;
            if ((num_entrada_inodo % (BLOCKSIZE / TAMENTRADA)) == 0) {
                // Leer el bloque correspondiente
                offset += BLOCKSIZE;
                memset(entradas, 0, sizeof(entradas)); // Inicializar nuevamente el buffer
                if (mi_read_f(*p_inodo_dir, entradas, offset, BLOCKSIZE) == FALLO) {
                    return FALLO;
                }
            }
        }
        memcpy(&entrada, &entradas[num_entrada_inodo%(BLOCKSIZE/TAMENTRADA)], TAMENTRADA); // Copiar la entrada leída
    }
    // Comprobar si la entrada existe o no
    if ((strcmp(inicial, entradas[num_entrada_inodo%(BLOCKSIZE/TAMENTRADA)].nombre) != 0) && num_entrada_inodo == cant_entradas_inodo) {
        // La entrada no existe
        switch (reservar) {
            case 0: // Modo consulta
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            case 1: //modo escritura
                if (inodo_dir.tipo == 'f') {
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }

                if ((inodo_dir.permisos & 2) != 2) {
                    return ERROR_PERMISO_ESCRITURA;
                } else {
                    // Copiar el nombre de la entrada
                    strcpy(entrada.nombre, inicial);
                    if (tipo == 'd') {
                        if (strcmp(final, "/") == 0) {
                            entrada.ninodo = reservar_inodo('d', permisos); // Reservamos un inodo como directorio
                        } else {
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    } else {
                        entrada.ninodo = reservar_inodo('f', permisos); // Reservamos un inodo como fichero
                    }
#if DEBUGN7 || DEBUGN8 || DEBUGN9 || DEBUGN10
                    fprintf(stderr, GRAY"[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n"RESET, entrada.ninodo, tipo, permisos, entrada.nombre);
                    fprintf(stderr, GRAY"[buscar_entrada()→ creada entrada: %s, %d]\n"RESET, inicial, entrada.ninodo);
#endif               
                    // Escribir la entrada en el directorio
                    if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * TAMENTRADA, TAMENTRADA) == FALLO) {
                        if (entrada.ninodo != FALLO) { // Si se había reservado un inodo
                            liberar_inodo(entrada.ninodo); // Liberar el inodo reservado 
                        }
                        return FALLO;
                    }
                }
        }
    }

    // Si hemos llegado al final del camino
    if ((strcmp(final, "/") == 0) || (strcmp(final, "") == 0)) {
        if (num_entrada_inodo < cant_entradas_inodo && (reservar == 1)) {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return EXITO;
    } else {
        // Recursividad
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return EXITO;
}
/**
 * @brief Muestra un mensaje de error según el código de error
 * @param error Código de error
 */
void mostrar_error_buscar_entrada(int error) {
    switch (error) {
    case -2:
        fprintf(stderr, RED "Error: Camino incorrecto.\n" RESET);
        break;
    case -3:
        fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET);
        break;
    case -4:
        fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET);
        break;
    case -5:
        fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n" RESET);
        break;
    case -6:
        fprintf(stderr, RED "Error: Permiso denegado de escritura.\n" RESET);
        break;
    case -7: 
        fprintf(stderr, RED "Error: El archivo ya existe.\n" RESET);
        break;
    case -8:
        fprintf(stderr, RED "Error: No es un directorio.\n" RESET);
        break;
    }
}
/**
 * @brief Muestra el contenido de un directorio
 * @param camino Ruta del directorio a mostrar
 * @param buffer Buffer donde se almacenará el contenido
 * @param tipo Tipo de entrada ('d' para directorio, 'f' para fichero)
 * @param flag Indica si se debe mostrar en formato largo ('l') o corto ('c')
 * @return Número de entradas leídas o error
 */
int mi_dir(const char *camino, char *buffer, char tipo, char flag) {
    struct inodo inodo;
    struct tm *tm;
    char tmp[100];
    unsigned int n_entrada = 0;
    unsigned int p_inodo = 0, p_inodo_dir = 0;
    int n_entradas = 0;

    // Buscar la entrada correspondiente a 'camino'
    if (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &n_entrada, 0, 0) < 0) {
        fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET);
        return FALLO;
    }

    // Leer el inodo del directorio
    if (leer_inodo(p_inodo, &inodo) < 0) {
        fprintf(stderr, RED "Error al leer inodo\n" RESET);
        return FALLO;
    }

    // Verificar permisos de lectura
    if (!(inodo.permisos & 4)) {
        fprintf(stderr, RED "Error: permiso de lectura denegado\n" RESET);
        return FALLO;
    }

    // Si es un fichero
    if (inodo.tipo == 'f') {
        if (flag == 'l') {
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d",
                    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                    tm->tm_hour, tm->tm_min, tm->tm_sec);

            sprintf(buffer + strlen(buffer), "Tipo\tModo\tmTime\t\t\tTamaño\t\tNombre\n");
            sprintf(buffer + strlen(buffer), "---------------------------------------------------------------------\n");
            sprintf(buffer + strlen(buffer), "%c\t", inodo.tipo);
            strcat(buffer, (inodo.permisos & 4) ? "r" : "-");
            strcat(buffer, (inodo.permisos & 2) ? "w" : "-");
            strcat(buffer, (inodo.permisos & 1) ? "x" : "-");
            sprintf(buffer + strlen(buffer), "\t%s\t%u\t\t", tmp, inodo.tamEnBytesLog);
            sprintf(tmp, "%s%s%s\n", CYAN, strrchr(camino, '/') + 1, RESET);
            strcat(buffer, tmp);
        } else {
            strcat(buffer, strrchr(camino, '/') + 1);
            strcat(buffer, "\n");
        }
        return EXITO;
    }

    // Si es un directorio, leemos sus entradas bloque por bloque
    int offset = 0, leidos = 0;
    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    int num_entradas_bloque = 0;
    int cabacera = 0;

    while ((leidos = mi_read_f(p_inodo, entradas, offset, BLOCKSIZE)) > 0) {
        if(cabacera == 0) {
            cabacera = 1;
            if (flag == 'l') {
                strcat(buffer, "Tipo\tModo\tmTime\t\t\tTamaño\t\tNombre\n");
                strcat(buffer, "---------------------------------------------------------------------\n");
            }
        }
        num_entradas_bloque = leidos / sizeof(struct entrada);
        for (int i = 0; i < num_entradas_bloque; i++) {

            struct inodo inodo_aux;
            if (leer_inodo(entradas[i].ninodo, &inodo_aux) < 0) {
                fprintf(stderr, RED "Error al leer inodo de entrada.\n" RESET);
                continue;
            }

            if (flag == 'l') {
                tm = localtime(&inodo_aux.mtime);
                sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d",
                        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                        tm->tm_hour, tm->tm_min, tm->tm_sec);

                sprintf(buffer + strlen(buffer), "%c\t", inodo_aux.tipo);
                strcat(buffer, (inodo_aux.permisos & 4) ? "r" : "-");
                strcat(buffer, (inodo_aux.permisos & 2) ? "w" : "-");
                strcat(buffer, (inodo_aux.permisos & 1) ? "x" : "-");
                sprintf(buffer + strlen(buffer), "\t%s\t%u\t\t", tmp, inodo_aux.tamEnBytesLog);

                if (inodo_aux.tipo == 'd') {
                    sprintf(tmp, "%s%s%s\n", ORANGE, entradas[i].nombre, RESET);
                } else {
                    sprintf(tmp, "%s%s%s\n", CYAN, entradas[i].nombre, RESET);
                }
                strcat(buffer, tmp);
            } else {
                if (inodo_aux.tipo == 'd') {
                    sprintf(tmp, "%s%s%s\t", ORANGE, entradas[i].nombre, RESET);
                } else {
                    sprintf(tmp, "%s%s%s\t", CYAN, entradas[i].nombre, RESET);
                }
                strcat(buffer, tmp);
            }
            n_entradas++;
        }
        offset += BLOCKSIZE;
    }

    if (flag != 'l') strcat(buffer, "\n");

    printf("Total: %d\n", n_entradas);

    return n_entradas;
}

/**
 * @brief Cambia los permisos de un fichero o directorio
 * @param camino Ruta del fichero o directorio
 * @param permisos Permisos a establecer
 * @return EXITO si se ha cambiado correctamente, FALLO en caso contrario
 */
int mi_chmod(const char *camino, unsigned char permisos) {
    unsigned int p_inodo, p_inodo_dir = 0, p_entrada;

    // Buscar la entrada
    int res = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if (res < 0) {
        fprintf(stderr, RED "Error en buscar_entrada() para el camino '%s'\n", camino);
        return res;
    }

    // Cambiar los permisos del inodo
    if (mi_chmod_f(p_inodo, permisos) == FALLO) {
        fprintf(stderr, RED "Error en mi_chmod_f() para el inodo %d\n", p_inodo);
        return FALLO;
    }

    return EXITO;
}
/**
 * @brief Muestra el estado de un fichero o directorio
 * @param camino Ruta del fichero o directorio
 * @param p_stat Puntero a la estructura donde se almacenará el estado
 * @return EXITO si se ha mostrado correctamente, FALLO en caso contrario
 */
int mi_stat(const char *camino, struct STAT *p_stat) {
    unsigned int p_inodo, p_inodo_dir = 0, p_entrada;

    int resultado = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (resultado < 0) {
        fprintf(stderr, "Error en buscar_entrada() para el camino '%s'\n", camino);
        return resultado;
    }

    if (mi_stat_f(p_inodo, p_stat) == FALLO) {
        fprintf(stderr, "Error en mi_stat_f() para el inodo %d\n", p_inodo);
        return FALLO;
    }

    printf("Nº de inodo: %d\n", p_inodo); // Mostrar número de inodo

    return EXITO; // 0
} 
/**
 * @brief Crea un fichero o directorio
 * @param camino Ruta del fichero o directorio a crear
 * @param permisos Permisos a establecer
 * @return EXITO si se ha creado correctamente, FALLO en caso contrario
 */
int mi_creat(const char *camino, unsigned char permisos) {
    mi_waitSem();
    unsigned int p_inodo = 0, p_inodo_dir = 0, p_entrada;
    int error;
    p_inodo_dir = 0;
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    if (error < 0) {
        mi_signalSem(); 
        return error;
    } else {
        mi_signalSem();
        return EXITO;
    }
}
/**
 * @brief Escribe en un fichero
 * @param camino Ruta del fichero a escribir
 * @param buf Buffer que contiene los datos a escribir
 * @param offset Offset desde donde se comenzará a escribir
 * @param nbytes Número de bytes a escribir
 * @return Número de bytes escritos o error
 */
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo = 0;
    unsigned int p_inodo_dir = 0;
    unsigned int p_entrada = 0;

#if USARCACHE == 1 || USARCACHE == 2 || USARCACHE == 3 // Si se utiliza caché
    int encontrada = 0; // Variable para comprobar si la entrada está en caché

    for (int i = 0; i < CACHE_SIZE && encontrada == 0; i++){

        // Comprobar si la entrada está en caché
        if (strcmp(cache[i].camino, camino) == 0) {
            p_inodo = cache[i].p_inodo;
            encontrada = 1;

#if USARCACHE == 3 // Si se utiliza estrategia LRU
            // Actualizar la última consulta
            gettimeofday(&cache[i].ultima_consulta, NULL);
#endif

#if DEBUGN9 && USARCACHE == 1
            fprintf(stderr, BLUE "[mi_write()→ Utilizamos la caché de escritura en vez de llamar a buscar_entrada()]\n" RESET);
#endif

#if DEBUGN9 && (USARCACHE == 2 || USARCACHE == 3)
            fprintf(stderr, BLUE "[mi_write()→ Utilizamos caché[%d]: %s]\n" RESET, i, camino);
#endif
        break;
        }
    }
    
    if(!encontrada) { // Si no está en caché
#endif
        // Buscar la entrada
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < 0) return error;

#if USARCACHE == 1 || USARCACHE == 2 || USARCACHE == 3 // Si se utiliza caché

        int indice = 0;

        if(CACHE_LIBRE > 0){ // Si hay espacio en la caché
            indice = CACHE_SIZE - CACHE_LIBRE;

            // Copiar el camino y el inodo en la caché
            strcpy(cache[indice].camino, camino);
            cache[indice].p_inodo = p_inodo;
            CACHE_LIBRE--; 

#if USARCACHE == 3
    gettimeofday(&cache[indice].ultima_consulta, NULL);
#endif

        } else{ // Si no hay espacio en la caché

#if USARCACHE == 1 || USARCACHE == 2 // última L/E, FIFO
            indice = ultima_entrada_mod;
            ultima_entrada_mod = (ultima_entrada_mod + 1) % CACHE_SIZE;
#endif

#if USARCACHE == 3 // Si se utiliza estrategia LRU
            indice = 0;
            for (int i = 0; i < CACHE_SIZE; i++){
                if(comparar_timeval(cache[i].ultima_consulta, cache[indice].ultima_consulta) < 0){
                    indice = i;
                }
            }
#endif
        // Reemplazar la entrada
        strcpy(cache[indice].camino, camino);
        cache[indice].p_inodo = p_inodo;

#if USARCACHE == 3
        // Actualizar la última consulta
        gettimeofday(&cache[indice].ultima_consulta, NULL);
#endif
        }

#if DEBUGN9 && USARCACHE == 1
        fprintf(stderr, ORANGE "[mi_write()→ Actualizamos la caché de escritura]\n" RESET);
#endif

#if DEBUGN9 && (USARCACHE == 2 || USARCACHE == 3)
        fprintf(stderr, ORANGE "[mi_write()→ Reemplazamos caché[%d]: %s]\n" RESET, indice, camino);
#endif

    }
#endif

    return mi_write_f(p_inodo, buf, offset, nbytes);
}

/**
 * @brief Lee un fichero y lo almacena en el buffer
 * @param camino Ruta del fichero a leer
 * @param buf Buffer donde se almacenará el contenido leído
 * @param offset Offset desde donde se comenzará a leer
 * @param nbytes Número de bytes a leer
 * @return Número de bytes leídos o error
 */
int mi_read(const char *camino, char *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo = 0;
    unsigned int p_inodo_dir = 0;
    unsigned int p_entrada = 0;

#if USARCACHE == 1 || USARCACHE == 2 || USARCACHE == 3 // Si se utiliza caché
    int encontrada = 0; // Variable para comprobar si la entrada está en caché

    for (int i = 0; i < CACHE_SIZE; i++){

        // Comprobar si la entrada está en caché
        if (strcmp(cache[i].camino, camino) == 0) {
            p_inodo = cache[i].p_inodo;
            encontrada = 1;

#if USARCACHE == 3 // Si se utiliza estrategia LRU
            // Actualizar la última consulta
            gettimeofday(&cache[i].ultima_consulta, NULL);
#endif

#if DEBUGN9 && USARCACHE == 1
            fprintf(stderr, BLUE "[mi_read()→ Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n" RESET);
#endif

#if DEBUGN9 && (USARCACHE == 2 || USARCACHE == 3)
            fprintf(stderr, BLUE "[mi_read()→ Utilizamos caché[%d]: %s]\n" RESET, i, camino);
#endif
        break;
        }
    }
    
    if(!encontrada) { // Si no está en caché
#endif
        // Buscar la entrada
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < 0){
            mostrar_error_buscar_entrada(error);
            return error;
        }

#if USARCACHE == 1 || USARCACHE == 2 || USARCACHE == 3 // Si se utiliza caché

        int indice = 0;

        if(CACHE_LIBRE > 0){ // Si hay espacio en la caché
            indice = CACHE_SIZE - CACHE_LIBRE;

            // Copiar el camino y el inodo en la caché
            strcpy(cache[indice].camino, camino);
            cache[indice].p_inodo = p_inodo;
            CACHE_LIBRE--; 

#if USARCACHE == 3
        // Actualizar la última consulta
        gettimeofday(&cache[indice].ultima_consulta, NULL);
#endif

        } else{ // Si no hay espacio en la caché

#if USARCACHE == 1 || USARCACHE == 2 // Si se utiliza estrategia FIFO
            indice = ultima_entrada_mod;
            ultima_entrada_mod = (ultima_entrada_mod + 1) % CACHE_SIZE;
#endif

#if USARCACHE == 3 // Si se utiliza estrategia LRU
            indice = 0;
            for (int i = 0; i < CACHE_SIZE; i++){
                if(comparar_timeval(cache[i].ultima_consulta, cache[indice].ultima_consulta) < 0){
                    indice = i;
                }
            }
#endif

        // Reemplazar la entrada
        strcpy(cache[indice].camino, camino);
        cache[indice].p_inodo = p_inodo;

#if USARCACHE == 3
        // Actualizar la última consulta
        gettimeofday(&cache[indice].ultima_consulta, NULL);
#endif

        }

#if DEBUGN9 && USARCACHE == 1
        fprintf(stderr, ORANGE "[mi_read()→ Actualizamos la caché de lectura]\n" RESET);
#endif

#if DEBUGN9 && (USARCACHE == 2 || USARCACHE == 3)
        fprintf(stderr, ORANGE "[mi_read()→ Reemplazamos caché[%d]: %s]\n" RESET, indice, camino);
#endif

    }
#endif

    return mi_read_f(p_inodo, buf, offset, nbytes);
}

/**
 * @brief Compara dos estructuras timeval
 * @param a Primera estructura timeval
 * @param b Segunda estructura timeval
 * @return -1 si a < b, 1 si a > b, 0 si son iguales
 */
int comparar_timeval(struct timeval a, struct timeval b) {
    if (a.tv_sec < b.tv_sec) return -1;
    if (a.tv_sec > b.tv_sec) return 1;
    if (a.tv_usec < b.tv_usec) return -1;
    if (a.tv_usec > b.tv_usec) return 1;
    return 0;
}
/**
 * @brief Crea un enlace a un fichero
 * @param camino1 Ruta del fichero origen
 * @param camino2 Ruta del fichero destino
 * @return EXITO si se ha creado correctamente, FALLO en caso contrario
 */
int mi_link(const char *camino1, const char *camino2){
    mi_waitSem();

    unsigned int p_inodo_dir1, p_inodo1, p_entrada1;
    unsigned int p_inodo_dir2, p_inodo2, p_entrada2;
    struct inodo inodo1, inodo2;
    struct entrada entrada;

    // Buscar camino fichero origen
    int err = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4);
    if (err < 0) {
        fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Leer inodo de fichero origen
    if (leer_inodo(p_inodo1, &inodo1) == FALLO){
        fprintf(stderr, RED "Error: No se pudo leer el inodo del fichero origen.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Comprobar que sea un fichero
    if (inodo1.tipo != 'f') {
        fprintf(stderr, RED "Error: El origen debe ser un fichero.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Comprobar permisos de lectura
    if ((inodo1.permisos & 4) != 4) {
        fprintf(stderr, RED "Error: Permiso denegado de lectura en el fichero origen.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Crear camino fichero destino
    err = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
    if (err < 0) {
        if (err == ERROR_ENTRADA_YA_EXISTENTE) {
            fprintf(stderr, RED "Error: El archivo ya existe.\n" RESET);
        }
        mi_signalSem();
        return FALLO;
    }
    
    // Leer inodo de camino destino
    if (leer_inodo(p_inodo2, &inodo2) == FALLO){
        fprintf(stderr, RED "Error: No se pudo leer el inodo del fichero destino.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Leer entrada del destino
    if (mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
        fprintf(stderr, RED "Error: No se pudo leer la entrada del destino.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Modificar entrada para que apunte al inodo del fichero origen
    entrada.ninodo = p_inodo1;
    if (mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
        fprintf(stderr, RED "Error: No se pudo escribir la entrada modificada.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Liberar inodo creado del destino
    liberar_inodo(p_inodo2);

    // Incrementar nlinks de origen y actualizar tiempo de modificación
    inodo1.nlinks++;
    inodo1.ctime = time(NULL);
    escribir_inodo(p_inodo1, &inodo1);

    mi_signalSem();
    return EXITO;
}
/**
 * @brief Elimina un enlace a un fichero o directorio
 * @param camino Ruta del fichero o directorio a eliminar
 * @return EXITO si se ha eliminado correctamente, FALLO en caso contrario
 */
int mi_unlink(const char *camino){
    mi_waitSem();
    unsigned int p_inodo_dir = 0, p_inodo, p_entrada;
    struct inodo inodo, inodo_dir;
    struct entrada entrada;
    int nentradas;
    int error;

    // Buscar entrada (comprobamos que existe)
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (error < 0) {
        fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Leer el inodo
    if (leer_inodo(p_inodo, &inodo) == FALLO) {
        mi_signalSem();
        return FALLO;
    }
    // Si es directorio, comprobar que está vacío
    if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0) {
        fprintf(stderr, RED "Error: El directorio %s no está vacío.\n" RESET, camino);
        mi_signalSem();
        return FALLO;
    }

    // Leer inodo del directorio padre
    if (leer_inodo(p_inodo_dir, &inodo_dir) == FALLO){
        mi_signalSem();
        return FALLO;
    }
    // Calcular número de entradas
    nentradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);

    if (nentradas > 0 && p_entrada != (nentradas - 1)) { // No es la última entrada, copiamos la última en su lugar
        if (mi_read_f(p_inodo_dir, &entrada, (nentradas - 1) * sizeof(struct entrada), sizeof(struct entrada)) < 0){
            mi_signalSem();
            return FALLO;
        }
        if (mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) < 0){
            mi_signalSem();
            return FALLO;
        }
    }

    // Truncar el inodo del directorio padre
    int nbytes = inodo_dir.tamEnBytesLog - sizeof(struct entrada);
    if (mi_truncar_f(p_inodo_dir, nbytes) < 0) {
        mi_signalSem();
        return FALLO;
    }
    // Decrementar nlinks del inodo borrado
    inodo.nlinks--;

    if (inodo.nlinks == 0) {
        // Liberar inodo si no hay más enlaces
        liberar_inodo(p_inodo);
    } else {
        // Actualizar tiempo de modificación y escritura del inodo
        inodo.ctime = time(NULL);
        if (escribir_inodo(p_inodo, &inodo) == FALLO) {
            mi_signalSem();
            return FALLO;
        }
    }
    mi_signalSem();
    return EXITO;
}

/**
 * @brief Cambia el nombre de un fichero o directorio
 * @param camino Ruta del fichero o directorio a renombrar
 * @param nuevo Nuevo nombre
 * @return EXITO si se ha renombrado correctamente, FALLO en caso contrario
 */
int mi_rename(const char *camino, const char *nuevo) {
    struct entrada entrada;
    char caminoNuevo[strlen(nuevo)+1];
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;

    p_inodo_dir = 0;
    
    // Buscar la entrada
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0){
        mostrar_error_buscar_entrada(error);
        return error;
    }

    // Construir el nuevo camino
    sprintf(caminoNuevo, "/%s", nuevo);

    // Comprobar que el nuevo nombre no exista
    if ((error = buscar_entrada(caminoNuevo, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) != ERROR_NO_EXISTE_ENTRADA_CONSULTA){
        if (error == EXITO){
            fprintf(stderr, RED"Error: La entrada ya existe\n"RESET);
        }
        return error;
    }
    
    // Leer el inodo del directorio padre
    if ((error = mi_read_f(p_inodo_dir, &entrada, p_entrada*TAMENTRADA, TAMENTRADA)) < 0){
        return error;
    }

    strcpy(entrada.nombre, nuevo);

    // Escribir la entrada en el directorio padre
    if ((error = mi_write_f(p_inodo_dir, &entrada, p_entrada*TAMENTRADA, TAMENTRADA)) < 0){
        return error;
    }

    return EXITO;
}

int mi_cp_f(const char *camino_origen, const char *camino_destino) {
    char nombre_origen[TAMNOMBRE];
    unsigned int p_inodo_origen = 0, p_inodo_dir_origen = 0;
    unsigned int p_inodo_destino, p_inodo_dir_destino = 0;
    unsigned int p_entrada_origen, p_entrada_destino;
    struct entrada entrada_origen;
    struct inodo inodo_destino;
    int error;

    // Comprobar que el origen existe
    if (buscar_entrada(camino_origen, &p_inodo_dir_origen, &p_inodo_origen, &p_entrada_origen, 0, 4) == FALLO) {
        fprintf(stderr, "Error: el origen no existe\n");
        return FALLO;
    }

    // Copiar el nombre del origen
    if ((error = mi_read_f(p_inodo_dir_origen, &entrada_origen, p_entrada_origen*TAMENTRADA, TAMENTRADA)) < 0){
        return error;
    }

    strcpy(nombre_origen, entrada_origen.nombre);

    // Comprobar que el destino existe y es un directorio
    if (buscar_entrada(camino_destino, &p_inodo_dir_destino, &p_inodo_destino, &p_entrada_destino, 0, 4) < 0) {
        mostrar_error_buscar_entrada(error);
        //fprintf(stderr, "Error: el directorio destino no existe\n");
        return FALLO;
    }

    if (leer_inodo(p_inodo_destino, &inodo_destino) < 0) {
        fprintf(stderr, RED "Error al leer inodo\n" RESET);
        return -1;
    }

    if (inodo_destino.tipo != 'd') {
        fprintf(stderr, "Error: el destino no es un directorio\n");
        return FALLO;
    }

    // Construir la ruta completa nueva
    char camino_nuevo[strlen(camino_destino) + strlen(nombre_origen) + 1];
    sprintf(camino_nuevo, "%s%s", camino_destino, nombre_origen);

    unsigned int p_entrada_nuevo, p_inodo_nuevo, p_inodo_dir_nuevo = 0;

    // Comprobar que no existe ya la entrada nueva y crearla
    error = buscar_entrada(camino_nuevo, &p_inodo_dir_nuevo, &p_inodo_nuevo, &p_entrada_nuevo, 1, 6);

    if (error == ERROR_ENTRADA_YA_EXISTENTE) {
        fprintf(stderr, "Error: la entrada destino ya existe\n");
        return FALLO;
    } else if(error < 0) {
        mostrar_error_buscar_entrada(error);
        fprintf(stderr, "Error: no se pudo crear la nueva entrada\n");
        return FALLO;
    }

    // Copiar contenido del fichero
    
    // Leer el inodo del origen
    struct inodo inodo_origen;
    if (leer_inodo(p_inodo_origen, &inodo_origen) < 0) {
        fprintf(stderr, RED "Error al leer el inodo del origen\n" RESET);
        return FALLO;
    }

    char buffer[BLOCKSIZE];
    for (int offset = 0, nBloque = 0; offset < inodo_origen.tamEnBytesLog; offset += BLOCKSIZE, nBloque++) {
        // Comprobar si el bloque está traducido (tiene información)
        if(traducir_bloque_inodo(p_inodo_origen, nBloque,0) > 0) {
            int leidos = 0;
            
            // Leer el bloque del fichero origen
            if((leidos = mi_read_f(p_inodo_origen, buffer, offset, BLOCKSIZE)) < 0) {
                fprintf(stderr, RED "Error al leer el bloque %d del fichero origen\n" RESET, nBloque);
                return FALLO;
            }

            // Escribir el bloque en el fichero destino
            if (mi_write_f(p_inodo_nuevo, buffer, offset, leidos) < 0) {
                fprintf(stderr, RED "Error al escribir en el fichero destino\n" RESET);
                return FALLO;
            }
        }
    }

    // Actualizar permisos del nuevo fichero
    struct inodo inodo_nuevo;
    if (leer_inodo(p_inodo_nuevo, &inodo_nuevo) < 0) {
        fprintf(stderr, RED "Error al leer el inodo del nuevo fichero\n" RESET);
        return FALLO;
    }
    inodo_nuevo.permisos = inodo_origen.permisos; // Copiar permisos
    escribir_inodo(p_inodo_nuevo, &inodo_nuevo);
    return EXITO;
}

int mi_cp(const char *camino_origen, const char *camino_destino) {
    char nombre_origen[TAMNOMBRE];
    unsigned int p_inodo_origen, p_inodo_dir_origen = 0;
    unsigned int p_inodo_destino, p_inodo_dir_destino = 0;
    unsigned int p_entrada_origen, p_entrada_destino;
    struct entrada entrada_origen;
    struct inodo inodo_origen, inodo_destino;
    int error = 0;

    // Comprobar que el origen existe
    if ((error = buscar_entrada(camino_origen, &p_inodo_dir_origen, &p_inodo_origen, &p_entrada_origen, 0, 4)) != EXITO) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    // Leer inodo del origen
    if (leer_inodo(p_inodo_origen, &inodo_origen) == FALLO){
        fprintf(stderr, RED "Error al leer el inodo del origen\n" RESET);
        return FALLO;
    }

    // Si es un fichero, usamos mi_cp_f y salimos
    if (inodo_origen.tipo == 'f') {
        return mi_cp_f(camino_origen, camino_destino);
    }

    // Leer la entrada para obtener el nombre del directorio
    if ((error = mi_read_f(p_inodo_dir_origen, &entrada_origen, p_entrada_origen * TAMENTRADA, TAMENTRADA)) < 0){
        return error;
    }
    strcpy(nombre_origen, entrada_origen.nombre);

    // Comprobar que el destino existe y es un directorio
    if ((error = buscar_entrada(camino_destino, &p_inodo_dir_destino, &p_inodo_destino, &p_entrada_destino, 0, 4)) < 0) {
        mostrar_error_buscar_entrada(error);
        //fprintf(stderr, "Error: el directorio destino no existe\n");
        return FALLO;
    }

    if (leer_inodo(p_inodo_destino, &inodo_destino) < 0 || inodo_destino.tipo != 'd') {
        fprintf(stderr, "Error: el destino no es un directorio\n");
        return FALLO;
    }

    // Crear el nuevo directorio en el destino
    char nuevo_camino[strlen(camino_destino) + strlen(nombre_origen) + 2];
    sprintf(nuevo_camino, "%s%s/", camino_destino, nombre_origen);

    unsigned int p_entrada_nuevo, p_inodo_nuevo, p_inodo_dir_nuevo = 0;
    error = buscar_entrada(nuevo_camino, &p_inodo_dir_nuevo, &p_inodo_nuevo, &p_entrada_nuevo, 1, 6);
    if (error == ERROR_ENTRADA_YA_EXISTENTE) {
        fprintf(stderr, "Error: la entrada destino ya existe\n");
        return FALLO;
    } else if (error < 0) {
        mostrar_error_buscar_entrada(error);
        fprintf(stderr, "Error: no se pudo crear el nuevo directorio\n");
        return FALLO;
    }

    // Recorrer entradas del directorio origen
    int n_entradas = inodo_origen.tamEnBytesLog / sizeof(struct entrada);
    struct entrada entrada_hija;

    for (int i = 0; i < n_entradas; i++) {
        if (mi_read_f(p_inodo_origen, &entrada_hija, i * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
            fprintf(stderr, "Error al leer entrada %d\n", i);
            return FALLO;
        }

        // Construir camino completo de origen
        char ruta_origen[strlen(camino_origen) + strlen(entrada_hija.nombre) + 2];
        sprintf(ruta_origen, "%s%s", camino_origen, entrada_hija.nombre);
        
        // Verificar si esta entrada es un directorio
        unsigned int p_inodo_hijo, p_inodo_dir_hijo = 0, p_entrada_hijo;
        if (buscar_entrada(ruta_origen, &p_inodo_dir_hijo, &p_inodo_hijo, &p_entrada_hijo, 0, 4) < 0) {
            mostrar_error_buscar_entrada(error);
            return FALLO;
        }
        struct inodo inodo_hijo;
        if (leer_inodo(p_inodo_hijo, &inodo_hijo) == FALLO) {
            fprintf(stderr, "Error al leer inodo de '%s'\n", ruta_origen);
            return FALLO;
        }

        // Si es un directorio, añadir '/' al final del origen
        if (inodo_hijo.tipo == 'd') {
            strcat(ruta_origen, "/");
        }

        // Copia recursiva
        if (mi_cp(ruta_origen, nuevo_camino) == FALLO) {
            //fprintf(stderr, "Error al copiar '%s' a '%s'\n", ruta_origen, ruta_destino);
            return FALLO;
        }
    }

    return EXITO;
}

int mi_rm_r(const char *camino) {
    unsigned int p_inodo, p_inodo_dir = 0, p_entrada;
    struct inodo inodo;
    int error;

    // Buscar la entrada
    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    // Leer el inodo
    if (leer_inodo(p_inodo, &inodo) < 0) {
        fprintf(stderr, RED "Error al leer el inodo del camino '%s'\n" RESET, camino);
        return FALLO;
    }

    // Recorrer sus entradas y eliminarlas recursivamente
    int offset = 0, leidos = 0;
    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    
    while ((leidos = mi_read_f(p_inodo, entradas, offset, BLOCKSIZE)) > 0) {
        int num_entradas_bloque = leidos / sizeof(struct entrada);
        for (int i = 0; i < num_entradas_bloque; i++) {

            // Leer el inodo de la entrada
            struct inodo inodo_aux;
            if (leer_inodo(entradas[i].ninodo, &inodo_aux) < 0) {
                fprintf(stderr, RED "Error al leer el inodo de la entrada '%s'\n" RESET, entradas[i].nombre);
                continue; // Continuar con la siguiente entrada
            }

            // Comprobar que la entrada es un fichero
            if(inodo_aux.tipo == 'f') {
                char subcamino[strlen(camino) + strlen(entradas[i].nombre) + 1];
                sprintf(subcamino, "%s%s", camino, entradas[i].nombre);
                mi_unlink(subcamino);
            } else{
                char subcamino[strlen(camino) + strlen(entradas[i].nombre) + 2];
                sprintf(subcamino, "%s%s/", camino, entradas[i].nombre);
                if(mi_rm_r(subcamino) == FALLO){ // Llamada recursiva para eliminar subdirectorios o ficheros
                fprintf(stderr, RED "Error al eliminar '%s'\n" RESET, subcamino);
                return FALLO;
                }
            }
        }
        offset += BLOCKSIZE;
    }

    // Finalmente eliminar el directorio vacío
    return mi_unlink(camino);
}

int mi_mv(const char *camino_origen, const char *camino_destino){
    if(mi_cp(camino_origen, camino_destino) == FALLO) {
        //fprintf(stderr, RED "Error al copiar '%s' a '%s'\n" RESET, camino_origen, camino_destino);
        return FALLO;
    }

    // Una vez copiado, eliminar el origen
    unsigned int p_inodo_origen, p_inodo_dir_origen = 0, p_entrada_origen;
    struct inodo inodo_origen;
    int error = buscar_entrada(camino_origen, &p_inodo_dir_origen, &p_inodo_origen, &p_entrada_origen, 0, 4);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    if (leer_inodo(p_inodo_origen, &inodo_origen) < 0) {
        fprintf(stderr, RED "Error al leer el inodo del origen '%s'\n" RESET, camino_origen);
        return FALLO;
    }
    if (inodo_origen.tipo == 'd') {
        // Si es un directorio, eliminarlo recursivamente
        if (mi_rm_r(camino_origen) == FALLO) {
            fprintf(stderr, RED "Error al eliminar el directorio '%s'\n" RESET, camino_origen);
            return FALLO;
        }
    } else {
        // Si es un fichero, eliminarlo directamente
        if (mi_unlink(camino_origen) == FALLO) {
            fprintf(stderr, RED "Error al eliminar el fichero '%s'\n" RESET, camino_origen);
            return FALLO;
        }
    }
    return EXITO;
}