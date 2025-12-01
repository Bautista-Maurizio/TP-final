#include "Backend.h"
#include "Raspi.h"
#include "Words.h"
#include "disdrv.h"

int main(void){
    game_t g;
    joy_t  j;
    int front_paused = 0; //0 no pausado, 1 pausado 
    int running = 1; 

    pi_init(); //inicializa display y joystick
    game_init(&g); //estado inicial del juego      

    while(running){
     
        read_joy(&j); //entrada 
        
        //pausa 
        if (j.pause && g.stage == STAGE_PLAY){
            front_paused = !front_paused; //invierte 
        }

        //update si no esta pausado 
        int allow_update = !(front_paused && g.stage == STAGE_PLAY);
        if (allow_update){
            //al backend le paso pause 0 para que no se mezclen 
            game_step(&g, DT, j.move, 0, j.reset);
        }

        events(&g); 

        //dibujo todo
        int paused_save = g.paused;
        if (front_paused && g.stage == STAGE_PLAY){
            g.paused = 1; //visual 
        }
        pi_draw(&g);
        g.paused = paused_save;

        //timing del frame 
        sleep_frame();
    }

    //apago display
    disp_clear();
    disp_update();
    return 0;
}
