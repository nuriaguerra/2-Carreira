/*
 * cons.c - Consumidor do problema produtor-consumidor
 *
 * Nuria Guerra Casal y Adrián Gijón Yubero
 * 
 * Retira caracteres do buffer compartido co produtor (pila LIFO)
 * sen ningún semáforo, para demostrar a aparición de carreiras críticas.
 *
 * Compilar: gcc -o cons cons.c
 * Executar: ./cons        (nun terminal distinto ao do produtor)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TAMANO_BUFFER 10        // número máximo de elementos na pila
#define FICHEIRO "buffer.dat"   // mesmo ficheiro que usa o produtor

// Estrutura compartida co produtor (debe ser idéntica á de prod.c)
typedef struct {
    char pila[TAMANO_BUFFER];
    int  cima;
    int  rematou;
} DatosCompartidos;


/*
 * Retira o carácter na cima da pila LIFO e substitúe esa posición por '-'.
 *
 * Primeiro gárdase a posición actual en 'posicion' ANTES do sleep
 * faise sleep() para que o produtor pode cambiar cima neste intre
 * Posteriormente retírase pila[posicion], ponse '-' e actualízase cima
 *
 * Se o produtor modifica 'cima' o consumidor usará un índice vello --> carreira crítica.
 */
char remove_item(DatosCompartidos *datos) {

    // PASO 1: gardamos a posición ANTES do sleep, así o índice queda fixo
    int posicion = datos->cima - 1;
    printf("Consumidor ve cima=%d, e intenta retirar de pila[%d]\n", datos->cima, posicion);

    sleep(2); // <-- sleep() carreira crítica (aumentar sobe a probabilidade)

    //usamos 'posicion', ignorando calquera cambio de cima
    // Se o produtor cambiou cima durante o sleep --> carreira crítica garantida
    char caracter         = datos->pila[posicion]; // retira o carácter da posición gardada
    datos->pila[posicion] = '-';                   // marca a posición como baleira
    datos->cima           = posicion;              // actualiza sen ter en conta o que fixo o produtor

    printf("O consumidor retira '%c' de pila[%d], agora cima=%d\n", caracter, posicion, datos->cima);

    // Mostramos o estado completo da pila para ver a carreira visualmente
    printf("[Consumidor] Pila: [");
    for (int k = 0; k < TAMANO_BUFFER; k++)
        printf("%c", datos->pila[k]);
    printf("]  cima=%d\n\n", datos->cima);

    return caracter;
}


// Conta as vogais do carácter consumido
void consume_item(char caracter, int *a, int *e, int *i, int *o, int *u) {
    printf("Consumido: '%c'\n", caracter);

    // Sumamos a vogal se é o caso do carácter consumido
    char minuscula = tolower(caracter);
    if (minuscula == 'a') (*a)++;
    if (minuscula == 'e') (*e)++;
    if (minuscula == 'i') (*i)++;
    if (minuscula == 'o') (*o)++;
    if (minuscula == 'u') (*u)++;
}


int main(void) {

    // Agardamos a que o produtor cree o ficheiro de memoria compartida
    struct stat info;
    while (stat(FICHEIRO, &info) != 0) // stat devolve -1 se o ficheiro non existe
        sleep(1);
    sleep(1); // tempo extra para que o produtor inicialice a estrutura

    // Abrimos o ficheiro de memoria compartida creado polo produtor
    int fd = open(FICHEIRO, O_RDWR);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    // Mapeamos a memoria compartida no espazo do proceso
    DatosCompartidos *datos = mmap(
        NULL,                     // o sistema escolle a dirección de memoria
        sizeof(DatosCompartidos), // tamaño da zona a mapear
        PROT_READ | PROT_WRITE,   // permisos de lectura e escritura
        MAP_SHARED,               // cambios visibles para outros procesos
        fd, 0);                   // file descriptor e offset 0 (desde o inicio)

    close(fd); // pechamos o descriptor, a memoria xa está mapeada
    if (datos == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    // Contadores de vogais consumidas
    int cant_a = 0, cant_e = 0, cant_i = 0, cant_o = 0, cant_u = 0;

    printf("Consumidor iniciado  (buffer N=%d)\n\n", TAMANO_BUFFER);

    //agarda datos, retíraos e consúmeos
    while (1) {
        // Espera activa mentres o buffer está baleiro
        if (datos->cima == 0) {
            if (datos->rematou)
                break; // produtor rematou e buffer baleiro: saímos
            printf("Buffer baleiro (cima=%d), agardando...\n", datos->cima);
            sleep(1);
            continue;
        }

        char caracter = remove_item(datos);
        consume_item(caracter, &cant_a, &cant_e, &cant_i, &cant_o, &cant_u);
    }

    printf("\nConsumidor, vogais contadas:\n");
    printf("       a=%-3d e=%-3d i=%-3d o=%-3d u=%-3d\n", cant_a, cant_e, cant_i, cant_o, cant_u);

    munmap(datos, sizeof(DatosCompartidos));
    return EXIT_SUCCESS;
}