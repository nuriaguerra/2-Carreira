/*
 * prod_cons.c  –  Problema produtor-consumidor con carreiras críticas
 *
 * Nuria Guerra Casal y Alejandra Linaje Vallejo
 *
 *   - Uso de fios (pthread) no canto de procesos separados.
 *     Os fios comparten directamente memoria, polo que non se usa mmap().
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

// Buffer FIFO compartido entre os dous fios
typedef struct {
    int buf[TAMANO_BUFFER]; //array circular de enteiros
    int cabeza; //índice de saída (remove_item lee de aquí)
    int cola_fin; //índice de entrada (insert_item escribe aquí)
    int conta; //número de elementos no buffer agora mesmo
    int  rematou; // o produtor acabou de ler o ficheiro
} BufferFIFO;

BufferFIFO buffer; //compartida entre os dous fios

//sumas acumuladas para verificar o resultado final
int suma_produtor  = 0;   
int suma_consumidor = 0;  

//produce_item() le un número do ficheiro e actualiza a suma do produtor
int produce_item(FILE *f) {
    int numero;
    if (fscanf(f, "%d", &numero) != 1) {
        return -1; //indicamos que non hai máis números
    }
    printf("[Produtor]   Le: %d\n", numero);
    suma_produtor += numero;   
    return numero;
}

//insert_item() insire un número no buffer, sen ningunha sincronización
void insert_item(int numero) {

    //Espera activa mentres o buffer está cheo
    while (buffer.conta >= TAMANO_BUFFER || buffer.conta < 0) {
        usleep(100); 
    }

    //Gárdase o estado actual do buffer localmente (Capturamos o erro)
    int pos_vella   = buffer.cola_fin;
    int conta_vella = buffer.conta;

    printf("[Produtor]: ve conta=%d, garda local=%d. Destino buf[%d]\n",
           buffer.conta, conta_vella, pos_vella);

    sleep(2); //O consumidor pode alterar 'cabeza' e 'conta' aquí, provocando unha carreira crítica

    //Escribe o número no buffer cos índices que gardamos, ignorando os cambios do consumidor
    buffer.buf[pos_vella] = numero;
    
    //Machacamos a conta global coa nosa copia local + 1, ignorando ao consumidor
    buffer.cola_fin = (pos_vella + 1) % TAMANO_BUFFER;
    buffer.conta = conta_vella + 1; 

    printf("[Produtor]: Inserta %d en buf[%d]. Nova conta=%d (ignorando cambios)\n",
        numero, pos_vella, buffer.conta);

    //Amosamos o estado do buffer
    printf("[Produtor]   Buffer: [");
    for (int k = 0; k < TAMANO_BUFFER; k++){
        printf("%3d", buffer.buf[k]);
    }
    printf(" ]  cabeza=%d  cola_fin=%d\n\n", buffer.cabeza, buffer.cola_fin);
}


//remove_item() retira un número do buffer, sen ningunha sincronización
int remove_item(void) {

    //Espera activa mentres o buffer está baleiro
    while (buffer.conta <= 0 || buffer.conta > TAMANO_BUFFER) {
        usleep(100);
    }

    //Gárdase o estado actual do buffer localmente (Capturamos o erro)
    int pos_vella   = buffer.cabeza;
    int conta_vella = buffer.conta;

    printf("[Consumidor]: ve conta=%d, garda local=%d. Orixe buf[%d]\n",
           buffer.conta, conta_vella, pos_vella);

    sleep(2); //O produtor pode alterar 'cola_fin' e 'conta' aquí, provocando unha carreira crítica

    //Lemos o número do buffer cos índices que gardamos, ignorando os cambios do produtor
    int numero = buffer.buf[pos_vella];
    buffer.buf[pos_vella] = 0; //Limpamos con 0 para que se note se alguén le "baleiro"
    
    //Machacamos a conta global coa nosa copia local - 1
    buffer.cabeza = (pos_vella + 1) % TAMANO_BUFFER;
    buffer.conta = conta_vella - 1;

    printf("[Consumidor]: Retira %d de buf[%d]. Nova conta=%d (ignorando cambios)\n",
           numero, pos_vella, buffer.conta);

    //Amosamos o estado do buffer
    printf("[Consumidor] Buffer: [");
    for (int k = 0; k < TAMANO_BUFFER; k++){
        printf("%3d", buffer.buf[k]);
    }
    printf(" ]  cabeza=%d  cola_fin=%d\n\n", buffer.cabeza, buffer.cola_fin);

    return numero;
}


//consume_item() actualiza a suma do consumidor
void consume_item(int numero) {
    printf("[Consumidor] Consume: %d\n", numero);
    suma_consumidor += numero;
}


//funcion_produtor() le números do ficheiro e insíreos no buffer
void *funcion_produtor(void *arg) {

    char *nome_ficheiro = (char *)arg;

    FILE *f = fopen(nome_ficheiro, "r");
    if (!f) { 
        perror("fopen produtor"); 
        pthread_exit(NULL); 
    }

    int numero;
    while ((numero = produce_item(f)) != -1) { //Mentres haxa números no ficheiro
        insert_item(numero);
    }

    // marcar que rematou
    buffer.rematou = 1;
    printf("[Produtor] Rematou de producir.\n");

    fclose(f);
    pthread_exit(NULL);
}


//funcion_consumidor() retira números do buffer e actualiza a suma do consumidor
void *funcion_consumidor(void *arg) {
    while (1) {

        if (buffer.conta <= 0) {

            if (buffer.rematou)
                break;

            usleep(100);
            continue;
        }

        int numero = remove_item();
        consume_item(numero);
    }
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {

    if (argc != 2) { //Verificamos que se proporcionou o nome do ficheiro
        fprintf(stderr, "Uso: %s <ficheiro_de_numeros>\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < TAMANO_BUFFER; i++){
        buffer.buf[i] = 0;
    }

    //Inicializamos os índices e a conta do buffer
    buffer.cabeza = 0; 
    buffer.cola_fin = 0; 
    buffer.conta = 0;
    buffer.rematou = 0;

    //Creamos os dous fios: produtor e consumidor
    pthread_t hilo_prod, hilo_cons; //identificadores dos fios
    pthread_create(&hilo_prod, NULL, funcion_produtor,  argv[1]);
    pthread_create(&hilo_cons, NULL, funcion_consumidor, NULL);

    //Agardamos a que ambos fios rematen
    pthread_join(hilo_prod, NULL);
    pthread_join(hilo_cons, NULL);


    printf("\n\n  Suma do Produtor   : %d\n", suma_produtor);
    printf("  Suma do Consumidor : %d\n", suma_consumidor);

    return EXIT_SUCCESS;
}