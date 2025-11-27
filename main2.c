// main2.c  -- versión Raspi (display + joystick)

#include <stdio.h>
#include <unistd.h>
#include "Backend.h"
#include "Frontend2.h"
#include "Words.h"

int main(void){
    game_t g;
    joy_t j;
    int quit = 0;

    // inicializo hardware Raspi
    pi_init();

    // inicializo lógica del juego (vidas, pared, TITLE, bola quieta, etc.)
    game_init(&g);

    while (!quit) {

        // 1) Leo joystick
        read_joy(&j);

        // 2) Mapear botón corto a "START" cuando estoy en el título
        if (g.stage == STAGE_TITLE && j.pause) {
            stage_request_start(&g);  // le digo al backend "arrancá el juego"
            j.pause = 0;              // NO lo uso como pausa en este caso
        }

        // 3) Reset
        if (j.reset) {
            game_init(&g);
            // después de game_init ya vuelve a TITLE con la bola quieta
        }

        // 4) Quit (salir del juego)
        if (j.quit) {
            quit = 1;
        }

        // 5) Avanzar la lógica del juego
        //    DT es 1.0/FPS definido en Frontend2.h
        game_step(&g, DT, j.move, j.pause, j.reset);

        // 6) Eventos de HUD (vidas, game over, win, título)
        events(&g);

        // 7) Dibujar en la matriz: baus, bola y ladrillos
        pi_clear();
        pi_draw(&g);
        sleep_frame();
    }

    pi_clear();
    return 0;
}