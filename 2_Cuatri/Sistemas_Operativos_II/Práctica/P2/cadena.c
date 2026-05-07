/*
 * cadena.c - Exercicio opcional 3: cadea de P procesos con semáforos
 *
 * Nuria Guerra Casal y Adrián Gijón Yubero
 *
 * Xeneralización dos códigos prod_sem.c e cons_sem.c para P procesos
 * encadeados, onde cada proceso intermedio é á vez consumidor do anterior
 * e produtor do seguinte, transformando cada carácter no seguinte alfabético.
 *
 * Para P procesos créanse:
 *   - P-1 buffers compartidos
 *   - 3*(P-1) semáforos POSIX con nome
 *
 * Todos os procesos lánzanse con fork() para garantir a máxima concorrencia.
 *
 * Compilar: gcc -o cadena cadena.c -pthread
 * Executar: ./cadena <numProcesos> <ficheiro>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>

#define TAMANO_BUFFER 10 // tamaño de cada buffer da cadea
#define ITERACIÓNS 80 // caracteres que pasa cada proceso
#define FICHEIRO_SHM  "cadena.dat" // ficheiro de memoria compartida

// Un buffer compartido entre dous procesos.
typedef struct {
    char pila[TAMANO_BUFFER];
    int  cima;
} DatosCompartidos;

// Constrúe o nome do semáforo para o buffer 'idx' co prefixo dado
void nome_sem(char *dest, const char *prefixo, int idx) {
    sprintf(dest, "/%s_%d", prefixo, idx); //sprintf para formatar o nome do semáforo como "/PREFIXO_IDX"
}

// Crea e inicializa un semáforo
sem_t *crear_sem(const char *nome, unsigned int valor) {

    sem_unlink(nome); // eliminamos por se quedou activo dunha execución anterior
    sem_t *s = sem_open(nome, O_CREAT, 0700, valor); //crea un semáforo con nome, permisos 0700 e valor inicial

    if (s == SEM_FAILED) { 
        perror("sem_open (crear)"); 
        exit(EXIT_FAILURE); 
    }
    return s;
}

// Abre un semáforo xa existente
sem_t *abrir_sem(const char *nome) {
    sem_t *s = sem_open(nome, 0);

    if (s == SEM_FAILED) { 
        perror("sem_open (abrir)"); 
        exit(EXIT_FAILURE); 
    }
    return s;
}


// Seguinte carácter alfabético
char seguinte (char c) {
    if (c == 'z'){
        return 'a';
    }
    if (c == 'Z'){
        return 'A';
    }
    if (isalpha(c)){
        return c + 1;
    }
    return c;
}


void insert_item(DatosCompartidos *buf, char caracter, sem_t *vacias, sem_t *cheas, sem_t *mutex) {

    sem_wait(vacias); // agarda oco libre; bloquea se o buffer está cheo
    sem_wait(mutex);  // entra na rexión crítica en exclusión mutua

    // rexión crítica: acceso exclusivo ao buffer compartido
    buf->pila[buf->cima] = caracter;
    buf->cima++;

    sem_post(mutex);  // sae da rexión crítica
    sem_post(cheas);  // avisa ao seguinte proceso de que hai un elemento
}


char remove_item(DatosCompartidos *buf, sem_t *vacias, sem_t *cheas, sem_t *mutex) {

    sem_wait(cheas);  //agarda elemento dispoñible; bloquea se o buffer está baleiro
    sem_wait(mutex);  //entra na rexión crítica en exclusión mutua

    // rexión crítica: acceso exclusivo ao buffer compartido
    buf->cima--;
    int pos = buf->cima;
    char caracter = buf->pila[pos];
    buf->pila[pos] = '-'; // marca a posición como baleira

    sem_post(mutex);  //sae da rexión crítica
    sem_post(vacias); //avisa ao proceso anterior de que hai un oco libre

    return caracter;
}

/*
 * Proceso 0: só produce.
 * Le caracteres do ficheiro e méteos no buffer 0.
 */
void rol_produtor(int id, DatosCompartidos *bufs, const char *fich) {

    //Abre os semáforos do buffer 0 (creados polo proceso pai)
    char vacio[32], cheo[32], muteado[32];
    nome_sem(vacio, "VAC", 0);
    nome_sem(cheo, "CHE", 0);
    nome_sem(muteado, "MUT", 0);
    sem_t *vacias = abrir_sem(vacio);
    sem_t *cheas  = abrir_sem(cheo);
    sem_t *mutex  = abrir_sem(muteado);

    FILE *f = fopen(fich, "r");
    if (!f) { 
        perror("fopen"); 
        exit(EXIT_FAILURE); 
    }

    int cant_a=0, cant_e=0, cant_i=0, cant_o=0, cant_u=0;
    printf("[P%d-Productor] Iniciado\n", id);

    for (int iter = 0; iter < ITERACIÓNS; iter++) {

        int c = fgetc(f);
        if (c == EOF) { 
            rewind(f); 
            c = fgetc(f); 
        } // volta ao inicio ao chegar ao EOF

        char ch = (char)c; //converte int a char

        // conta vogais lidas
        char m = tolower(ch);
        if (m == 'a') cant_a++;
        if (m == 'e') cant_e++;
        if (m == 'i') cant_i++;
        if (m == 'o') cant_o++;
        if (m == 'u') cant_u++;

        printf("[P%d-Productor] Le '%c' → buffer[0]\n", id, ch);
        insert_item(&bufs[0], ch, vacias, cheas, mutex);

        // sleeps fóra da rexión crítica (igual que en prod_sem.c)
        if (iter < 30) { 
            //produtor rápido: buffer énchese
        } else if (iter < 60) { 
            sleep(3); //produtor lento: buffer baleirase
        } else                { 
            sleep(rand() % 4); //aleatorio 0-3s
        }
    }

    fclose(f);
    printf("\n[P%d-Productor] Rematou. Vogais lidas: a=%d e=%d i=%d o=%d u=%d\n",
           id, cant_a, cant_e, cant_i, cant_o, cant_u);

    sem_close(vacias); 
    sem_close(cheas); 
    sem_close(mutex);
}

/*
 * proceso intermedio
 * Consume do buffer k-1, calcula o seguinte carácter alfabético e produce no buffer k.
 * É á vez cons_sem.c (para o buffer anterior) e prod_sem.c (para o seguinte).
 */
void rol_intermedio(int id, DatosCompartidos *bufs) {

    int ent = id - 1; // índice do buffer do que le
    int sal = id; // índice do buffer ao que escribe

    //Abre semáforos do buffer de entrada
    char vacio0[32], cheo0[32], muteado0[32];
    nome_sem(vacio0, "VAC", ent);
    nome_sem(cheo0, "CHE", ent);
    nome_sem(muteado0, "MUT", ent);
    sem_t *v_ent = abrir_sem(vacio0);
    sem_t *c_ent = abrir_sem(cheo0);
    sem_t *m_ent = abrir_sem(muteado0);

    // Abre semáforos do buffer de saída
    char vacio1[32], cheo1[32], muteado1[32];
    nome_sem(vacio1, "VAC", sal);
    nome_sem(cheo1, "CHE", sal);
    nome_sem(muteado1, "MUT", sal);
    sem_t *v_sal = abrir_sem(vacio1);
    sem_t *c_sal = abrir_sem(cheo1);
    sem_t *m_sal = abrir_sem(muteado1);

    printf("[P%d-Intermedio] Iniciado (buffer %d → buffer %d)\n", id, ent, sal);

    for (int iter = 0; iter < ITERACIÓNS; iter++) {

        //retira do buffer de entrada
        char c = remove_item(&bufs[ent], v_ent, c_ent, m_ent);

        //calcula o seguinte carácter alfabético
        char c_novo = seguinte (c);

        printf("[P%d-Intermedio] '%c' → '%c'  (buffer[%d] → buffer[%d])\n", id, c, c_novo, ent, sal);

        //mete o carácter transformado no buffer de saída
        insert_item(&bufs[sal], c_novo, v_sal, c_sal, m_sal);

        if (iter >= 60){
            sleep(rand() % 3); // algo de aleatoriedade na fase final
        }
    }

    printf("[P%d-Intermedio] Rematou.\n", id);

    sem_close(v_ent); 
    sem_close(c_ent); 
    sem_close(m_ent);
    sem_close(v_sal); 
    sem_close(c_sal); 
    sem_close(m_sal);
}

/*
 * Proceso P-1: só consume.
 * Retira do último buffer e conta vogais.
 */
void rol_consumidor(int id, DatosCompartidos *bufs) {

    int ent = id - 1; // índice do último buffer

    //Abre os semáforos do último buffer (creados polo proceso pai)
    char vacio[32], cheo[32], muteado[32];
    nome_sem(vacio, "VAC", ent);
    nome_sem(cheo, "CHE", ent);
    nome_sem(muteado, "MUT", ent);
    sem_t *vacias = abrir_sem(vacio);
    sem_t *cheas  = abrir_sem(cheo);
    sem_t *mutex  = abrir_sem(muteado);

    int cant_a=0, cant_e=0, cant_i=0, cant_o=0, cant_u=0;
    printf("[P%d-Consumidor] Iniciado\n", id);

    for (int iter = 0; iter < ITERACIÓNS; iter++) {

        char c = remove_item(&bufs[ent], vacias, cheas, mutex);

        //conta vogais consumidas
        char m = tolower(c);

        if (m == 'a') cant_a++;
        if (m == 'e') cant_e++;
        if (m == 'i') cant_i++;
        if (m == 'o') cant_o++;
        if (m == 'u') cant_u++;

        printf("[P%d-Consumidor] Consume '%c'\n", id, c);

        if (iter < 30) { 
            sleep(3); //consumidor lento: buffer énchese
        } else if (iter < 60) { 
            //consumidor rápido: buffer baleirase
        } else { 
            sleep(rand() % 4); // aleatorio 0-3s */ 
        }
    }

    printf("\n[P%d-Consumidor] Rematou. Vogais consumidas: a=%d e=%d i=%d o=%d u=%d\n",
           id, cant_a, cant_e, cant_i, cant_o, cant_u);

    sem_close(vacias); 
    sem_close(cheas); 
    sem_close(mutex);
}


int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Uso: %s <numProcesos> <ficheiro>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int P = atoi(argv[1]);
    if (P < 2) {
        fprintf(stderr, "Erro: P debe ser maior ou igual a 2\n");
        return EXIT_FAILURE;
    }

    const char *fich = argv[2];
    int n_bufs = P - 1; // un buffer entre cada par de procesos consecutivos

    //Memoria compartida
    int fd = open(FICHEIRO_SHM, O_CREAT | O_RDWR, 0666);
    if (fd < 0) { 
        perror("open"); 
        return EXIT_FAILURE; 
    }
    ftruncate(fd, sizeof(DatosCompartidos) * n_bufs); // redimensiona o ficheiro para que poida conter n_bufs buffers

    DatosCompartidos *bufs = mmap(NULL, sizeof(DatosCompartidos) * n_bufs, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (bufs == MAP_FAILED) { 
        perror("mmap"); 
        return EXIT_FAILURE; 
    }

    // Inicializamos todos os buffers con guións e cima=0
    for (int i = 0; i < n_bufs; i++) {
        for (int j = 0; j < TAMANO_BUFFER; j++) {
            bufs[i].pila[j] = '-';
        }
        bufs[i].cima = 0;
    }

    // O proceso pai crea todos os semáforos antes do fork() para que os fillos os atopen dispoñibles
    sem_t *sems_v[n_bufs], *sems_c[n_bufs], *sems_m[n_bufs];
    for (int i = 0; i < n_bufs; i++) {
        char vacio[32], cheo[32], muteado[32];
        nome_sem(vacio, "VAC", i);
        nome_sem(cheo, "CHE", i);
        nome_sem(muteado, "MUT", i);
        sems_v[i] = crear_sem(vacio, TAMANO_BUFFER); // vacias = N (todos os ocos libres)
        sems_c[i] = crear_sem(cheo, 0);  // cheas  = 0 (ningún elemento)
        sems_m[i] = crear_sem(muteado, 1);  // mutex  = 1 (libre)
    }

    srand(time(NULL));
    printf("[PAI] Cadea de %d procesos, %d buffers, %d iteracións\n\n", P, n_bufs, ITERACIÓNS);

    //Lanzamos os P procesos con fork() ç
    pid_t pids[P];
    for (int k = 0; k < P; k++) {
        pids[k] = fork();

        if (pids[k] < 0) { 
            perror("fork"); 
            return EXIT_FAILURE; 
        }

        if (pids[k] == 0) {
            // Proceso fillo: cada un coa súa semente aleatoria propia
            srand(time(NULL) + k);

            if  (k == 0) {
                rol_produtor(k, bufs, fich);
            }
            else if (k == P - 1) {
                rol_consumidor(k, bufs);
            }
            else {
                rol_intermedio(k, bufs);
            }

            exit(EXIT_SUCCESS);
        }
        // O proceso pai continúa creando os seguintes fillos
    }

    // Agardamos a que todos os procesos rematen, proceso pai
    for (int k = 0; k < P; k++){
        waitpid(pids[k], NULL, 0);
    }

    printf("\n[PAI] Todos os procesos remataron.\n");

    //eliminamos semáforos e memoria compartida
    for (int i = 0; i < n_bufs; i++) {
        char vacio[32], cheo[32], muteado[32];
        nome_sem(vacio, "VAC", i);
        nome_sem(cheo, "CHE", i);
        nome_sem(muteado, "MUT", i);
        sem_close(sems_v[i]); 
        sem_unlink(vacio);
        sem_close(sems_c[i]); 
        sem_unlink(cheo);
        sem_close(sems_m[i]); 
        sem_unlink(muteado);
    }

    munmap(bufs, sizeof(DatosCompartidos) * n_bufs);
    unlink(FICHEIRO_SHM);

    return EXIT_SUCCESS;
}
