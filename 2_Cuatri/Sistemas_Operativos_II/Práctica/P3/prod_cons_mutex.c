/*
 * prod_cons_mutex.c  –  Problema produtor-consumidor con mutex y variables de condición
 *
 * Nuria Guerra Casal y Alejandra Linaje Vallejo
 *
 * Compilar:  gcc -o prod_cons prod_cons.c -pthread
 * Executar:  ./prod_cons numeros.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define TAMANO_BUFFER 10  //número máximo de elementos no buffer FIFO
#define ITERACIONS 80  //iteracións de cada fio (produtor e consumidor)

// Buffer FIFO compartido entre os dous fios
typedef struct {
    int buf[TAMANO_BUFFER]; //array circular de enteiros
    int cabeza; //índice de saída (remove_item lee de aquí)
    int cola_fin; //índice de entrada (insert_item escribe aquí)
    int conta; //número de elementos no buffer agora mesmo
} BufferFIFO;

BufferFIFO buffer; //compartida entre os dous fios

//Mutex e variables de condición para sincronización
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //Protege o acceso ao buffer
pthread_cond_t non_cheo = PTHREAD_COND_INITIALIZER; //Indica que o buffer non está cheo (produtor pode inserir)
pthread_cond_t non_baleiro = PTHREAD_COND_INITIALIZER; //Indica que o buffer non está baleiro (consumidor pode retirar)

//sumas acumuladas para verificar o resultado final
int suma_produtor  = 0;   
int suma_consumidor = 0;

//Procuce_item() le un número do ficheiro e actualiza a suma do produtor
int produce_item(FILE *f) {
    int numero;
    if (fscanf(f, "%d", &numero) != 1) {
        rewind(f); //se chegamos ao final do ficheiro, voltamos ao principio
        fscanf(f, "%d", &numero);
    }
    printf("[Produtor]  Le: %d\n", numero);
    suma_produtor += numero;   
    return numero;
}

//insert_item() insire un número no buffer, con sincronización usando mutex e variables de condición
void insert_item(int numero) {
    pthread_mutex_lock(&mutex); //Protexemos o acceso ao buffer

    //Agarda por espazo dispoñible no buffer
    while (buffer.conta >= TAMANO_BUFFER) {
        printf("[Produtor]   Buffer cheo (conta=%d), agardando...\n", buffer.conta);
        pthread_cond_wait(&non_cheo, &mutex); 
        //Libera o mutex e agarda a que o consumidor sinalice que hai espazo
    }

    //Rexión crítica: inserimos o número no buffer
    int pos = buffer.cola_fin; //Índice onde o produtor vai escribir

    printf("[Produtor]   Intenta inserir %d en buf[%d]  (conta=%d)\n",
           numero, pos, buffer.conta);

    //O outro fio non pode entrar aquí porque temos o mutex bloqueado
    
    buffer.buf[pos] = numero; //Escribe o número no buffer
    buffer.cola_fin = (pos + 1) % TAMANO_BUFFER; //Actualiza o índice de entrada (circular)
    buffer.conta++; //Actualiza a conta de elementos no buffer

    printf("[Produtor]   Inserta %d en buf[%d]  cola_fin=%d  conta=%d\n",
           numero, pos, buffer.cola_fin, buffer.conta);

    printf("[Produtor]   Buffer: [");
    for (int k = 0; k < TAMANO_BUFFER; k++){
        printf("%3d", buffer.buf[k]);
    }
    printf(" ]  cabeza=%d  cola_fin=%d\n\n", buffer.cabeza, buffer.cola_fin);

    pthread_cond_signal(&non_baleiro); //Sinaliza ao consumidor que hai un elemento dispoñible
    pthread_mutex_unlock(&mutex); //Libera o mutex para que o consumidor poida acceder ao buffer
}

//remove_item() retira un número do buffer, con sincronización usando mutex e variables de condición
int remove_item(void) {
    pthread_mutex_lock(&mutex); //Protexemos o acceso ao buffer

    //Agarda por elementos dispoñibles no buffer
    while (buffer.conta <= 0) {
        printf("[Consumidor] Buffer baleiro (conta=%d), agardando...\n", buffer.conta);
        pthread_cond_wait(&non_baleiro, &mutex); //Libera o mutex e agarda a que o produtor sinalice que hai un elemento dispoñible
    }

    //Rexión crítica: retiramos o número do buffer
    int pos = buffer.cabeza;
    printf("[Consumidor] Intenta retirar de buf[%d]  (conta=%d)\n",
           pos, buffer.conta);

    int numero = buffer.buf[pos]; //Lemos o número do buffer na posición actual da cabeza
    buffer.buf[pos] = 0; //Limpa a posición para que se note se alguén le "baleiro"                  
    buffer.cabeza = (pos + 1) % TAMANO_BUFFER; 
    buffer.conta--; //Actualiza a conta de elementos no buffer                          

    printf("[Consumidor] Retira %d de buf[%d]  cabeza=%d  conta=%d\n",
           numero, pos, buffer.cabeza, buffer.conta);

    printf("[Consumidor] Buffer: [");
    for (int k = 0; k < TAMANO_BUFFER; k++){
        printf("%3d", buffer.buf[k]);
    }
    printf(" ]  cabeza=%d  cola_fin=%d\n\n", buffer.cabeza, buffer.cola_fin);

    pthread_cond_signal(&non_cheo); //Sinaliza ao produtor que hai espazo dispoñible no buffer
    pthread_mutex_unlock(&mutex); //Libera o mutex para que o produtor poida acceder ao buffer

    return numero;
}

//consume_item() actualiza a suma do consumidor
void consume_item(int numero) {
    printf("[Consumidor] Consume: %d\n", numero);
    suma_consumidor += numero;
}

//Fios con velocidade variable para forzar diferentes escenarios de acceso ao buffer (cheo, baleiro, etc.)
void *funcion_produtor(void *arg) {

    FILE *f = fopen((char *)arg, "r");
    if (!f) { 
        perror("fopen"); 
        pthread_exit(NULL); 
    }

    //Cada iteración produce un número e insíreo no buffer
    for (int iter = 0; iter < ITERACIONS; iter++) {
        insert_item(produce_item(f));

        //Velocidade fóra da rexión crítica
        if (iter < 30) {
            // Rápido: sen sleep (énchese o buffer)
        } else if (iter < 60) {
            sleep(1); // Lento (baléirase o buffer)
        } else {
            sleep(rand() % 4); // Aleatorio
        }
    }
    fclose(f);
    pthread_exit(NULL);
}

//Fio consumidor con velocidade variable para forzar diferentes escenarios de acceso ao buffer (cheo, baleiro, etc.)
void *funcion_consumidor(void *arg) {

    for (int iter = 0; iter < ITERACIONS; iter++) {
        consume_item(remove_item());

        //Velocidade fóra da rexión crítica
        if (iter < 30) {
            sleep(1); // Lento (énchese o buffer)
        } else if (iter < 60) {
            // Rápido (baléirase o buffer)
        } else {
            sleep(rand() % 4); // Aleatorio
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    if (argc != 2) { //Verificamos que se proporcionou o nome do ficheiro
        fprintf(stderr, "Uso: %s <ficheiro_de_numeros>\n", argv[0]);
        return EXIT_FAILURE;
    }

    srand(time(NULL)); //Inicializamos a semente para o aleatorio

    //Inicializamos o buffer e os índices
    for (int i = 0; i < TAMANO_BUFFER; i++){
        buffer.buf[i] = 0;
    }
    buffer.cabeza = 0; 
    buffer.cola_fin = 0; 
    buffer.conta = 0;

    //Creamos os fios produtor e consumidor
    pthread_t h_prod, h_cons;
    pthread_create(&h_prod, NULL, funcion_produtor, argv[1]);
    pthread_create(&h_cons, NULL, funcion_consumidor, NULL);

    //Agardamos a que ambos fios rematen
    pthread_join(h_prod, NULL);
    pthread_join(h_cons, NULL);

    //Destruímos o mutex e as variables de condición
    pthread_mutex_destroy(&mutex); 
    pthread_cond_destroy(&non_cheo);
    pthread_cond_destroy(&non_baleiro);

    printf("\n\n  Suma do Produtor   : %d\n", suma_produtor);
    printf("  Suma do Consumidor : %d\n", suma_consumidor);
    
    return EXIT_SUCCESS;
}