# Base de Datos II

> Datos · Segundo curso — 2º cuatrimestre

---

## De que vai esta materia

Base de Datos II amplía o que se aprendeu en BD I cara a dous frontes: por un lado, a optimización e administración avanzada de bases de datos relacionais; por outro, os modelos non relacionais (NoSQL) que dominan boa parte da industria actual.

---

## Contidos habituais

- Optimización de consultas: plans de execución, índices avanzados, estatísticas
- Transacciones avanzadas: niveis de illamento, control de concorrencia (2PL, MVCC)
- Recuperación ante fallos: logs, checkpoints, protocolos ARIES
- Bases de datos distribuídas: fragmentación, replicación, consistencia eventual
- Modelos NoSQL: documento (MongoDB), clave-valor (Redis), columnar (Cassandra), grafo (Neo4j)
- Teorema CAP e BASE vs ACID
- Data Warehousing e OLAP: esquemas estrela e copo de neve
- Introdución ao Big Data: Hadoop, Spark (segundo o temario)

---

## Estrutura deste directorio

```
Base_de_Datos_II/
├── README.md
├── teoria/
├── practicas/
│   ├── sql/            ← optimización e consultas avanzadas
│   ├── mongodb/        ← exercicios NoSQL
│   └── modelos/
└── examenes/
```

---

## Consellos

- O teorema CAP é fundamental para entender por que non existe a base de datos perfecta para todos os casos. Enténdeo ben.
- NoSQL non é mellor nin peor que SQL — é diferente. A clave é saber cando usar cada un.
- Os plans de execución de SQL son moi útiles na vida real. Aprender a lelos é unha habilidade que se nota moito en entrevistas.
- MongoDB é a base de datos de documentos máis usada — se tes a oportunidade de practicar con ela, aproveita.

---

> ⚠ Este material é orientativo. Contrasta sempre co temario oficial da túa universidade.
