/**
 * @file semaforo_mutex_posix.h
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
 #include <stdlib.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <semaphore.h>
 
 
 #define SEM_NAME "/mymutex" /* Usamos este nombre para el semáforo mutex */
 #define SEM_INIT_VALUE 1 /* Valor inicial de los mutex */
 
 
 sem_t *initSem();
 void deleteSem();
 void signalSem(sem_t *sem);
 void waitSem(sem_t *sem);