Autores: Xavier Campos, Pedro Félix, Harpo Joan

Para la implementación de la función de liberar_bloques_inodo se ha usado el pseudocódigo de la versión 2 (la versión recursiva), y se han añadido ciertas variables y fragmentos de código adicionales para los mensajes de depuración.

Debido a que al final de la ejecución de la función se deben indicar el total de breads y bwrites, a la función recursiva liberar_indirectos_recursivo se le han añadido los parámetros &b_reads y &b_writes (ambos punteros a enteros). De esta forma, solucionamos con un puntero el almacenamiento de la cantidad de lecturas y escrituras en la recursividad.

También se presenta el problema de imprimir los saltos entre bloques lógicos, esto ocasiona la declaración de las variables oldsBL[], endBL[] y oldBL (todos tipo long). 

Por un lado, el propósito de oldBL es el de guardar el bloque lógico inicial del salto. De esta forma, al imprimir el salto solo deberemos llamar a oldBL y a nBL que será el final del salto.

Por otro lado, asumiendo las condiciones de que cuando se acaben los bloques de punteros primero se deben imprimir las liberaciones de sus bloques físicos, y posteriormente los saltos de bloques lógicos (desde el bloque lógico donde se encontraba el último bloque físico tratado al último bloque lógico del bloque de punteros); hemos tenido que crear los arreglos oldsBL[] y endBL[] para guardar precisamente los bloques lógicos de estos saltos finales de los bloques de punteros. En oldsBL guardamos los bloques lógicos correspondientes a los últimos bloques físicos tratados, y en endBL[] los bloques lógicos finales de los bloques de punteros. Las posiciones de estos valores en los arreglos dependen del nivel de punteros en el que se encuentre el bloque de punteros. Además, cabe añadir que para solucionar que entre llamadas recursivas se pierda la información por ser variables locales, se han declarado como variables estáticas.

Finalmente, también se han añadido las variables freeBL, old_nivel_punteros y salto.

static long freeBL --> para guardar el valor del bloque lógico correspondiente al bloque de datos liberado. Es información necesaria para la impresión de la liberación del bloque de punteros

static int old_nivel_punteros --> guarda el nivel de punteros más bajo al que hemos tenido que acceder. Esto es importante para los mensajes de saltos finales de bloques de punteros. Como tenemos que imprimir estos saltos al final deberemos acordarnos cuantos niveles hemos bajado.

int salto --> tiene el propósito de ser un booleano y controla la impresión de mensajes de salto. Esta variable es necesaria para no imprimir los saltos entre diferentes niveles de indirectos y cuando el salto es inmediato.


