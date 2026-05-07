/*
 * cons_sem.c - Consumidor do problema produtor-consumidor CON SEMÁFOROS
 *
 * Nuria Guerra Casal y Adrián Gijón Yubero
 * 
 * Usa tres semáforos para eliminar as carreiras críticas:
 *   VACIAS : conta os ocos libres no buffer (valor inicial = N)
 *   CHEAS  : conta os elementos listos para consumir (valor inicial = 0)
 *   MUTEX  : garante acceso exclusivo á rexión crítica (valor inicial = 1)
 *
 * Compilar: gcc -o cons_sem cons_sem.c
 * Executar: ./cons_sem      (nun terminal distinto ao do produtor)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>

#define TAMANO_BUFFER 10    // tamaño da pila LIFO
#define ITERACIÓNS 80       // número de iteracións do bucle principal
#define FICHEIRO "buffer.dat" // ficheiro usado como memoria compartida

// Estrutura de memoria compartida
typedef struct {
    char pila[TAMANO_BUFFER];
    int  cima;
    //non fai falta a bandeira rematou porque ambos fai un número fixo de iteracións, así ambos rematan ao mesmo tempo
} DatosCompartidos;


//remove_item - retira o carácter da cima da pila e substitúe por '-'.

char remove_item(DatosCompartidos *datos, sem_t *vacias, sem_t *cheas, sem_t *mutex) {

    sem_wait(cheas);  //agarda a elemento dispoñible, bloquea se o buffer está baleiro, resta 1 a cheas
    sem_wait(mutex);  //entra na rexión crítica en exclusión mutua

    //rexión crítica: acceso exclusivo a datos compartidos (protexida polo mutex)
    datos->cima--;
    int pos = datos->cima;
    char caracter = datos->pila[datos->cima]; //retira o carácter da cima da pila
    datos->pila[datos->cima] = '-'; // marca a posición como baleira

    printf("Consumidor retira '%c' de pila[%d]  cima=%d\n", caracter, pos, datos->cima);
    printf("[Consumidor] Pila: [");
    for (int k = 0; k < TAMANO_BUFFER; k++){
        printf("%c", datos->pila[k]);
    }
    printf("]  cima=%d\n\n", datos->cima);
    //fin da rexión crítica

    sem_post(mutex);  //sae da rexión crítica
    sem_post(vacias); // avisa ao produtor de que hai un oco libre, suma 1 a vacias

    return caracter;
}


// Conta as vogais
void consume_item(char caracter, int *a, int *e, int *i, int *o, int *u) {
    printf("Consumido: '%c'\n", caracter);

    char m = tolower(caracter);
    if (m == 'a') (*a)++;
    if (m == 'e') (*e)++;
    if (m == 'i') (*i)++;
    if (m == 'o') (*o)++;
    if (m == 'u') (*u)++;
}


int main(void) {

    // Agardamos a que o produtor cree o ficheiro de memoria compartida
    printf("[CONS] Agardando ao produtor...\n");
    struct stat info;
    while (stat(FICHEIRO, &info) != 0){
        sleep(1); //stat devolve -1 se o ficheiro non existe, agardamos a que o produtor o cree
    }
    sleep(1); //tempo para que o produtor inicialice a estrutura e os semáforos

    // Abrimos o ficheiro de memoria compartida creado polo produtor
    int fd = open(FICHEIRO, O_RDWR);
    if (fd < 0) { 
        perror("open"); 
        return EXIT_FAILURE; 
    }

    //Mapeamos a memoria compartida
    DatosCompartidos *datos = mmap(NULL, sizeof(DatosCompartidos), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (datos == MAP_FAILED) { 
        perror("mmap"); 
        return EXIT_FAILURE;
    }

    /*
     * Abrimos os semáforos sen inicializar (creados polo produtor) para sincronizar co produtor.
     * Usamos sem_open() sen O_CREAT nin valor inicial.
     */
    sem_t *vacias = sem_open("/VACIAS", 0);
    sem_t *cheas  = sem_open("/CHEAS",  0);
    sem_t *mutex  = sem_open("/MUTEX",  0);

    if (vacias == SEM_FAILED || cheas == SEM_FAILED || mutex == SEM_FAILED) {
        perror("sem_open");
        return EXIT_FAILURE;
    }

    int cant_a = 0, cant_e = 0, cant_i = 0, cant_o = 0, cant_u = 0;
    srand(time(NULL) + 1); //semente distinta á do produtor

    printf("Consumidor iniciado  (buffer N=%d, iteracións=%d)\n\n", TAMANO_BUFFER, ITERACIÓNS);

    for (int iter = 0; iter < ITERACIÓNS; iter++) {

        char caracter = remove_item(datos, vacias, cheas, mutex); //retira un carácter do buffer compartido
        consume_item(caracter, &cant_a, &cant_e, &cant_i, &cant_o, &cant_u); //contamos as vogais

        /*
         * Sleeps fóra da rexión crítica para diferentes velocidades:
         *   - Iteracións  0-29 : consumidor lento (sleep 3s) ---> o buffer énchese
         *   - Iteracións 30-59 : consumidor rápido (sen sleep) ---> o buffer baleirase
         *   - Iteracións 60-79 : velocidade aleatoria (0-3s)
         */
        if (iter < 30) {
            sleep(3); //consumidor vai lento
        } else if (iter < 60) {
            // consumidor non dorme
        } else {
            sleep(rand() % 4); //velocidade aleatoria entre 0 e 3 segundos
        }
    }

    printf("Consumidor, vogais contadas:\n");
    printf("       a=%-3d e=%-3d i=%-3d o=%-3d u=%-3d\n", cant_a, cant_e, cant_i, cant_o, cant_u);

    // Pechamos os semáforos (o produtor encárgase do sem_unlink)
    sem_close(vacias);
    sem_close(cheas);
    sem_close(mutex);

    munmap(datos, sizeof(DatosCompartidos));
    return EXIT_SUCCESS;
}
