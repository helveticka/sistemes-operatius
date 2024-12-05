// Autores: Xavier Campos, Pedro Felix, Harpo Joan
#include "nivel6.h"
/**
 * Método principal del archivo.
 */
int main(int argc, char *argv[]) {
    // Señales
    signal(SIGCHLD, reaper); 
    signal(SIGINT, ctrlc); 
    signal(SIGTSTP, ctrlz);
    // Inicialización de la lista de trabajos
    jobs_list[FOREGROUND].pid = 0;
    jobs_list[FOREGROUND].estado = NINGUNO;
    memset(jobs_list[FOREGROUND].cmd, '\0', sizeof(jobs_list[FOREGROUND].cmd));
    strcpy(mi_shell,argv[0]);
    // Bucle para leer líneas y ejecutarlas
    while (1) {
        if (read_line(line)) {
            execute_line(line);
        }
    }
    return EXIT_SUCCESS;
}
/**
 Imprime el prompt y lee una linea de consola.
 *line: puntero donde se almacenará la linea leída.
 return: puntero a la línea leída.
 */
char *read_line(char *line) {
    print_prompt();
    fflush(stdout);
    memset(line, '\0', COMMAND_LINE_SIZE);
    char *ptr = fgets(line,COMMAND_LINE_SIZE,stdin);
    // Lee una línea desde stdin
    if(ptr != NULL){
        // Sustituye el carácter '\n' por '\0'
        line[strlen(line) - 1] = '\0';
    } else{
        // Verifica si fue por Ctrl + D
        if(feof(stdin)){
#if DEBUGN1
            fprintf(stderr, GRIS "\rAdiós\n" RESET);
#endif
            exit(EXIT_SUCCESS);
        }
    }
    return ptr;
}
/**
 Ejecuta una línea de comandos.
 *line: puntero a la línea de comandos.
 return: 0 si todo fue bien, -1 si hubo algún error.
 */
int execute_line(char *line) {
    char *args[ARGS_SIZE];
    int numArgs = parse_args(args, line);
    if (numArgs > 0) {
        // Crear un string con el comando para mostrarlo en los mensajes
        char command[COMMAND_LINE_SIZE] = "";
        for (size_t i = 0; i < numArgs; i++) {
            strcat(command, args[i]);
            if (i < numArgs - 1) {
                strcat(command, " ");
            }
        }
        command[COMMAND_LINE_SIZE - 1] = '\0';
        // Verificar si es un comando interno
        if (check_internal(args) == 0) {
            int bg = is_background(args);
            // Comando externo, crear proceso hijo
            pid_t pid = fork();
            if (pid < 0) {
                perror("Error en fork");
                return -1;
            }
            // Proceso hijo
            if (pid == 0) { 
                // Asignar acción por defecto a la señal SIGCHLD
                signal(SIGCHLD, SIG_DFL);
                // Ignorar la señal SIGINT en el proceso hijo
                signal(SIGINT, SIG_IGN);
                // Ignorar la señal SIGTSTP en el proceso hijo
                signal(SIGTSTP, SIG_IGN);
                // Comprobar si hay redirección de entrada
                is_output_redirection(args);
                // Ejecuta el comando externo
                if (execvp(args[0], args) < 0) {
                    // Si execvp falla, muestra el error y termina
                    fprintf(stderr, ROJO "%s: ", args[0]);
                    perror("");
                    printf(RESET);
                    exit(-1);
                }
                exit(0);
            // Proceso padre (minishell)
            } else if (pid > 0) {
                if(bg == 0){
                    // Actualizar jobs_list para el proceso en foreground
                    jobs_list[0].pid = pid;
                    strncpy(jobs_list[0].cmd, command, COMMAND_LINE_SIZE - 1);
                    jobs_list[0].cmd[COMMAND_LINE_SIZE - 1] = '\0';
                    jobs_list[0].estado = EJECUTANDOSE;
#if DEBUGN3 || DEBUGN4 || DEBUGN5
                    // Imprimir información de depuración
                    printf(GRIS"[execute_line()→ PID padre: %d (%s)]\n"RESET, getpid(), mi_shell);
                    printf(GRIS"[execute_line()→ PID hijo: %d (%s)]\n"RESET, pid, jobs_list[0].cmd);
#endif
                } else{
                    strncpy(jobs_list[0].cmd, command, COMMAND_LINE_SIZE - 1);
                    jobs_list[0].cmd[COMMAND_LINE_SIZE - 1] = '\0';
#if DEBUGN3 || DEBUGN4 || DEBUGN5
                    // Imprimir información de depuración
                    printf(GRIS"[execute_line()→ PID padre: %d (%s)]\n"RESET, getpid(), mi_shell);
                    printf(GRIS"[execute_line()→ PID hijo: %d (%s)]\n"RESET, pid, jobs_list[0].cmd);
#endif
                    jobs_list_add(pid, EJECUTANDOSE, command);
                }
                while (jobs_list[0].pid > 0) {
                    pause();
                }
            } else {
                fprintf(stderr, ROJO "fork: %s\n"RESET, strerror(errno));
                exit(-2);
            }
        }
    }
    return EXIT_SUCCESS;
}
/**
 Parsea una línea de comandos.
 *args: array de punteros a char donde se almacenarán los argumentos.
 *line: línea de comandos a parsear.
 return: número de argumentos leídos.
 */
int parse_args(char **args, char *line) {
    const char *delimiters = " \t\n\r";
    char *token;
    unsigned int counter = 0;
    token = strtok(line,delimiters);
    // Mientras haya tokens, se añaden al array de argumentos
    while(token != NULL){
        if(token[0] == '#'){
#if DEBUGN1
            fprintf(stderr, GRIS "[parse_args()→ token %d: %s]\n" RESET, counter,token);
#endif
            token = NULL;
            args[counter] = token;
#if DEBUGN1
            fprintf(stderr, GRIS "[parse_args()→ token %d corregido: %s]\n" RESET, counter,token);
#endif
            return counter;
        }
        args[counter] = token;
#if DEBUGN1
        fprintf(stderr, GRIS "[parse_args()→ token %d: %s]\n" RESET, counter,token);
#endif
        counter++;
        token = strtok(NULL,delimiters);
    }
    // Se añade NULL al final del array de argumentos
    args[counter] = NULL;
#if DEBUGN1
    fprintf(stderr, GRIS "[parse_args()→ token %d: %s]\n" RESET, counter, token);
#endif
    return counter;
}
/**
 Checkea si el comando es interno.
 *args: array de punteros a char con los argumentos introducidos.
 return: 1 si es interno, 0 si no lo es.
 */
int check_internal(char **args) {
    int command = 0;
    // Si no hay argumentos, no se hace nada
    if (args == NULL || args[0] == NULL) {
        return -1;
    }
    else if (!strcmp(args[0], "cd")) {
        internal_cd(args);
        command = 1;
    }
    else if (!strcmp(args[0], "export")) {
        internal_export(args);
        command = 1;
    }
    else if (!strcmp(args[0], "source")) {
        internal_source(args);
        command = 1;
    }
    else if (!strcmp(args[0], "jobs")) {
        internal_jobs(args);
        command = 1;
    }
    else if (!strcmp(args[0], "fg")) {
        internal_fg(args);
        command = 1;
    }
    else if (!strcmp(args[0], "bg")) {
        internal_bg(args);
        command = 1;
    }
    else if (!strcmp(args[0], "exit")) {
        exit(0);
        return EXIT_SUCCESS;
    }
    return command;
}
/**
 Cambia de directorio.
 *args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien, -1 si hubo algún error.
 */
int internal_cd(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_cd()→ Esta función cambiará de directorio]\n" RESET);
#endif
    // Obtener el directorio HOME
    char *home_dir = getenv("HOME");    
    // Buffer para el directorio actual
    char current_dir[1024];           
    int result = -1;
    // Caso 1: Sin argumentos (ir al directorio HOME)
    if (args[1] == NULL) {
        if (home_dir == NULL) {
            fprintf(stderr, ROJO "Error: la variable HOME no está definida.\n" RESET);
            return EXIT_FAILURE;
        }
        result = chdir(home_dir);
    }
    // Caso 2: Un solo argumento (directorio especificado en args[1])
    else if (args[2] == NULL) {
        result = chdir(args[1]);
    }
    // Verificar si `chdir()` tuvo éxito
    if (result != 0) {
        // Si `chdir` falla, imprime el error y retorna -1
        perror(ROJO "chdir() error");
        printf(RESET);
        return EXIT_FAILURE;
    }
    // Obtener el directorio actual después del cambio
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror(ROJO "chdir() error");
        printf(RESET);
        return EXIT_FAILURE;
    }
#if DEBUGN2
    fprintf(stderr, GRIS "[internal_cd()→ PWD: %s]\n" RESET, current_dir);
#endif
    return EXIT_SUCCESS;
}
/**
 Asigna valores a variables de entorno.
 *args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien, -1 si hubo algún error.
 */
int internal_export(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_export()→ Esta función asignará valores a variables cd de entorno]\n" RESET);
#endif
    // Verificar si el número de argumentos es adecuado (debe tener args[1])
    if (args[1] == NULL || args[2] != NULL) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return EXIT_FAILURE;
    }
    // Buscar el primer '=' en args[1]
    char *equal_sign = strchr(args[1], '=');
    if (equal_sign == NULL) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return EXIT_FAILURE;
    }
    // Dividir en nombre y valor
    size_t name_len = equal_sign - args[1];
    char *name = (char *)malloc(name_len + 1);
    strncpy(name, args[1], name_len);
    name[name_len] = '\0';
    char *value = equal_sign + 1;
    // Verificar si el formato es correcto (debe haber un nombre y un valor)
    if (name == NULL || value == NULL) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return EXIT_FAILURE;
    }
    // Mostrar el valor inicial de la variable de entorno
    char *initial_value = getenv(name);
    if (initial_value != NULL) {
#if DEBUGN2
        printf(GRIS "[internal_export()→ nombre: %s]\n" RESET, name);
        printf(GRIS "[internal_export()→ valor: %s]\n" RESET, value);
        printf(GRIS "[internal_export()→ antiguo valor para %s: %s\n" RESET, name, initial_value);
#endif
    } else {
        fprintf(stderr, ROJO "Error: la variable %s no estaba definida.\n", name);
    }
    // Asignar el nuevo valor a la variable de entorno
    if (setenv(name, value, 1) != 0) {
        perror("setenv() error");
        return EXIT_FAILURE;
    }
    // Mostrar el nuevo valor de la variable de entorno
    char *new_value = getenv(name);
    if (new_value != NULL) {
#if DEBUGN2
        printf(GRIS "[internal_export()→ nuevo valor para %s: %s\n" RESET, name, new_value);
#endif
    } else {
        fprintf(stderr, ROJO "Error: la variable %s no se estableció correctamente.\n" RESET, name);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
/**
 Ejecuta un fichero de líneas de comandos.
 *args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien
 */
int internal_source(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_source()→ Esta función ejecutará un fichero de líneas de comandos]\n" RESET);
#endif
    // Comprobamos si el comando source tiene un argumento
    if (!args[1]) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: source <nombre fichero>\n" RESET);
        return EXIT_FAILURE;
    }
    // Abrimos el fichero en modo lectura
    char buffer[COMMAND_LINE_SIZE];
    FILE *file = fopen(args[1], "r");
    // Comprobamos si el fichero se ha abierto correctamente
    if (!file) {  
        fprintf(stderr, ROJO "fopen: %s\n" RESET, strerror(errno));
        return (EXIT_FAILURE);
    }
    // Leemos el fichero línea a línea
    while (fgets(buffer, COMMAND_LINE_SIZE, file) != NULL) {
        for (int i = 0; i < COMMAND_LINE_SIZE; i++) {
            if (buffer[i] == '\n') { 
                buffer[i] = '\0';
            }
        }
#if DEBUGN3        
        fprintf(stderr, GRIS "[internal_source() → LINE: %s]\n" RESET, buffer);
#endif       
        // Ejecutamos la línea
        fflush(file);
        execute_line(buffer); 
    }
    // Cerramos el fichero
    fclose(file); 
    return EXIT_FAILURE;
} 
/**
 Captura la señal SIGINT (Ctrl+C) y la envía al proceso en foreground
 signum: número de la señal
 */
void ctrlc(int signum) {
    // Reasignar signal() para capturar futuras señales SIGINT
    signal(SIGINT, ctrlc);
    // Imprimir un salto de línea para mantener un prompt limpio
    printf("\n");
    fflush(stdout);
#if DEBUGN5
    printf(GRIS"[ctrlc()→ Soy el proceso con PID %d (./nivel5), el proceso en foreground es %d (%s)]\n"RESET, getpid(),jobs_list[0].pid, jobs_list[0].cmd);
    printf(GRIS"[ctrlc()→ recibida señal %d (SIGINT)]\n"RESET, SIGINT);
#endif
    // Verificar si hay un proceso en foreground
    if (jobs_list[0].pid > 0) {
        // Obtener el nombre del comando en ejecución en foreground
        char *foreground_cmd = jobs_list[0].cmd;
        // Verificar que el proceso en foreground NO sea el shell actual
        if (strncmp(foreground_cmd, mi_shell, strlen(mi_shell)) != 0) {
            // Enviar la señal SIGTERM al proceso en foreground
            if (kill(jobs_list[0].pid, SIGTERM) == 0) {
#if DEBUGN5
                printf(GRIS"[ctrlc()→ Señal %d (SIGTERM) enviada a %d (%s) por %d (./nivel5)]\n"RESET, SIGTERM, jobs_list[0].pid, jobs_list[0].cmd, getpid());
#endif
            }
        } else {
#if DEBUGN5
            printf(GRIS"[ctrlc()→ Señal %d (SIGTERM) no enviada por %d (./nivel5) debido a que el proceso en foreground es el shell]\n"RESET, SIGTERM, getpid());
#endif
        }
    } else {
#if DEBUGN5
        printf(GRIS"[ctrlc()→ Señal %d (SIGTERM) no enviada por %d (./nivel5) debido a que no hay proceso en foreground]\n"RESET, SIGTERM, getpid());
#endif
    }
    // Asegurar la impresión inmediata del mensaje
    fflush(stdout); 
}
/**
 Captura la señal SIGTSTP (Ctrl+Z) y la envía al proceso en foreground
 signum: número de la señal
 */
void ctrlz(int signum) {
    signal(SIGTSTP, ctrlz);
#if DEBUGN4 || DEBUGN5
    printf(GRIS"\n[ctrlz()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n"RESET, getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
    printf(GRIS"[ctrlz()→ recibida señal %d (SIGTSTP)]\n"RESET, signum);
#endif
    // Reasignar signal() para capturar futuras señales SIGTSTP
    if (jobs_list[0].pid > 0) {
        if (strcmp(jobs_list[0].cmd, mi_shell)) {
            // Enviar la señal SIGSTOP al proceso en foreground
            kill(jobs_list[0].pid, SIGSTOP);
#if DEBUGN4 || DEBUGN5
            printf(GRIS"[ctrlz()→ Señal %d (SIGSTOP) enviada a %d (%s) por %d (%s)]\n"RESET, signum, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
#endif
            // Actualizamos el proceso detenido y lo añadimos a la lista de jobs
            jobs_list[0].estado = DETENIDO;
            jobs_list_add(jobs_list[0].pid, jobs_list[0].estado, jobs_list[0].cmd);
            // Actualizamos el foreground con sus propiedades de serie (Reset)
            jobs_list[0].pid = 0;
            jobs_list[0].estado = NINGUNO;
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
        } else {
#if DEBUGN4 || DEBUGN5
            printf(GRIS"[ctrlz()→ Señal %d (SIGSTOP) no enviada por %d (%s) debido a que su proceso en foreground es el shell]\n"RESET, signum, getpid(), mi_shell);
#endif
        }
    } else {
#if DEBUGN4 || DEBUGN5
        printf(GRIS"[ctrlz()→ Señal %d (SIGSTOP) no enviada por %d (%s) debido a que no hay proceso en foreground]\n"RESET, signum, getpid(), mi_shell);
#endif
    }
    fflush(stdout);
}
/**
 Captura la señal SIGCHLD y maneja los procesos hijos que han terminado.
 signum: número de la señal
 */
void reaper(int signum) {
    int status;
    pid_t ended;
    // Reasignar la señal SIGCHLD al manejador
    signal(SIGCHLD, reaper);
    // Procesar todos los hijos que hayan terminado
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0) {
        // Si el hijo terminado es el proceso en primer plano
        if (ended == jobs_list[0].pid) {
            // Mostrar información del hijo terminado
            if (WIFEXITED(status)) {
#if DEBUGN4 || DEBUGN5
                printf(GRIS"[reaper()→ recibida señal %d (SIGCHLD)]\n"RESET,SIGCHLD);
                printf(GRIS"[reaper()→ Proceso hijo %d en foreground (%s) finalizado con exit code %d]\n"RESET, ended, jobs_list[0].cmd, WEXITSTATUS(status));
#endif
            } else if (WIFSIGNALED(status)) {
#if DEBUGN4 || DEBUGN5
                printf(GRIS"[reaper()→ recibida señal %d (SIGCHLD)]\n"RESET,SIGCHLD);
                printf(GRIS"[reaper()→ Proceso hijo %d en foreground (%s) finalizado por señal %d]\n"RESET, ended, jobs_list[0].cmd, WTERMSIG(status));
#endif        
            }
            // Resetear el proceso en primer plano
            jobs_list[0].pid = 0;
            jobs_list[0].estado = FINALIZADO;
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
        } else{
            int pos = jobs_list_find(ended);
            // Mostrar información del hijo terminado
            if (WIFEXITED(status)) {
#if DEBUGN4 || DEBUGN5
                printf(GRIS"[reaper()→ recibida señal %d (SIGCHLD)]\n"RESET,SIGCHLD);
                printf(GRIS"[reaper()→ Proceso hijo %d en background (%s) finalizado con exit code %d]\n"RESET, ended, jobs_list[pos].cmd, WEXITSTATUS(status));
#endif
            } else if (WIFSIGNALED(status)) {
#if DEBUGN4 || DEBUGN5
                printf(GRIS"[reaper()→ recibida señal %d (SIGCHLD)]\n"RESET,SIGCHLD);
                printf(GRIS"[reaper()→ Proceso hijo %d en background (%s) finalizado por señal %d]\n"RESET, ended, jobs_list[pos].cmd, WTERMSIG(status));
#endif        
            }
            printf("Terminado PID %d (%s) en jobs_list[%d] con status %d\n", ended, jobs_list[pos].cmd, pos, jobs_list[pos].estado);
            // Eliminar el trabajo de la lista de trabajos
            jobs_list_remove(pos);
        }
    }
    // Manejar errores de waitpid
    if (ended == -1 && errno != ECHILD) {
        perror("Error en waitpid");
    }
}
/**
 Muestra los procesos que no están en foreground.
 *args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien
 */
int internal_jobs(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_jobs()→ Esta función mostrará el PID de los procesos que no estén en foreground]\n" RESET);
#endif
    for (int i = 1; i < N_JOBS; i++) {
        if (jobs_list[i].pid > 0) {
            fprintf(stderr, "[%d] %d	%c		%s\n", i, jobs_list[i].pid, jobs_list[i].estado, jobs_list[i].cmd);
        }
    }
    return EXIT_SUCCESS;
}
/**
 Trae los procesos más recientes al primer plano.
 *args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien
 */
int internal_fg(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_fg()→ Esta función traerá los procesos más recientes al primer plano]\n" RESET);
#endif
    // Checkear la sintaxis del comando
    if (args[1] == NULL) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: fg <numero trabajo>\n" RESET);
        return EXIT_FAILURE;
    }
    // Obtener la posición del trabajo en la lista de trabajos
    int pos = atoi(args[1]);
    // Verificar si el trabajo existe
    if ((pos > n_job) || (pos <= 0)) {
        fprintf(stderr, ROJO "fg %d: no existe ese trabajo\n" RESET, pos);
        return EXIT_FAILURE;
    }
    // Enviar la señal SIGCONT al proceso si está detenido
    if (jobs_list[pos].estado == DETENIDO) {
        kill(jobs_list[pos].pid, SIGCONT);
#if DEBUGN6
            fprintf(stderr, GRIS "[internal_fg()→ Señal %d (SIGCONT) enviada a %d (%s)]\n" RESET, SIGCONT, jobs_list[pos].pid, jobs_list[pos].cmd);
#endif
    }
    // Eliminar el token "&" del comando
    for (int i=0; i < strlen(jobs_list[pos].cmd); i++) {
        if (jobs_list[pos].cmd[i] == '&') {
            jobs_list[pos].cmd[i - 1] = '\0';
        }
    }
    // Actualizar el estado del trabajo a EJECUTANDOSE
    jobs_list[pos].estado = EJECUTANDOSE;
    // Copiar los datos del trabajo a jobs_list[0]
    jobs_list[0] = jobs_list[pos];
    // Eliminar el trabajo de la lista de trabajos
    jobs_list_remove(pos);
    // Imprimir el comando en ejecución sin el token "&"
    fprintf(stderr, "%s\n", jobs_list[0].cmd);
    // Esperar a que el proceso en primer plano termine
    while (jobs_list[0].pid > 0) {
        pause();
    }
    return EXIT_SUCCESS;

}
/**
 Muestra los procesos parados o en segundo plano.
 *args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien
 */
int internal_bg(char **args) {
#if DEBUGN1
    fprintf(stderr, GRIS "[internal_bg()→ Esta función mostrará los procesos parados o en segundo plano]\n" RESET);
#endif
    // Checkear la sintaxis del comando
    if (args[1] == NULL) {
        fprintf(stderr, ROJO "Error de sintaxis. Uso: bg <numero trabajo>\n" RESET);
        return EXIT_FAILURE;
    }
    // Obtener la posición del trabajo en la lista de trabajos
    int pos = atoi(args[1]);
    // Verificar si el trabajo existe
    if ((pos > n_job) || (pos <= 0)) {
        fprintf(stderr, ROJO "bg %d: no existe ese trabajo\n" RESET, pos);
        return EXIT_FAILURE;
    }
    // Enviar la señal SIGCONT al proceso si está detenido
    if (jobs_list[pos].estado == EJECUTANDOSE) {
        fprintf(stderr, ROJO "bg %d: el trabajo ya está en segundo plano\n" RESET, pos);
        return EXIT_FAILURE;
    }
    // Añadir a "&" al final del comando
    strcat(jobs_list[pos].cmd, " &");
    // Actualizar el estado del trabajo a EJECUTANDOSE
    jobs_list[pos].estado = EJECUTANDOSE;
    // Enviar la señal SIGCONT
    kill(jobs_list[pos].pid, SIGCONT);
#if DEBUGN6
        fprintf(stderr, GRIS "[internal_bg()→ Señal %d (SIGCONT) enviada a %d (%s)]\n" RESET, SIGCONT, jobs_list[pos].pid, jobs_list[pos].cmd);
#endif
    // Imprimir el trabajo en segundo plano
    printf("[%d] %d	%c		%s\n", n_job, jobs_list[n_job].pid, jobs_list[n_job].estado, jobs_list[n_job].cmd);    
    return EXIT_SUCCESS;
}
/**
 Imprime el prompt.
 */
void print_prompt(void) {
    // Obtener el nombre de usuario
    user = getenv("USER");
    char path[200];
    getcwd(path, sizeof(path));
    // Prints del prompt
    printf(NEGRITA MAGENTA "%s" RESET, user);
    printf(NEGRITA ":" RESET);
    printf(NEGRITA CYAN "MINISHELL" RESET);
    printf(NEGRITA "%c " RESET, PROMPT);
    fflush(stdout);
}
/**
 Busca en el array de trabajos el PID retorna su posición.
 pid: PID del proceso a buscar.
 return: posición del proceso en el array de trabajos.
 */
int jobs_list_find(pid_t pid) {
    for (int pos = 0; pos < N_JOBS; pos++) {
        if (jobs_list[pos].pid == pid) {
            return pos;
        }
    }
    return EXIT_FAILURE;
}
/**
 Elimina un trabajo de la lista de trabajos.
 pos: posición del trabajo a eliminar.
 return: 0 si todo fue bien
 */
int jobs_list_remove(int pos) {

    // Mover el último trabajo a la posición eliminada, si no es el mismo
    if (pos != n_job) {
        jobs_list[pos] = jobs_list[n_job];
    }

    // Limpiar el último trabajo
    jobs_list[n_job].pid = 0;
    jobs_list[n_job].estado = 0;
    jobs_list[n_job].cmd[0] = '\0';

    n_job--; // Decrementar el número de trabajos

    return 0; // Éxito
}
/**
 Comprueba si un comando se ejecutará en background.
 **args: array de punteros a char con los argumentos introducidos.
 return: 0 si todo fue bien, -1 si hubo algún error.
 */
int is_background(char **args) {
    int i = 0;
    // Recorrer el array de argumentos para buscar el token "&"
    while (args[i] != NULL) {
        if (strcmp(args[i], "&") == 0) {
            args[i] = NULL; // Sustituir el token "&" por NULL
            return 1;       // Es un comando en background
        }
        i++;
    }
    return 0; // No se encontró el token "&", es foreground
}
/**
 Añade un trabajo a la lista de trabajos.
 pid: PID del proceso a añadir.
 estado: estado del proceso a añadir.
 *cmd: comando del proceso a añadir.
 return: 0 si todo fue bien, -1 si hubo algún error.
 */
int jobs_list_add(pid_t pid, char estado, char *cmd) {
    // Incrementar el contador global de trabajos
    n_job++;
    // Verificar si se ha alcanzado el límite de trabajos
    if (n_job >= N_JOBS) {
        return -1; // Error
    }
    // Añadir el nuevo trabajo a la lista
    jobs_list[n_job].pid = pid;
    jobs_list[n_job].estado = estado;
    // Copiar el comando al campo correspondiente
    strncpy(jobs_list[n_job].cmd, cmd, COMMAND_LINE_SIZE - 1);
    jobs_list[n_job].cmd[COMMAND_LINE_SIZE - 1] = '\0'; // Asegurar el final nulo
    // Imprimir información del trabajo añadido
    printf("[%d] %d	%c		%s\n", n_job, jobs_list[n_job].pid, jobs_list[n_job].estado, jobs_list[n_job].cmd);
    // Éxito
    return 0; 
}

/**
 Recorre los argumentos buscando el token '>' 
 seguido del nombre de un fichero. Al encontrarlo, asocia
 el descriptor 1 a dicho fichero
 **args: array de punteros a char con los argumentos introducidos.
 return: TRUE (1) si ha funcionado, FALSE (0) en caso contrario
 */
int is_output_redirection(char **args) {
    int i = 0; // Índice para recorrer los argumentos
    int fd;    // Descriptor de fichero

    while (args[i] != NULL) {
        // Buscar el token '>'
        if (strcmp(args[i], ">") == 0) {
            // Comprobar que hay un argumento después del '>'
            if (args[i + 1] == NULL) {
                return 0;
            }

            // Intentar abrir el fichero en modo de escritura (crearlo si no existe, truncarlo si existe)
            fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                return 0;
            }

            // Redirigir la salida estándar (stdout) al fichero usando dup2
            if (dup2(fd, STDOUT_FILENO) < 0) {   
                close(fd);
                return 0;
            }

            // Cerrar el descriptor de fichero original
            close(fd);

            // Sustituir '>' por NULL para que execvp() no lo procese
            args[i] = NULL;

            // Sustituir el nombre del fichero también por NULL
            args[i + 1] = NULL;

            return 1;
        }
        i++; // Avanzar al siguiente argumento
    }

    return 0;
}