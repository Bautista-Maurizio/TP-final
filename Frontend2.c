//frontend2.c
#include <stdio.h>
#include <unistd.h>
#include "Backend.h"
#include "Frontend2.h"
#include "disdrv.h" //api del display matricial
#include "joydrv.h" //api del joystick 
#include <math.h>
#include "Words.h"

//boton del joystick 
int pressed_prev=0; //0 no apretado, 1 estaba apretado previamente
int hold=0; //tiempo que estuvo apretado el boton

void pi_init(void){
    joy_init(); //inicializo el joystick
    disp_init(); //inicializo display
    disp_clear(); //borra el buffer 
    disp_update(); 

    //estado del boton
    pressed_prev=0;
    hold=0;
}

void read_joy(joy_t * p){
    p->move  = 0;
    p->pause = 0;
    p->reset = 0;
    p->quit  = 0;

    //leo el joystick 
    joyinfo_t j= joy_read(); 

    //movimiento en x
    if (j.x > MOVE) {
        p->move = 1; //se mueve a la derecha 
    } 
    else if (j.x < -MOVE) {
        p->move = -1; //se mueve a la izq 
    } 
    else {
        p->move = 0; //no se mueve
    }

    //boton
    int pressed;
    if (j.sw != J_NOPRESS) {
        pressed = 1;
    } 
    else {
        pressed = 0;
    }

    //si esta apretado acumulo ms
    if (pressed==1){
        hold=hold + (int)(DT * 1000.0);
    }

    //si antes estaba libre y ahora apretado
    if((pressed_prev==0) && (pressed==1)){
        hold=0; //reseteo contaodr
    }

    //si antes estaba apretado y ahora no
    if ((pressed_prev==1)&& (pressed==0)){
        if (hold >= HOLD_FOR_QUIT) {
            p->quit = 1;
        } 
        else if (hold >= HOLD_FOR_RESET) {
            p->reset = 1;
        } 
        else {
            p->pause = 1;  //apretarlo poquito
        }
            
        hold= 0; //reinicio acumulador 
    }
    pressed_prev=pressed;

}

void sleep_frame(void){
    useconds_t frame_s= DT * 1000000; //1/FPS en microsegundos 
    usleep(frame_s); 
}

void pi_clear(void){
    disp_clear(); //limpio el buffer
    disp_update();
}

//ancho y alto de la matriz del disp
int disp_w(void){
    return (DISP_MAX_X - DISP_MIN) + 1;
}
int disp_h(void){
    return (DISP_MAX_Y - DISP_MIN) + 1;
}

//encender un led si x,y caen en el rango de la matriz 
void led_on(int x, int y){
    if (x < DISP_MIN) {
        //se fue por izq no dibujo
    } 
    else if (x > DISP_MAX_X) {
        //se fue por derecha no dibujo
    } 
    else {
        if (y < DISP_MIN) {
            //se fue por arriba
        } 
        else if (y > DISP_MAX_Y) {
            //se fue por abajo
        } 
        else {
            dcoord_t p; //estrctura con coordenadas de la matriz 
            p.x = x;
            p.y = y;
            disp_write(p, D_ON); //marca que la coordenada p tiene que estar encendida
        }
    }
}

int map_x(float x){
    //para que x no salga del rango del juego
    if (x < 0.0) {
        x = 0.0;
    } 
    else if (x > ANCHO) {
        x = ANCHO;
    } 
    float u = x / ANCHO; //x entre 0 y 1       
    int X = lroundf( u * (disp_w() - 1) ); //redondea al entero mas cercano
    if (X < DISP_MIN) {
        X = DISP_MIN;
    } 
    else if (X > DISP_MAX_X) {
        X = DISP_MAX_X;
    } 
    return X;
}

int map_y(float y){

    if (y < 0.0) {
        y = 0.0;
    } 
    else if (y > ALTO) {
        y = ALTO;
    } 
    
    float v = y / ALTO; //entre 0 y 1        
    int Y = lroundf( v * (disp_h() - 1) );//redondeo
    if (Y < DISP_MIN) {
        Y = DISP_MIN;
    } 
    else if (Y > DISP_MAX_Y) {
        Y = DISP_MAX_Y;
    } 
    return Y;        
}

//bloques
void draw_bricks(game_t *g){
    int r=0;
    while (r < BR_ROWS) { //recorro filas de grilla de bloques 

        int c = 0;
        while (c < BR_COLS) { //columnas
            brick_t* b = &g->bricks[r][c];

            //dibujo si el bloque esta vivo 
            if (b->alive !=0) {
                //centro con las coord del mundo
                float cx = b->x + (BR_W * 0.5);
                float cy = b->y + (BR_H * 0.5);

                //lo mapeo al disp
                int X = map_x(cx);
                int Y = map_y(cy);

                //enciendo el led
                led_on(X, Y);
            } 
            //si esta muerto no dibujo nada 

            c = c + 1;
        }

        r = r + 1;
    }
}

void draw_pause(void){
    //marco en el borde del disp
    //horizontales
    int x= DISP_MIN;
    while (x <= DISP_MAX_X) {
        led_on(x, DISP_MIN);
        led_on(x, DISP_MAX_Y);
        x = x + 1;
    }

    //verticales
    int y = DISP_MIN;
    while (y <= DISP_MAX_Y) {
        led_on(DISP_MIN,  y);
        led_on(DISP_MAX_X,y);
        y = y + 1;
    }
}

void pi_draw(game_t *g){
    disp_clear();
    draw_bricks(g); //bloques

    //vaus
    float vaus_px =2.0* g->vaus.half; //ancho del vaus en el disp

    //centro coordenadas en el display
    int cx = map_x(g->vaus.x);
    int vy = map_y(g->vaus.y);

    int len= lroundf((vaus_px / ANCHO) * (disp_w() - 1)); //largo del vaus

    //valido minimo y maximo
    if (len < 2) {
        len = 2;
    } 
    
    if (len > disp_w()) {
        len = disp_w();
    }

    //simetrico
    int half = len / 2;
    int dx = -half;
    while (dx <= half) {
        led_on(cx + dx, vy);
        dx = dx + 1;
    }

    //bola
    //led en la posicion actual
    int bx = map_x(g->ball.p.x);
    int by = map_y(g->ball.p.y);
    led_on(bx, by);
    
    //marco pausa
    if (g->paused == 1) {
        draw_pause();
    } 

    disp_update(); //mostrar

}

//para detectar los eventos y mostrar carteles 
void events(game_t *g){
    
    static int initialized = 0; //para ver si ya copie los valores previos 
    static int prev_lives = 0; //para guardar vidas del frame anterior
    static stage_t prev_stage = STAGE_TITLE; //estado del frame anterior 

    if (initialized == 0) {
        prev_lives = g->vidas; //cargo vidas actuales como previas 
        prev_stage = g->stage; //cargo stage actual como previo 
        initialized = 1;
        if(g->stage==STAGE_TITLE){
            show_menu();
        }
    } 
    else {
        //bajo una vida 
        if (g->vidas < prev_lives) {
            int lives_left = g->vidas; //vidas que quedan ahora 
            if (lives_left < 0) {
                lives_left = 0;
            } 
            else if (lives_left > 3) {
                lives_left = 3;
            } 
            show_life_lost(lives_left); 
        }

        //cambio stage
        if (g->stage != prev_stage) {
            if (g->stage == STAGE_GAME_OVER) {
                show_game_over();     
            } 
            else {
                if (g->stage == STAGE_WIN) {
                    show_win();           
                    show_score(g->score); 
                } 
                else {
                    if (g->stage == STAGE_TITLE) {
                        show_menu();      
                    }
                }
            }
        }

        //actualizo previos 
        prev_lives = g->vidas;
        prev_stage = g->stage;
    }
}