# Arkanoid - Trabajo Práctico Final

Este proyecto es una implementación del juego **Arkanoid** desarrollado en lenguaje **C**. 

La característica principal de este desarrollo es su arquitectura modular, que separa la lógica del juego (Backend) de la interfaz gráfica (Frontend). Esto permite que el mismo juego pueda compilarse y ejecutarse en dos plataformas totalmente distintas compartiendo el mismo núcleo lógico.

## Plataformas 

1.  **Raspberry Pi:** Utilizando una Matriz de LEDs 16x16 y un Joystick analógico.
2.  **PC:** Utilizando la librería gráfica **Allegro 5**.

---

## Características Técnicas

### Arquitectura Backend/Frontend
* **Backend:** Maneja toda la física, colisiones, estados del juego (Menú, Juego, Ganar, Perder), puntajes y niveles.
* **Frontend:** Se encarga de dibujar el estado actual del juego y leer los inputs del usuario.
    * Raspi.c: versión para Raspberry Pi (Drivers de Joystick y Display).
    * Allegro.c: versión para PC (Allegro).

### Juego 
* **Sistema de Niveles:** 3 niveles con dificultad progresiva (Pared, Arco y Pirámide)
* **Dificultad:** aumenta la velocidad y la cantidad de choques para romper el bloque, lo que da mayor puntaje.
* **Vidas:** se tienen 3 vidas las cuales no se renuevan en todo el juego.

---

## Controles

El juego se controla mediante el Joystick físico (en Raspberry) o Teclado (en PC).

**Mover Vaus** Joystick| Flechas Izq/Der, mouse
**Pausar** Click fuerte | Espacio

---

### Compilación

El proyecto utiliza un `Makefile` para gestionar la compilación en ambas plataformas.

**1. Para Raspberry Pi:**
make raspi genera el ejecutable arkopi

**2. Para PC:**
make allegro genera el ejecutable arkanoid 
