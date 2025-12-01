#include <stdio.h>
#include "Backend.h"
#include "Allegro.h"
#include "Stage.h"

int main(void) {
    //estado del juego y allegro
    game_t g;
    allegro_t ui;

    game_init(&g); //inicializacion del juego 

    //defino tamaños 
    int win_w = ANCHO;
    int win_h = ALTO;

    if (allegro_init(&ui, &g, win_w, win_h)==0) {
        fprintf(stderr, "No se pudo inicializar Allegro.\n");
        return 1; //error 
    }

    //flags
    int move  = 0;  //-1: izquierda, 1: derecha, 0: quieto
    int pause = 0;  //espacio para pausar
    int reset = 0;  //M para ir al menu
    int quit  = 0;  //esc o cerrar ventana para cerrarlo

    while (!quit) {

        allegro_read_input(&ui, &g, &move, &pause, &reset, &quit); //procesa eventos de teclado y mouse 

        //si se toco la m volvemos al menu
        if (reset) {
            game_init(&g); //reseteamos 
            stage_show_title(&g); //volvemos al menú inicial
            reset = 0;     
        }
        if (allegro_should_draw(&ui)) {
            float dt = 1.0 / FPS; //paso de tiempo

            game_step(&g, dt, move, pause, 0); //avanza la logica del juego 
            allegro_draw(&ui, &g); //dibuja la escena 
        }

    }

    allegro_shutdown(&ui); //destruyo
    return 0; 
}