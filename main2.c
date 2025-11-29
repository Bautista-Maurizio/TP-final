// main2.c — loop simple estable

#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <stdio.h>

#include "Backend.h"
#include "Frontend2.h"
#include "Words.h"
#include "disdrv.h"

#ifndef FPS
#define FPS 60
#endif

int main(void){
    game_t g;
    joy_t  j;
    int front_paused = 0;


    pi_init();          // inicializa display+joystick
    game_init(&g);      // deja el juego listo (usa tu backend)

    for(;;){
        // 1) entrada
        read_joy(&j);
        if (j.pause && g.stage == STAGE_PLAY){
            front_paused = !front_paused;
        }
        // 2) update — pasamos move/pause/reset derecho al backend
        int allow_update = !(front_paused && g.stage == STAGE_PLAY);
        if (allow_update){
            // pasamos pause = 0 al backend para no mezclar dos pausas
            game_step(&g, DT, j.move, 0 /*pause*/, j.reset);
        }

        // 3) eventos (vidas, cartel win/over). SIN forzar start.
        events(&g);

        // 4) draw
        int paused_save = g.paused;
        if (front_paused && g.stage == STAGE_PLAY){
            g.paused = 1;        // solo visual; no afecta la física porque ya la frenamos arriba
        }
        pi_draw(&g);
        g.paused = paused_save;

        // 5) timing
        sleep_frame();
    }

    // apago display
    disp_clear();
    disp_update();
    return 0;
}