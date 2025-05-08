/**
 * @file directorios.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"

static struct UltimaEntrada UltimaEntradaEscritura = {"", -1};

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
        // Hay al menos un segundo '/'
        size_t len = segundo_slash - (camino + 1); // longitud de inicial
        strncpy(inicial, camino + 1, len);
        inicial[len] = '\0';  // terminador nulo
        strcpy(final, segundo_slash); // desde el segundo slash incluido

        // Si el final es solo "/" => directorio final
        if (strcmp(final, "/") == 0) {
            *tipo = 'd';
        } else {
            *tipo = 'd'; // intermedio aún, pero sigue siendo directorio
        }
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
 * @return EXITO si se encuentra la entrada, ERROR_NO_EXISTE_ENTRADA_CONSULTA si no existe
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
            fprintf(stderr, "No se puede modificar la raíz\n");
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

    // Leer el inodo del directorio
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if ((inodo_dir.permisos & 4) != 4) {
        return ERROR_PERMISO_LECTURA;
    }
#if DEBUGN7
    fprintf(stderr, GRAY "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n"RESET, inicial, final, reservar);
#endif
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / TAMENTRADA;
    num_entrada_inodo = 0;
    offset = 0;
    // Inicializar el buffer de lectura
    struct entrada entradas [BLOCKSIZE / TAMENTRADA];
    memset(&entrada, 0, TAMENTRADA);
    memset(entradas, 0, sizeof(entradas)); // Inicializar el buffer de entradas

    cant_entradas_inodo = inodo_dir.tamEnBytesLog/sizeof(entrada);
    num_entrada_inodo = 0;

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
            // Simulación de leer la siguiente entrada
            }
        memcpy(&entrada, &entradas[num_entrada_inodo%(BLOCKSIZE/TAMENTRADA)], TAMENTRADA); // Copiar la entrada leída
        }
    }
    // Comprobar si la entrada existe o no
    if ((strcmp(inicial, entradas[num_entrada_inodo%(BLOCKSIZE/TAMENTRADA)].nombre) != 0) && num_entrada_inodo == cant_entradas_inodo) {
        // La entrada no existe
        switch (reservar) {
            case 0: // Modo consulta
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            case 1: //modo escritua
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
#if DEBUGN7
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

int mi_dir(const char *camino, char *buffer, char tipo, char flag) {
    struct entrada entrada;
    struct inodo inodo;
    unsigned int p_inodo;
    unsigned int p_entrada;
    int total = 0;
    char tmp[100];
    buffer[0] = '\0';  // vaciamos buffer

    // Buscar la entrada
    int error = buscar_entrada(camino, &p_inodo, &p_entrada, 0, 0, 0);
    if (error < 0) return error;

    // Leer el inodo de la entrada
    if (leer_inodo(p_inodo, &inodo) < 0) return -1;

    // Comprobación de sintaxis y tipo
    if (tipo == 'd' && inodo.tipo != 'd') {
        fprintf(stderr, "Error: la sintaxis no concuerda con el tipo (esperaba directorio)\n");
        return -1;
    }
    if (tipo == 'f' && inodo.tipo != 'f') {
        fprintf(stderr, "Error: la sintaxis no concuerda con el tipo (esperaba fichero)\n");
        return -1;
    }

    // Comprobar permisos de lectura
    if (!(inodo.permisos & 4)) {
        fprintf(stderr, "Error: no tienes permisos de lectura\n");
        return -1;
    }

    if (inodo.tipo == 'd') {  // Si es un directorio
        int n_entradas = inodo.tamEnBytesLog / sizeof(struct entrada);
        if (n_entradas == 0) return 0;

        strcat(buffer, "Total: ");
        sprintf(tmp, "%d\n", n_entradas);
        strcat(buffer, tmp);

        if (flag == 1) {  // Formato extendido estilo ls -l
            strcat(buffer, "Tipo\tPermisos\tmTime\t\t\tTamaño\tNombre\n");
            strcat(buffer, "-------------------------------------------------------------\n");
        }

        for (int i = 0; i < n_entradas; i++) {
            if (mi_read_f(p_inodo, &entrada, i * sizeof(struct entrada), sizeof(struct entrada)) != sizeof(struct entrada)) {
                fprintf(stderr, "Error al leer entrada %d\n", i);
                return -1;
            }

            struct inodo inodo_aux;
            int ninodo_aux = entrada.ninodo;

            if (leer_inodo(ninodo_aux, &inodo_aux) < 0) return -1;

            if (flag == 1) {  // Extendida
                // Tipo
                if (inodo_aux.tipo == 'd') strcat(buffer, "d\t");
                else strcat(buffer, "f\t");

                // Permisos
                strcat(buffer, (inodo_aux.permisos & 4) ? "r" : "-");
                strcat(buffer, (inodo_aux.permisos & 2) ? "w" : "-");
                strcat(buffer, (inodo_aux.permisos & 1) ? "x\t" : "-\t");

                // Fecha
                struct tm *tm;
                tm = localtime(&inodo_aux.mtime);
                sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t",
                    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                    tm->tm_hour, tm->tm_min, tm->tm_sec);
                strcat(buffer, tmp);

                // Tamaño
                sprintf(tmp, "%d\t", inodo_aux.tamEnBytesLog);
                strcat(buffer, tmp);

                // Nombre
                strcat(buffer, entrada.nombre);
                strcat(buffer, "\n");
            } else {  // Simple
                strcat(buffer, entrada.nombre);
                strcat(buffer, "\n");
            }
            total++;
        }
        return total;
    } else {  // Si es un fichero
        if (flag == 1) {  // Formato extendido para fichero
            strcat(buffer, "Tipo\tPermisos\tmTime\t\t\tTamaño\tNombre\n");
            strcat(buffer, "-------------------------------------------------------------\n");

            // Tipo
            strcat(buffer, "f\t");

            // Permisos
            strcat(buffer, (inodo.permisos & 4) ? "r" : "-");
            strcat(buffer, (inodo.permisos & 2) ? "w" : "-");
            strcat(buffer, (inodo.permisos & 1) ? "x\t" : "-\t");

            // Fecha
            struct tm *tm;
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);

            // Tamaño
            sprintf(tmp, "%d\t", inodo.tamEnBytesLog);
            strcat(buffer, tmp);

            // Nombre
            if (mi_read_f(p_inodo, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) != sizeof(struct entrada)) {
                fprintf(stderr, "Error al leer la entrada\n");
                return -1;
            }
            strcat(buffer, entrada.nombre);
            strcat(buffer, "\n");
        } else {  // Simple
            if (mi_read_f(p_inodo, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) != sizeof(struct entrada)) {
                fprintf(stderr, "Error al leer la entrada\n");
                return -1;
            }
            strcat(buffer, entrada.nombre);
            strcat(buffer, "\n");
        }
        return 1;  // solo un fichero
    }
}


int mi_chmod(const char *camino, unsigned char permisos) {
    unsigned int p_inodo, p_inodo_dir, p_entrada;

    // Buscar la entrada
    int res = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (res < 0) {
        fprintf(stderr, "Error en buscar_entrada() para el camino '%s'\n", camino);
        return res;
    }

    // Cambiar los permisos del inodo
    if (mi_chmod_f(p_inodo, permisos) == FALLO) {
        fprintf(stderr, "Error en mi_chmod_f() para el inodo %d\n", p_inodo);
        return FALLO;
    }

    return EXITO;
}

int mi_stat(const char *camino, struct STAT *p_stat) {
    unsigned int p_inodo, p_inodo_dir, p_entrada;

    int resultado = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (resultado < 0) {
        fprintf(stderr, "Error en buscar_entrada() para el camino '%s'\n", camino);
        return resultado;
    }

    if (mi_stat_f(p_inodo, p_stat) == FALLO) {
        fprintf(stderr, "Error en mi_stat_f() para el inodo %d\n", p_inodo);
        return FALLO;
    }

    printf("Inodo: %d\n", p_inodo); // Mostrar número de inodo

    return EXITO; // 0
} 

int mi_creat(const char *camino, unsigned char permisos) {
    struct superbloque SB;
    unsigned int p_inodo, p_inodo_dir, p_entrada;
    int error;
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, "mi_creat(): error al leer el superbloque\n");
        return FALLO;
    }
    p_inodo_dir = SB.posInodoRaiz; 
    p_inodo = SB.posInodoRaiz;
    p_entrada = 0;
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    if (error < 0) {
        return error;
    } else {
        return EXITO;
    }
}

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo;
    unsigned int p_inodo_dir = 0;
    unsigned int p_entrada;

    // Comprobar si el camino está en caché
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0) {
        p_inodo = UltimaEntradaEscritura.p_inodo;
    } else {
        // No está en caché, buscar la entrada
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 2); // modo lectura+escritura
        if (error < 0) return error;

        // Actualizar la caché
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }

    // Llamar a mi_write_f de la capa de ficheros
    return mi_write_f(p_inodo, buf, offset, nbytes);
}

int mi_read(const char *camino, char *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo, p_inodo_dir, p_entrada;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (error < 0) {
        return FALLO;
    }
    return mi_read_f(p_inodo, buf, offset, nbytes);
}