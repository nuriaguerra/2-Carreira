/*
 * prod.c - Produtor do problema produtor-consumidor
 *
 * Nuria Guerra Casal y Adrián Gijón Yubero
 * 
 * Le caracteres dun ficheiro de texto e colócaos nun buffer
 * compartido co consumidor (pila LIFO) sen ningún semáforo,
 * para demostrar a aparición de carreiras críticas.
 *
 * Compilar: gcc -o prod prod.c
 * Executar: ./prod ficheiro.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TAMANO_BUFFER 10        // número máximo de elementos na pila
#define FICHEIRO "buffer.dat"   // ficheiro usado como memoria compartida

/*
 * Estrutura que se comparte entre o produtor e o consumidor.
 * Está mapeada en memoria compartida con mmap().
 */
typedef struct {
    char pila[TAMANO_BUFFER]; // buffer tipo pila LIFO
    int  cima;                // índice do seguinte oco libre (0=baleiro, N=cheo)
    int  rematou;             // o produtor acabou de ler o ficheiro
} DatosCompartidos;


// Conta as vogais do carácter lido e imprime o que se está a producir
void produce_item(char caracter, int *a, int *e, int *i, int *o, int *u) {
    printf("Produtor le: '%c'\n", caracter);

    // Sumamos a vogal se é o caso do carácter lido
    char minuscula = tolower(caracter);
    if (minuscula == 'a') (*a)++;
    if (minuscula == 'e') (*e)++;
    if (minuscula == 'i') (*i)++;
    if (minuscula == 'o') (*o)++;
    if (minuscula == 'u') (*u)++;
}


/*
 * Engade o carácter na cima da pila compartida.
 *
 * Primeiro gárdase a posición actual en 'posicion' ANTES do sleep
 * faise un sleep() para que o consumidor pode cambiar cima neste intre
 * Posteriormente, escríbese en pila[posicion] e actualízase cima
 *
 * Se o consumidor modifica 'cima' o produtor usará un índice vello --> carreira crítica.
 */
void insert_item(DatosCompartidos *datos, char caracter) {

    // Espera activa mentres o buffer está cheo
    while (datos->cima >= TAMANO_BUFFER) {
        printf("Buffer cheo (cima=%d), agardando...\n", datos->cima);
        sleep(1);
    }

    // gardamos a posición antes do sleep, así o índice queda fixo
    int posicion = datos->cima;
    printf("cima=%d  produtor intenta inserir '%c' en pila[%d]\n", posicion, caracter, posicion);

    sleep(2); // <-- sleep() carreira crítica (aumentar sobe a probabilidade)

    // usamos 'posicion', ignorando calquera cambio de cima
    // Se o consumidor cambiou cima durante o sleep --> carreira crítica garantida
    datos->pila[posicion] = caracter; // inserta na posición gardada (pode ser obsoleta)
    datos->cima = posicion + 1;       // actualiza sen ter en conta o que fixo o consumidor

    printf("Produtor inserta '%c' en pila[%d], agora cima=%d\n", caracter, posicion, datos->cima);

    // Mostramos o estado completo da pila para ver a carreira visualmente
    printf("[Produtor]    Pila: [");
    for (int k = 0; k < TAMANO_BUFFER; k++)
        printf("%c", datos->pila[k]);
    printf("]  cima=%d\n\n", datos->cima);
}


int main(int argc, char *argv[]) {
    // Comprobamos que se pasou o nome do ficheiro a ler
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <ficheiro>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Abrimos o ficheiro de texto en modo lectura
    FILE *ficheiro = fopen(argv[1], "r");
    if (!ficheiro) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    /*
     * Creamos o ficheiro de memoria compartida que o consumidor abrirá despois.
     * O_CREAT crea o arquivo se non existe.
     * O_RDWR abre para lectura e escritura.
     * 0666 son os permisos para todos os usuarios.
     */
    int fd = open(FICHEIRO, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    // ftruncate() establece o tamaño do arquivo de memoria compartida (por defecto é 0)
    ftruncate(fd, sizeof(DatosCompartidos));

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

    // Inicializamos a pila con guións, cima=0 e rematou=0
    for (int j = 0; j < TAMANO_BUFFER; j++)
        datos->pila[j] = '-';
    datos->cima    = 0;
    datos->rematou = 0;

    // Contadores de vogais lidas polo produtor
    int cant_a = 0, cant_e = 0, cant_i = 0, cant_o = 0, cant_u = 0;

    printf("Produtor iniciado  (buffer N=%d)\n\n", TAMANO_BUFFER);

    //le un carácter do ficheiro e méteo no buffer compartido
    int c;
    while ((c = fgetc(ficheiro)) != EOF) {
        produce_item((char)c, &cant_a, &cant_e, &cant_i, &cant_o, &cant_u);
        insert_item(datos, (char)c);
    }

    fclose(ficheiro);

    // Avisamos ao consumidor antes de agardar, para que empece a baleirar o buffer
    datos->rematou = 1;
    printf("Produtor chegou ao final, agarda a que o consumidor baleire o buffer...\n");

    // Agardamos a que o consumidor baleiro o buffer.
    // Sen esta espera, munmap liberaría a memoria antes de que o consumidor remate.
    while (datos->cima > 0)
        sleep(1);
    sleep(1); // marxe extra para que o consumidor remate de imprimir

    printf("\nProdutor, vogais contadas:\n");
    printf("       a=%-3d e=%-3d i=%-3d o=%-3d u=%-3d\n", cant_a, cant_e, cant_i, cant_o, cant_u);

    munmap(datos, sizeof(DatosCompartidos)); // liberamos a memoria compartida
    return EXIT_SUCCESS;
}