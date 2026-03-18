# Computación Gráfica

> Programación · Segundo curso — 2º cuatrimestre

---

## De que vai esta materia

Computación Gráfica é das materias máis visualmente gratificantes da carreira: o que programas aparece directamente en pantalla. Estuda os fundamentos matemáticos e algorítmicos que fan posible os videoxogos, as simulacións, a realidade virtual e calquera sistema de visualización.

Require soltura con álxebra lineal — é onde se ven as aplicacións reais de matrices e vectores propios.

---

## Contidos habituais

- Fundamentos matemáticos: vectores, matrices, transformacións afíns
- Pipeline gráfico: vértices, primitivas, rasterización, fragmentos
- Transformacións 3D: traslación, rotación, escala, proxección perspectiva e ortográfica
- Sistemas de coordenadas: modelo, mundo, cámara, clip, pantalla
- Iluminación e sombra: modelos de Phong e Blinn-Phong, luces, materiais
- Texturas: mapeamento UV, filtrado, mipmapping
- Introdución a OpenGL ou WebGL
- Shaders: vertex shader e fragment shader en GLSL
- Curvas e superficies: Bézier, B-splines
- Introdución ao ray tracing

---

## Estrutura deste directorio

```
Computacion_Grafica/
├── README.md
├── teoria/
├── practicas/
│   ├── opengl/         ← proxectos e exercicios en OpenGL/WebGL
│   └── shaders/        ← código GLSL
└── examenes/
```

---

## Consellos

- Repasa álxebra lineal antes de empezar — multiplicación de matrices e cambios de base son o pan de cada día aquí.
- As matrices de transformación teñen un orde. Aplicar rotación antes que traslación non é o mesmo que ao revés. Este erro custa horas.
- Os shaders son o corazón da gráfica moderna. Canto antes te sintas cómoda escribindo GLSL, mellor.
- Three.js (WebGL) é moi accesible para ver resultados rápidos sen afogarse en código de inicialización de OpenGL. É bo complemento para aprender.

---

> ⚠ Este material é orientativo. Contrasta sempre co temario oficial da túa universidade.
