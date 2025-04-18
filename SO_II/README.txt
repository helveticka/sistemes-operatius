# README - SF capa de ficheros

## Autores:
- Xavier Campos
- Pedro Félix
- Harpo Joan

## Mejoras y Modificaciones Implementadas
A partir del pseudocódigo base, se han realizado las siguientes mejoras:

### 1. **Gestión de `b_reads` y `b_writes` en la recursividad**
Para calcular el total de lecturas (`b_reads`) y escrituras (`b_writes`) realizadas al final de la ejecución de `liberar_bloques_inodo()`, se ha modificado la función recursiva `liberar_indirectos_recursivo()`, añadiendo los parámetros `&b_reads` y `&b_writes`.
Gracias a estos punteros, se logra un seguimiento eficiente de estas operaciones sin perder información entre llamadas recursivas.

### 2. **Manejo de los saltos entre bloques lógicos**
Se ha añadido la capacidad de imprimir los saltos entre bloques lógicos mediante la introducción de las siguientes variables estáticas (estáticas para conservar los valores en la recursividad):

- `oldsBL[]` → Guarda los bloques lógicos correspondientes a los últimos bloques físicos tratados.
- `endBL[]` → Almacena los bloques lógicos finales de los bloques de punteros.
- `oldBL` → Mantiene el bloque lógico inicial del salto, facilitando su impresión.

Estos arreglos permiten garantizar que los saltos se impriman en el orden adecuado:
1. **Primero** se liberan e imprimen los bloques físicos.
2. **Después** se imprimen los saltos de bloques lógicos.

La posición de cada valor en `oldsBL[]` y `endBL[]` depende del nivel de punteros en el que se encuentra el bloque de punteros.

### 3. **Variables adicionales para optimización**
Se han añadido las siguientes variables estáticas para controlar el proceso de mensajes de liberación y saltos:

- `static long freeBL` → Almacena el bloque lógico correspondiente al bloque de datos liberado.
  - Esto es crucial para imprimir correctamente la liberación del bloque de punteros asociado.
- `static int old_nivel_punteros` → Mantiene el nivel de punteros previo en la recursividad.
- `int salto` → Controla si se debe imprimir un salto entre bloques o no.
