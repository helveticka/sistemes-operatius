/**
 * @file directorios.h
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
*/

#include "ficheros.h"
#include <sys/time.h> // struct timeval

#define TAMNOMBRE 60 //tamaño del nombre de directorio o fichero, en Ext2 = 256
#define TAMENTRADA sizeof(struct entrada) // tamaño de la entrada
#define PROFUNDIDAD 32 //profundidad máxima del árbol de directorios
#define ERROR_CAMINO_INCORRECTO (-2)
#define ERROR_PERMISO_LECTURA (-3)
#define ERROR_NO_EXISTE_ENTRADA_CONSULTA (-4)
#define ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO (-5)
#define ERROR_PERMISO_ESCRITURA (-6)
#define ERROR_ENTRADA_YA_EXISTENTE (-7)
#define ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO (-8)
#define TAMFILA 100 // Tamaño de la fila del directorio
#define TAMBUFFER (TAMFILA*1000) // suponemos un máx de 1000 entradas, aunque debería ser SB.totInodos
#define USARCACHE 3 //0:sin caché, 1:última L/E, 2:tabla FIFO, 3:tabla LRU

struct entrada {
  char nombre[TAMNOMBRE];
  unsigned int ninodo;
};

struct ultimaEntrada {
  char camino[TAMNOMBRE * PROFUNDIDAD];
  int p_inodo;
  #if USARCACHE == 3 //tabla LRU
    struct timeval ultima_consulta;
  #endif
};


// Funciones
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo);
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos);
void mostrar_buscar_entrada(char *camino, char reservar);
void mostrar_error_buscar_entrada(int error);

int mi_dir(const char *camino, char *buffer, char tipo, char flag);
int mi_chmod(const char *camino, unsigned char permisos);
int mi_creat(const char *camino, unsigned char permisos);
int mi_stat(const char *camino, struct STAT *p_stat);

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes);
int mi_read(const char *camino, char *buf, unsigned int offset, unsigned int nbytes);

int mi_link(const char *camino1, const char *camino2);
int mi_unlink(const char *camino);

// Extras
int mi_rename(const char *camino, const char *nuevo);
int mi_cp_f(const char *camino_origen, const char *camino_destino);
int mi_cp(const char *camino_origen, const char *camino_destino);
int mi_mv(const char *camino_origen, const char *camino_destino);
int mi_rm_r(const char *camino);

// Adicionales
int comparar_timeval(struct timeval a, struct timeval b);