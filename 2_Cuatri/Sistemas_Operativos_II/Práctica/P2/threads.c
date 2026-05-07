/*
 * threads.c - Solución con semáforos del problema productor-consumidor utilizando hilos
 *
 * Nuria Guerra Casal y Adrián Gijón Yubero
 *
 * Usa tres semáforos para eliminar as carreiras críticas:
 *   vacias : conta os ocos libres no buffer (valor inicial = N)
 *   cheas  : conta os elementos listos para consumir (valor inicial = 0)
 *   mutex  : garante acceso exclusivo á rexión crítica (valor inicial = 1)
 *
 * A diferenza clave respecto ao apartado 2 (procesos) é que os threads
 * comparten a memoria directamente, polo que non fai falta mmap() nin
 * semáforos con nome: úsase sem_init() en lugar de sem_open().
 *
 * Compilar: gcc -o threads threads.c -pthread
 * Executar: ./threads fichero.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define TAMANO_BUFFER 10    // tamaño da pila LIFO
#define ITERACIONES   80    // número de iteracións de cada thread

//Buffer compartido: variable global, visible directamente por ambos threads 
//(non fai falta mmap porque os threads comparten o espacio de memoria)
typedef struct {
    char pila[TAMANO_BUFFER];
    int  cima;
} DatosCompartidos;

DatosCompartidos datos; // variable global compartida entre productor e consumidor

//Semáforos globales compartidos entre threads.
sem_t vacias; // ocos libres no buffer (valor inicial = TAMANO_BUFFER)
sem_t cheas;  // elementos listos para consumir (valor inicial = 0)
sem_t mutex;  // exclusión mutua na rexión crítica (valor inicial = 1)

// Contadores de vogais: variables globais para que o main poida imprimilos
int prod_a=0, prod_e=0, prod_i=0, prod_o=0, prod_u=0;
int cons_a=0, cons_e=0, cons_i=0, cons_o=0, cons_u=0;


void produce_item(char c) {
    printf("[Productor] Le: '%c'\n", c);

    char m = tolower(c);
    if (m == 'a') prod_a++;
    if (m == 'e') prod_e++;
    if (m == 'i') prod_i++;
    if (m == 'o') prod_o++;
    if (m == 'u') prod_u++;
}


void insert_item(char c) {
    sem_wait(&vacias); //agarda oco libre; bloquea se o buffer está cheo
    sem_wait(&mutex);  //entra en exclusión mutua

    // rexión crítica: acceso exclusivo ao buffer
    datos.pila[datos.cima] = c;
    datos.cima++;

    printf("[Productor] Inserido '%c' en pila[%d]  cima=%d\n", c, datos.cima - 1, datos.cima);
    printf("[Productor] Pila: [");
    for (int k = 0; k < TAMANO_BUFFER; k++){
        printf("%c", datos.pila[k]);
    }
    printf("]  cima=%d\n\n", datos.cima);
    // fin rexión crítica

    sem_post(&mutex); //sae da exclusión mutua
    sem_post(&cheas); //avisa ao consumidor de que hai un elemento
}


// función principal do fio productor
void *productor(void *arg) {
    char *nombre_fichero = (char *)arg;

    FILE *f = fopen(nombre_fichero, "r");
    if (!f) { 
        perror("fopen"); 
        pthread_exit(NULL); 
    }

    srand(time(NULL)); // semente para velocidade aleatoria

    for (int iter = 0; iter < ITERACIONES; iter++) {

        int c = fgetc(f);
        if (c == EOF) { 
            rewind(f); 
            c = fgetc(f); 
        } // volta ao inicio se chega ao EOF

        produce_item((char)c);
        insert_item((char)c);
        if      (iter < 30) { 
            //sen sleep
        } else if (iter < 60) {
            sleep(3); //produtor lento: buffer énchese
        } else                { 
            sleep(rand() % 4); //velocidade aleatoria entre 0 e 3 segundos
        }
    }

    fclose(f);
    pthread_exit(NULL); //o produtor remata
}


char remove_item(void) {
    sem_wait(&cheas);  //agarda elemento dispoñible, bloquea se buffer baleiro
    sem_wait(&mutex);  //entra en exclusión mutua

    // rexión crítica: acceso exclusivo ao buffer
    datos.cima--;
    int pos = datos.cima;
    char c = datos.pila[pos];
    datos.pila[pos] = '-'; // substituímos por '-'

    printf("[Consumidor] Retirado '%c' de pila[%d]  cima=%d\n", c, pos, datos.cima);
    printf("[Consumidor] Pila: [");

    for (int k = 0; k < TAMANO_BUFFER; k++){
        printf("%c", datos.pila[k]);
    }
    printf("]  cima=%d\n\n", datos.cima);
    //fin rexión crítica

    sem_post(&mutex);  //sae da exclusión mutua
    sem_post(&vacias); //avisa ao produtor de que hai un oco libre

    return c;
}


void consume_item(char c) {
    printf("[Consumidor] Consumido: '%c'\n", c);

    char m = tolower(c);
    if (m == 'a') cons_a++;
    if (m == 'e') cons_e++;
    if (m == 'i') cons_i++;
    if (m == 'o') cons_o++;
    if (m == 'u') cons_u++;
}


void *consumidor(void *arg) {
    (void)arg; //non usa argumentos

    srand(time(NULL) + 1); //semente distinta á do produtor

    for (int iter = 0; iter < ITERACIONES; iter++) {

        char c = remove_item();
        consume_item(c);

        if      (iter < 30) { 
            sleep(3); 
        } else if (iter < 60) { 
            //sen sleep
        } else { 
            sleep(rand() % 4); 
        }
    }

    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <ficheiro>\n", argv[0]);
        return EXIT_FAILURE;
    }

    //Inicializamos o buffer con guións e a cima a 0
    datos.cima = 0;
    for (int i = 0; i < TAMANO_BUFFER; i++){
        datos.pila[i] = '-';
    }

    /*
     * Inicializamos os semáforos con sem_init().
     *   pshared=0: o semáforo compártese entre threads do mesmo proceso.
     *   pshared=1: compártese entre procesos (requeriría mmap).
     * Non fai falta sem_unlink() porque os semáforos sen nome non persisten
     * no sistema de ficheiros tras rematar o programa.
     */

    sem_init(&vacias, 0, TAMANO_BUFFER); // ocos libres = N
    sem_init(&cheas,  0, 0); // elementos dispoñibles = 0
    sem_init(&mutex,  0, 1);  // mutex libre = 1

    pthread_t th_prod, th_cons;

    //Creamos os dous threads
    pthread_create(&th_prod, NULL, productor, argv[1]); //o produtor recibe o nome do ficheiro como argumento
    pthread_create(&th_cons, NULL, consumidor, NULL);

    //Agardamos a que ambos threads rematen antes de imprimir os resultados
    pthread_join(th_prod, NULL);
    pthread_join(th_cons, NULL);

    //Destruímos os semáforos para liberar recursos
    sem_destroy(&vacias);
    sem_destroy(&cheas);
    sem_destroy(&mutex);

    //Imprimimos os conteos finais de ambos threads
    printf("\n[Productor] Vogais lidas:\n");
    printf("       a=%-3d e=%-3d i=%-3d o=%-3d u=%-3d\n", prod_a, prod_e, prod_i, prod_o, prod_u);

    printf("\n[Consumidor] Vogais consumidas:\n");
    printf("       a=%-3d e=%-3d i=%-3d o=%-3d u=%-3d\n", cons_a, cons_e, cons_i, cons_o, cons_u);

    return EXIT_SUCCESS;
}