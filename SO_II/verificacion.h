/**
 * @file verificacion.h
 * @authors Xavier Campos, Pedro Félix, Harpo Joan
 */
#include "simulacion.h"

struct INFORMACION {
    int pid;
    unsigned int nEscrituras;
    struct REGISTRO PrimeraEscritura;
    struct REGISTRO UltimaEscritura;
    struct REGISTRO MenorPosicion;
    struct REGISTRO MayorPosicion;
    struct timeval fecha;
};