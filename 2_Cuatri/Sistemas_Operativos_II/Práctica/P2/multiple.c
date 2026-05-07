/*
 * multiple.c - Productor-consumidor con N productores y M consumidores
 *
 * Nuria Guerra Casal y Adrián Gijón Yubero
 *
 * Usa tres semáforos para eliminar as carreiras críticas:
 *   VACIAS: conta os ocos libres no buffer (valor inicial = N)
 *   CHEAS: conta os elementos listos para consumir (valor inicial = 0)
 *   MUTEX: garante acceso exclusivo á rexión crítica (valor inicial = 1)
 *
 * Cada consumidor consume (num_prod * TOTAL_ITERS) / num_cons caracteres.
 * 
 * Compilar: gcc -o multiple multiple.c
 * Executar: ./multiple <N> <M> <fichero>
 *   N : número de productores (>= 1)
 *   M : número de consumidores (>= 1)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <time.h>

#define TAMANO_BUFFER 10        // tamaño de la pila LIFO
#define TOTAL_ITERS   80        // iteraciones por productor
#define FICHERO_SHM   "multiple.dat"

typedef struct {
    char pila[TAMANO_BUFFER];
    int  cima;
} DatosCompartidos;

void insert_item(DatosCompartidos *datos, char c, sem_t *vacias, sem_t *cheas, sem_t *mutex) {

    sem_wait(vacias); //espera oco libre, bloquea si o buffer está cheo
    sem_wait(mutex);  //entra en exclusión mutua

    //rexión crítica: só un proceso á vez pode modificar o buffer
    datos->pila[datos->cima] = c;
    datos->cima++;

    printf("Insertado '%c' en pila[%d]  cima=%d | Pila=[", c, datos->cima - 1, datos->cima);
    for (int i = 0; i < TAMANO_BUFFER; i++){
        printf("%c", datos->pila[i]);
    }
    printf("]\n");
    //fin rexión crítica

    sem_post(mutex);  //sae da exclusión mutua
    sem_post(cheas);  //avisa aos consumidores de que hai un elemento
}

char remove_item(DatosCompartidos *datos, sem_t *vacias, sem_t *cheas, sem_t *mutex) {

    sem_wait(cheas);  //espera elemento disponible, bloquea si o buffer está vacío
    sem_wait(mutex);  //entra en exclusión mutua

    //rexión crítica: só un proceso á vez pode modificar o buffer
    datos->cima--;
    int  pos = datos->cima;
    char c   = datos->pila[pos];
    datos->pila[pos] = '-'; // sustitue por '-'

    printf("\nRetirado '%c' de pila[%d]  cima=%d | Pila=[", c, pos, datos->cima);
    for (int i = 0; i < TAMANO_BUFFER; i++){
        printf("%c", datos->pila[i]);
    }
    printf("]\n");
    //fin rexión crítica

    sem_post(mutex);  //sae da exclusión mutua
    sem_post(vacias); //avisa aos productores de que hai un oco libre

    return c;
}

void productor(int id, char *fichero, DatosCompartidos *datos, sem_t *vacias, sem_t *cheas, sem_t *mutex) {

    FILE *f = fopen(fichero, "r");
    if (!f) { 
        perror("fopen"); 
        exit(EXIT_FAILURE); 
    }

    // Cada productor avanza 'id' posicions para ler partes distintas do ficheiro
    for (int i = 0; i < id; i++) {
        if (fgetc(f) == EOF){
            rewind(f);
        }
    }

    //Contadores de vogais lidas por este productor
    int a=0, e=0, i=0, o=0, u=0;

    srand(time(NULL) + id); //semente distinta para cada produtor (baseada no id)

    printf("[Productor %d] Iniciado (%d iteraciones)\n", id, TOTAL_ITERS);

    for (int iter = 0; iter < TOTAL_ITERS; iter++) {

        int c = fgetc(f);
        if (c == EOF) { 
            rewind(f); 
            c = fgetc(f); 
        }

        char ch = (char)c; //converte int a char para imprimir e contar vogais
        printf("[Productor %d] Le '%c'\n", id, ch);

        char m = tolower(ch);
        if (m == 'a') a++;
        if (m == 'e') e++;
        if (m == 'i') i++;
        if (m == 'o') o++;
        if (m == 'u') u++;

        insert_item(datos, ch, vacias, cheas, mutex);

        //Fases de velocidades
        if (iter < 30) { 
            //sin sleep
        } else if (iter < 60) { 
            sleep(3); 
        } else { 
            sleep(rand() % 4); 
        }
    }

    fclose(f);
    printf("\n\t\t[Productor %d] Rematou. Vogais: a=%d e=%d i=%d o=%d u=%d\n\n", id, a, e, i, o, u);

    //Pecha os semáforos antes de sair para liberar recursos
    sem_close(vacias); 
    sem_close(cheas); 
    sem_close(mutex);
    exit(EXIT_SUCCESS);
}


void consumidor(int id, DatosCompartidos *datos, int iter_cons, sem_t *vacias, sem_t *cheas, sem_t *mutex) {

    // Contadores de vogais consumidas por este consumidor
    int a=0, e=0, i=0, o=0, u=0;

    srand(time(NULL) + id + 100); //semente distinta para cada consumidor (baseada no id)
    printf("[Consumidor %d] Iniciado (%d iteraciones)\n", id, iter_cons);

    for (int iter = 0; iter < iter_cons; iter++) {

        char c = remove_item(datos, vacias, cheas, mutex);
        printf("[Consumidor %d] Consume '%c'\n", id, c);

        char m = tolower(c);
        if (m == 'a') a++;
        if (m == 'e') e++;
        if (m == 'i') i++;
        if (m == 'o') o++;
        if (m == 'u') u++;

        if (iter < 30) { 
            sleep(3); 
        } else if (iter < 60) { 
            //sin sleep
        } else { 
            sleep(rand() % 4); 
        }
    }

    printf("\n\t\t[Consumidor %d] Rematou. Vogais: a=%d e=%d i=%d o=%d u=%d\n\n", id, a, e, i, o, u);

    sem_close(vacias); 
    sem_close(cheas); 
    sem_close(mutex);
    exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[]) {

    if (argc != 4) {
        fprintf(stderr, "Uso: %s <N> <M> <fichero>\n", argv[0]);
        fprintf(stderr, "  N >= 1 : numero de productores\n");
        fprintf(stderr, "  M >= 1 : numero de consumidores\n");
        return EXIT_FAILURE;
    }

    int num_prod = atoi(argv[1]);
    int num_cons = atoi(argv[2]);
    char *fichero = argv[3];

    if (num_prod < 1 || num_cons < 1) { 
        //Tanto o número de productores como de consumidores debe ser polo menos 1
        fprintf(stderr, "Error: N y M deben ser >= 1\n");
        return EXIT_FAILURE;
    }

    //Calculamos as iteracións por consumidor para garantizar que total producido == total consumido
    int total = num_prod * TOTAL_ITERS;

    if (total % num_cons != 0) {
        fprintf(stderr, "Erro, N*TOTAL_ITERS (%d) non é divisible entre M (%d)\n", total, num_cons);
        return EXIT_FAILURE;
    }
    int iter_cons = total / num_cons;

    printf("[Main] %d productores, %d consumidores\n", num_prod, num_cons);
    printf("[Main] Cada productor: %d chars | Cada consumidor: %d chars | Total: %d\n\n", TOTAL_ITERS, iter_cons, total);

    //Memoria compartida (un único buffer para todos os procesos)
    int fd = open(FICHERO_SHM, O_CREAT | O_RDWR, 0666);
    if (fd < 0) { 
        perror("open"); 
        return EXIT_FAILURE; 
    }
    ftruncate(fd, sizeof(DatosCompartidos)); // redimensiona o ficheiro para que poida conter un buffer

    //Mapeamos a memoria compartida
    DatosCompartidos *datos = mmap(NULL, sizeof(DatosCompartidos), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd); //pechamos o descriptor, memoria xa mapeada

    if (datos == MAP_FAILED) { 
        perror("mmap"); 
        return EXIT_FAILURE; 
    }

    //Inicializamos o buffer con guións e a cima a 0
    datos->cima = 0;
    for (int i = 0; i < TAMANO_BUFFER; i++){
        datos->pila[i] = '-';
    }

    //Creamos os semáforos (antes do fork() para que os fillos atópenos disponibles
    sem_unlink("/VACIAS"); 
    sem_unlink("/CHEAS"); 
    sem_unlink("/MUTEX");

    sem_t *vacias = sem_open("/VACIAS", O_CREAT, 0700, TAMANO_BUFFER);
    sem_t *cheas  = sem_open("/CHEAS",  O_CREAT, 0700, 0);
    sem_t *mutex  = sem_open("/MUTEX",  O_CREAT, 0700, 1);

    if (vacias == SEM_FAILED || cheas == SEM_FAILED || mutex == SEM_FAILED) {
        fprintf(stderr, "Error ao crear semáforos\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL)); //semente para velocidades aleatorias

    //Lanzamos os N productores con fork()
    int total_procs = num_prod + num_cons;
    pid_t pids[total_procs]; // array para gardar os PIDs dos procesos creados

    for (int k = 0; k < num_prod; k++) {
        pids[k] = fork(); //crea un proceso fillo para cada productor
        if (pids[k] < 0) { 
            printf("Error no uso de fork()\n"); 
            return EXIT_FAILURE; 
        }
        if (pids[k] == 0){ // proceso fillo productor
            productor(k, fichero, datos, vacias, cheas, mutex);
        }
    }

    // Lanzamos os M consumidores con fork()
    for (int k = 0; k < num_cons; k++) {
        pids[num_prod + k] = fork(); //crea un proceso fillo para cada consumidor
        if (pids[num_prod + k] < 0) { 
            printf("Error no uso de fork()\n"); 
            return EXIT_FAILURE; 
        }
        if (pids[num_prod + k] == 0){
            consumidor(k, datos, iter_cons, vacias, cheas, mutex);
        }
    }

    //Espera a que rematen todos os procesos fillos (produtores e consumidores)
    for (int k = 0; k < total_procs; k++){
        waitpid(pids[k], NULL, 0);
    }


    //Pecha e elimina os semáforos para liberar recursos
    sem_close(vacias); 
    sem_unlink("/VACIAS");
    sem_close(cheas);  
    sem_unlink("/CHEAS");
    sem_close(mutex);  
    sem_unlink("/MUTEX");

    munmap(datos, sizeof(DatosCompartidos));
    unlink(FICHERO_SHM);

    return EXIT_SUCCESS;
}