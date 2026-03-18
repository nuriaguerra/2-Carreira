# Arquitectura de Computadores

> Hardware · Segundo curso — 2º cuatrimestre

---

## De que vai esta materia

Arquitectura de Computadores retoma o que se viu en Fundamentos de Computadores e vai moito máis fondo: como se deseña un procesador real, como funcionan as optimizacións que fan que o hardware moderno sexa tan rápido, e como interactúa o software co hardware a baixo nivel.

É a materia que explica por que certos patróns de código son máis rápidos que outros, e por que iso importa.

---

## Contidos habituais

- Revisión do ciclo de instrución e introdución ao pipeline
- Riscos do pipeline: estruturais, de datos e de control — solucións
- Arquitecturas superescalares e execución fóra de orde
- Predicción de saltos
- Xerarquía de memoria en profundidade: caché (mapeamento directo, asociativo), política de reemplazo, coherencia
- Memoria virtual avanzada: TLB, protección
- Procesadores multinúcleo: consistencia de memoria, sincronización hardware
- Entrada/Saída: buses, DMA, interrupciones
- Introdución ás arquitecturas RISC-V ou ARM modernas

---

## Estrutura deste directorio

```
Arquitectura_de_Computadores/
├── README.md
├── teoria/
├── practicas/
│   └── ensamblador/    ← exercicios en baixo nivel
└── examenes/
```

---

## Consellos

- O pipeline é o concepto central de toda a materia — todo o demais constrúese sobre el.
- A caché é o que máis inflúe no rendemento real dun programa. Entender como funciona axuda a escribir código moito máis eficiente.
- Os riscos de datos (RAW, WAW, WAR) custan de memorizar sen un exemplo concreto. Traballa sempre con código real en ensamblador.
- Se o teu plan inclúe RISC-V: é unha arquitectura limpa e moderna, ideal para aprender. Aproveita os simuladores dispoñibles.

---

> ⚠ Este material é orientativo. Contrasta sempre co temario oficial da túa universidade.
