/**
 * @file directorios.c
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "directorios.h"

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    // Validar que comience por '/'
    if (camino[0] != '/') {
        fprintf(stderr, "Error: el camino no comienza por '/'\n");
        return -1;
    }

    const char *segundo_slash = strchr(camino + 1, '/');

    if (segundo_slash == NULL) {
        // Solo hay un componente => fichero
        strcpy(inicial, camino + 1);  // omitimos el primer '/'
        final[0] = '\0';              // cadena vacía
        *tipo = 'f';
    } else {
        // Hay más de un componente => directorio
        size_t len = segundo_slash - (camino + 1); // longitud de *inicial
        strncpy(inicial, camino + 1, len);
        inicial[len] = '\0'; // añadir terminador nulo
        strcpy(final, segundo_slash); // incluye el segundo '/'
        *tipo = 'd';
    }

    return 0;
}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
    struct entrada entrada;
    struct inodo inodo_dir;
    struct superbloque SB;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    unsigned cant_entradas_inodo, num_entrada_inodo;

    // Caso especial para la raíz
    if (strcmp(camino_parcial, "/") == 0) {
        *p_inodo = SB.posInodoRaiz; // El inodo de la raíz
        *p_entrada = 0;
        return EXITO;
    }

    // Extraer el camino
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO) {
        return ERROR_CAMINO_INCORRECTO;
    }

    // Leer el inodo del directorio
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if ((inodo_dir.permisos & 4) != 4) {
        return ERROR_PERMISO_LECTURA;
    }

    // Inicializar el buffer de lectura
    int TAMENTRADA = sizeof(struct entrada);
    int NUMENTRADAS = BLOCKSIZE / TAMENTRADA;
    struct entrada entradas [BLOCKSIZE / sizeof(struct entrada)];
    memset(&entrada, 0, sizeof(entrada));

    cant_entradas_inodo = inodo_dir.tamEnBytesLog/sizeof(entrada);
    num_entrada_inodo = 0;

    if (cant_entradas_inodo > 0) {
        // Simulación de la lectura de entradas
        while (num_entrada_inodo < cant_entradas_inodo && (strcmp(inicial, entradas[num_entrada_inodo%NUMENTRADAS].nombre) != 0)) {
            num_entrada_inodo++;
            // Simulación de leer la siguiente entrada
            memset(entradas, 0, sizeof(entrada)); // Inicializar nuevamente el buffer
        }
    }

    // Comprobar si la entrada existe o no
    if ((strcmp(inicial, entradas[num_entrada_inodo%NUMENTRADAS].nombre) != 0) && num_entrada_inodo == cant_entradas_inodo) {
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
                }

                // Copiar el nombre de la entrada
                strcpy(entradas[num_entrada_inodo%NUMENTRADAS].nombre, inicial);

                if (tipo == 'd') {
                    if (strcmp(final, "/") == 0) {
                        reservar_inodo(*p_inodo, permisos); // Reservamos un inodo como directorio
                    } else {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                } else {
                    reservar_inodo(*p_inodo, permisos); // Reservamos un inodo como fichero
                }

                // Escribir la entrada en el directorio
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * TAMENTRADA, TAMENTRADA) == FALLO) {
                    if (entrada.ninodo != -1) { // Si se había reservado un inodo
                        liberar_inodo(entrada.ninodo); // Liberar el inodo reservado 
                    }
                    return FALLO;
                }
        }
    }

    // Si hemos llegado al final del camino
    if (strcmp(final, "") == 0) {
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
    // fprintf(stderr, "Error: %d\n", error);
    switch (error) {
    case -2: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
    case -3: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
    case -4: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
    case -5: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
    case -6: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
    case -7: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
    case -8: fprintf(stderr, "Error: No es un directorio.\n"); break;
    }
}

int mi_stat(const char *camino, struct STAT *p_stat) {
    struct entrada entrada;
    unsigned int p_inodo, p_inodo_dir, p_entrada;

    int resultado = buscar_entrada(camino, &entrada, &p_inodo, &p_inodo_dir, 0, 0);
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
 