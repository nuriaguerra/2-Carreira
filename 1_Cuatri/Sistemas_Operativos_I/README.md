# Sistemas Operativos I

> Sistemas · Segundo curso — 1º cuatrimestre

---

## De que vai esta materia

O sistema operativo é a capa de software que xestiona o hardware e permite que os programas funcionen sen ter que preocuparse polos detalles físicos. Esta materia abre a caixa negra: como se xestionan os procesos, a memoria e os recursos de forma eficiente e segura.

É das materias que máis cambia a forma de entender o que un programa "é" cando se executa.

---

## Contidos habituais

- Concepto de sistema operativo: funcións e tipos
- Procesos e fíos (threads): creación, estados, comunicación
- Planificación de CPU: algoritmos FCFS, SJF, Round Robin, prioridades
- Concorrencia: condicións de carreira, exclusión mutua, semáforos, mutex
- Interbloqueo (deadlock): condicions, detección, prevención e recuperación
- Xestión de memoria: paxinación, segmentación, memoria virtual
- Algoritmos de substitución de páxinas: FIFO, LRU, Óptimo
- Sistema de ficheiros: estrutura, directorios, permisos
- Introdución á E/S e drivers

---

## Estrutura deste directorio

```
Sistemas_Operativos_I/
├── README.md
├── teoria/
├── practicas/
│   └── codigo/         ← exercicios con procesos, threads e semáforos
└── examenes/
```

---

## Consellos

- A concorrencia é o tema máis difícil e o máis importante. Dedícalle tempo extra — os erros de concorrencia son dos máis difíciles de depurar na vida real.
- Debuxa os diagramas de estados dos procesos. Ver as transicións axuda moito máis que lelos.
- O deadlock ten catro condicións necesarias. Se entendes cada unha, a prevención e detección sáense soas.
- Programa con `fork()`, `pthread` e semáforos en C polo menos unha vez — nin que sexa un exemplo pequeno. A teoría soa abstracta ata que ves o código.

---

> ⚠ Este material é orientativo. Contrasta sempre co temario oficial da túa universidade.
