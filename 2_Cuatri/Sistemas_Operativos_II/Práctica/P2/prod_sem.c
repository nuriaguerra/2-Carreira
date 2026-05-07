/*
 * prod_sem.c - Produtor do problema produtor-consumidor con semáforos
 *
 * Nuria Guerra Casal y Adrián Gijón Yubero
 * 
 * Usa tres semáforos para eliminar as carreiras críticas:
 *   VACIAS : conta os ocos libres no buffer (valor inicial = N)
 *   CHEAS  : conta os elementos listos para consumir (valor inicial = 0)
 *   MUTEX  : garante acceso exclusivo á rexión crítica (valor inicial = 1)
 *
 * Compilar: gcc -o prod_sem prod_sem.c
 * Executar: ./prod_sem ficheiro.txt
 */

#include <stdio.h>
#include <stdlib.h>
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


// Conta as vogais dependendo do carácter lido
void produce_item(char caracter, int *a, int *e, int *i, int *o, int *u) {
    printf("Produtor le: '%c'\n", caracter);

    char m = tolower(caracter); // convertimos a minúscula para contar vogais sen importar maiúsculas/minúsculas
    if (m == 'a') (*a)++;
    if (m == 'e') (*e)++;
    if (m == 'i') (*i)++;
    if (m == 'o') (*o)++;
    if (m == 'u') (*u)++;
}


//insert_item - insire o carácter na cima da pila.

void insert_item(DatosCompartidos *datos, char caracter, sem_t *vacias, sem_t *cheas, sem_t *mutex) {

    sem_wait(vacias); //agarda a que haxa un oco libre (bloquea se o buffer está cheo), resta 1 a vacias
    sem_wait(mutex);  //entra na rexión crítica en exclusión mutua

    //Rexión crítica: acceso exclusivo a datos compartidos (protexida polo mutex)
    datos->pila[datos->cima] = caracter; //insire o carácter na cima da pila
    datos->cima++; //actualiza a cima

    printf("Produtor inserta '%c' en pila[%d]  cima=%d\n", caracter, datos->cima - 1, datos->cima);
    printf("[Produtor] Pila: [");
    for (int k = 0; k < TAMANO_BUFFER; k++){
        printf("%c", datos->pila[k]);
    }
    printf("]  cima=%d\n\n", datos->cima);
    //Fin de rexión crítica

    sem_post(mutex);  //sae da rexión crítica
    sem_post(cheas);  //avisa ao consumidor de que hai un elemento novo, suma 1 a cheas
}


int main(int argc, char *argv[]) {
    //Comprobamos que se pasou o nome do ficheiro a ler
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <ficheiro>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *ficheiro = fopen(argv[1], "r"); //abrimos o ficheiro de texto en modo lectura
    if (!ficheiro) { 
        perror("fopen"); 
        return EXIT_FAILURE; 
    }

    //Creamos o ficheiro de memoria compartida se non existe, e abrímolo para lectura e escritura (0666 son os permisos para todos os usuarios)
    int fd = open(FICHEIRO, O_CREAT | O_RDWR, 0666);
    if (fd < 0) { 
        perror("open"); 
        return EXIT_FAILURE; 
    }
    ftruncate(fd, sizeof(DatosCompartidos)); //establecemos o tamaño do arquivo de memoria compartida

    DatosCompartidos *datos = mmap(NULL, sizeof(DatosCompartidos), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd); //pechamos o descriptor, a memoria xa está mapeada
    if (datos == MAP_FAILED) { 
        perror("mmap"); 
        return EXIT_FAILURE; 
    }

    //Inicializamos a pila con guións e a cima a 0
    for (int j = 0; j < TAMANO_BUFFER; j++){
        datos->pila[j] = '-';
    }
    datos->cima = 0;

    /*
     * Creamos os semáforos e inicializámolos, 
     * antes de crealos eliminamos por se quedaron activos dunha execución anterior sen sem_unlink()
     */
    sem_unlink("/VACIAS");
    sem_unlink("/CHEAS");
    sem_unlink("/MUTEX");

    //vacias=N (todos os ocos libres), cheas=0 (ningún elemento), mutex=1 (libre)
    sem_t *vacias = sem_open("/VACIAS", //nome do semáforo
        O_CREAT, //crea o semáforo se non existe
        0700, //permisos de lectura, escritura e execución para o propietario (o grupo e outros non teñen permisos)
        TAMANO_BUFFER //valor inicial do semáforo (número de ocos libres no buffer)
    );
    sem_t *cheas  = sem_open("/CHEAS",  O_CREAT, 0700, 0);
    sem_t *mutex  = sem_open("/MUTEX",  O_CREAT, 0700, 1);

    if (vacias == SEM_FAILED || cheas == SEM_FAILED || mutex == SEM_FAILED) { //comprobamos que se crearon correctamente
        perror("sem_open");
        return EXIT_FAILURE;
    }

    int cant_a = 0, cant_e = 0, cant_i = 0, cant_o = 0, cant_u = 0;
    srand(time(NULL)); //semente para velocidade aleatoria

    printf("Produtor iniciado  (buffer N=%d, iteracións=%d)\n\n", TAMANO_BUFFER, ITERACIÓNS);

    for (int iter = 0; iter < ITERACIÓNS; iter++) {

        int c = fgetc(ficheiro); //le un carácter do ficheiro
        if (c == EOF) {
            // Se o ficheiro remata, volvemos ao inicio para completar as iteracións
            rewind(ficheiro);
            c = fgetc(ficheiro);
        }

        produce_item((char)c, &cant_a, &cant_e, &cant_i, &cant_o, &cant_u); //contamos as vogais do carácter lido
        insert_item(datos, (char)c, vacias, cheas, mutex); //insertamos o carácter no buffer compartido

        /*
         * Sleeps fóra da rexión crítica para diferentes velocidades:
         *   - Iteracións  0-29 : produtor rápido (sen sleep) ---> o buffer énchese
         *   - Iteracións 30-59 : produtor lento (sleep 3s) ---> o consumidor baleira o buffer
         *   - Iteracións 60-79 : velocidade aleatoria (0-3s)
         */
        if (iter < 30) {
            // produtor non dorme
        } else if (iter < 60) {
            sleep(3); //produtor vai lento
        } else {
            sleep(rand() % 4); //velocidade aleatoria entre 0 e 3 segundos
        }
    }

    fclose(ficheiro);

    printf("Productor, vogais contadas:\n");
    printf("       a=%-3d e=%-3d i=%-3d o=%-3d u=%-3d\n", cant_a, cant_e, cant_i, cant_o, cant_u);

    // Pechamos e eliminamos os semáforos
    sem_close(vacias);
    sem_close(cheas);
    sem_close(mutex);
    sem_unlink("/VACIAS");
    sem_unlink("/CHEAS");
    sem_unlink("/MUTEX");

    munmap(datos, sizeof(DatosCompartidos)); //liberamos a memoria compartida
    return EXIT_SUCCESS;
}
