// Autores: 

#define _POSIX_C_SOURCE 200112L

// Librerías
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

// Debugs
#define DEBUGN1 1

// Constantes
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64

char *user;
const char PROMPT = '$';
char line[COMMAND_LINE_SIZE];

#define RESET "\033[0m"
#define NEGRO "\x1b[30m"
#define GRIS "\x1b[94m"
#define ROJO "\x1b[31m"
#define VERDE "\x1b[32m"
#define AMARILLO "\x1b[33m"
#define AZUL "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define BLANCO "\x1b[97m"
#define NEGRITA "\x1b[1m"

// Funciones
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args); 
int internal_export(char **args); 
int internal_source(char **args); 
int internal_jobs(char **args); 
int internal_fg(char **args); 
int internal_bg(char **args); 
int internal_exit(char **args);
void print_prompt(void);