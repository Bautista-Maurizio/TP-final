#include <stdio.h>
#include <stdlib.h>

#include "Backend.h"
#include "Frontend1.h"

int main(void)
{
    game_t     game;
    allegro_t  allegro;

    // órdenes del jugador
    int move  = 0;
    int pause = 0;
    int reset = 0;
    int quit  = 0;

    // 1) Inicializo lógica del juego
    game_init(&game);

    // 2) Inicializo Allegro (ventana 800x600, igual que ANCHO/ALTO)
    if (!allegro_init(&allegro, &game, ANCHO, ALTO)) {
        fprintf(stderr, "Error inicializando Allegro\n");
        return 1;
    }

    // 3) Bucle principal
    while (!quit) {

        // Leo teclado/mouse y lleno move / pause / reset / quit
        allegro_read_input(&allegro, &game, &move, &pause, &reset, &quit);

        // Solo actualizo/dibujo cuando el timer lo pide
        if (allegro_should_draw(&allegro)) {

            // avanzo la simulación un paso
            game_step(&game, 1.0f / FPS, move, pause, reset);

            // pause y reset son “eventos” de un solo uso
            reset = 0;

            // dibujo todo
            allegro_draw(&allegro, &game);
        }
    }

    // 4) Libero recursos de Allegro
    allegro_shutdown(&allegro);

    return 0;
}
