# Base de Datos I

> Datos · Segundo curso — 1º cuatrimestre

---

## De que vai esta materia

Case calquera aplicación do mundo real almacena datos de forma persistente — e a meirande parte faino nunha base de datos relacional. Esta materia ensína a deseñar, crear e consultar bases de datos relacionais correctamente.

SQL é unha das linguaxes máis usadas na industria e aquí aprendes a usala ben.

---

## Contidos habituais

- Modelo entidade-relación (ER): entidades, atributos, relacións, cardinalidade
- Paso do modelo ER ao modelo relacional
- Álxebra relacional: selección, proxección, xunción, unión, diferenza
- SQL: DDL (CREATE, ALTER, DROP) e DML (SELECT, INSERT, UPDATE, DELETE)
- Consultas avanzadas: JOIN, subconsultas, GROUP BY, HAVING, funcións de agregación
- Vistas, índices e procedementos almacenados
- Normalización: formas normais 1FN, 2FN, 3FN e BCNF
- Transacciones: propiedades ACID
- Introdución á seguridade e control de acceso en bases de datos

---

## Estrutura deste directorio

```
Base_de_Datos_I/
├── README.md
├── teoria/
├── practicas/
│   ├── sql/            ← scripts de consultas e exercicios
│   └── modelos/        ← diagramas ER
└── examenes/
```

---

## Consellos

- O deseño do modelo ER é o paso máis importante — un mal deseño orixina problemas que non se resolven con SQL.
- A normalización parece burocrática pero é o que distingue unha base de datos que funciona dunha que crea problemas con datos duplicados ou inconsistentes.
- Practica SQL con bases de datos reais. Hai moitos datasets públicos gratuítos para practicar.
- Os `JOIN` custan ao principio. Debuxa as táboas e o resultado esperado antes de escribir a consulta.

---

> ⚠ Este material é orientativo. Contrasta sempre co temario oficial da túa universidade.
