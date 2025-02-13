/* Autores: Xavier Campos, Pedro Félix, Harpo Joan*/

#include <stdio.h>  //printf(), fprintf(), stderr, stdout, stdin
#include <fcntl.h> //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> //S_IRUSR, S_IWUSR
#include <stdlib.h>  //exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <unistd.h> // SEEK_SET, read(), write(), open(), close(), lseek()
#include <errno.h>  //errno
#include <string.h> // strerror()

// Constantes
#define BLOCKSIZE 1024 // bytes

#define EXITO 0 //para gestión errores
#define FALLO -1 //para gestión errores

#define BLACK   "\x1B[30m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1B[37m"
#define ORANGE  "\x1B[38;2;255;128;0m"
#define ROSE    "\x1B[38;2;255;151;203m"
#define LBLUE   "\x1B[38;2;53;149;240m"
#define LGREEN  "\x1B[38;2;17;245;120m"
#define GRAY    "\x1B[38;2;176;174;174m"
#define RESET   "\x1b[0m"

#define NEGRITA "\x1b[1m"

#define DEBUGN1 0
#define DEBUGN2 0
#define DEBUGN3 0
#define DEBUGN4 0
#define DEBUGN5 0
#define DEBUGN6 0
#define DEBUGN7 0
#define DEBUGN8 0
#define DEBUGN9 0
#define DEBUGN10 0
#define DEBUGN11 0
#define DEBUGN12 0
#define DEBUGN13 0

#define ENTREGA_1 0
#define ENTREGA_2 0
#define ENTREGA_3 0

// Variables
static int descriptor = 0;

// Funciones
int bmount(const char *camino);
int bumount();
int bwrite(unsigned int nbloque, const void *buf);
int bread(unsigned int nbloque, void *buf);