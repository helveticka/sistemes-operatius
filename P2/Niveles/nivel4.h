// Autores: Xavier Campos, Pedro Felix, Harpo Joan
#define _POSIX_C_SOURCE 200112L
// Librerías
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
// Debugs
#define DEBUGN1 0
#define DEBUGN2 0
#define DEBUGN3 0
#define DEBUGN4 1
// Constantes
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
const char PROMPT = '$';
#define N_JOBS 64
#define FOREGROUND 0
#define NINGUNO 'N'
#define EJECUTANDOSE 'E'
#define DETENIDO 'D'
#define FINALIZADO 'F'
// Variables globales
char *user;
char line[COMMAND_LINE_SIZE];
static char mi_shell[COMMAND_LINE_SIZE]; 
// Códigos de color
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
// Estructuras
struct info_job {
   pid_t pid;
   char estado; 
   char cmd[COMMAND_LINE_SIZE]; 
};
static struct info_job jobs_list [N_JOBS];
static char mi_shell[COMMAND_LINE_SIZE];
int active_jobs = 0;
char line[COMMAND_LINE_SIZE];
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
void reaper(int signum);
void ctrlc(int signum);