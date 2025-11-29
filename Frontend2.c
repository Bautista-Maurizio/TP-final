// frontend2.c
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "Backend.h"
#include "Frontend2.h"
#include "disdrv.h"
#include "joydrv.h"
#include "Words.h"
#include <time.h>
#include "Stage.h"


// -----------------------------------
// ESTADO DEL BOTÓN
// -----------------------------------
static int pressed_prev = 0;
static int hold = 0;

void pi_init(void){
    joy_init();
    disp_init();
    disp_clear();
    disp_update();

    pressed_prev = 0;
    hold = 0;
}

// -----------------------------------
// JOYSTICK
// -----------------------------------
void read_joy(joy_t *p){
    
    p->move = p->pause = p->reset = p->quit = 0;

    //leer hardware
    joyinfo_t j = joy_read();

    //movimiento
    int ax = j.x;
    int ay = j.y;

    int v = ( (ax >= 0 ? ax : -ax) >= (ay >= 0 ? ay : -ay) ) ? ax : ay;

    int dz = MOVE;                 
    if (dz < 80) dz = 80;         

    if (v >  dz)       p->move = +1;   //derecha
    else if (v < -dz)  p->move = -1;   //izquierda
    else               p->move =  0;   //zona muerta

    //boton: tap corto para pausa, tap largo para reset
    static int hold_ms = 0;

    int pressed = (j.sw != J_NOPRESS);
    if (pressed) hold_ms += (int)(DT * 1000.0f);
    if (!pressed_prev && pressed) hold_ms = 0;

    if (pressed_prev && !pressed){
        if (hold_ms >= HOLD_FOR_QUIT)       p->quit  = 1;
        else if (hold_ms >= HOLD_FOR_RESET) p->reset = 1;
        else                                p->pause = 1;  //tap corto
        hold_ms = 0;
    }
    pressed_prev = pressed;
}
// -----------------------------------
// FRAME
// -----------------------------------
void sleep_frame(void){
    struct timespec ts;
    ts.tv_sec  = 0;
    long ns = (long)(DT * 1e9);
    if (ns > 999999999L) ns = 999999999L;
    ts.tv_nsec = ns;
    nanosleep(&ts, NULL);
}
void pi_clear(void){
    disp_clear();
    disp_update();
}

// -----------------------------------
// DIMENSIONES FÍSICAS DEL DISPLAY
// -----------------------------------
int disp_w(void){ return 16; }
int disp_h(void){ return 16; }

// -----------------------------------
// LED seguro
// -----------------------------------
void led_on(int x, int y){
    if (x < 0 || x > 15 || y < 0 || y > 15)
        return;

    int Y = 15 - y;         

    dcoord_t p = { x, Y };
    disp_write(p, D_ON);
}

// -----------------------------------
// MAPEO de coordenadas del mundo
// -----------------------------------
int map_x(float x){
    if (x < 0) x = 0;
    if (x > ANCHO) x = ANCHO;

    float u = x / ANCHO;
    return (int)lroundf(u * 15.0f);  // 0-15
}

int map_y(float y){
    if (y < 0) y = 0;
    if (y > ALTO) y = ALTO;

    float v = y / ALTO;
    return (int)lroundf(v * 15.0f);
}

// -----------------------------------
// DIBUJO DE LADRILLOS
// -----------------------------------
void draw_bricks(game_t *g){
    for (int i = 0; i < BR_ROWS; i++){
        for (int j = 0; j < BR_COLS; j++){
            brick_t *b = &g->bricks[i][j];
            if (!b->alive) continue;

            // centro del ladrillo en el "mundo" (ANCHO×ALTO)
            float cx = b->x + BR_W * 0.5f;
            float cy = b->y + BR_H * 0.5f;

            // mapeo a 0..15 usando tus helpers
            int X = map_x(cx);
            int Y = map_y(cy);

            // enciende solo ese pixel
            led_on(X, Y);
        }
    }
}

// -----------------------------------
// DIBUJO DE PAUSA
// -----------------------------------
void draw_pause(void){
    for (int x = 0; x < 16; x++){
        led_on(x, 0);
        led_on(x, 15);
    }
    for (int y = 0; y < 16; y++){
        led_on(0, y);
        led_on(15, y);
    }
}

// -----------------------------------
// DIBUJO GENERAL
// -----------------------------------
void pi_draw(game_t *g){
    disp_clear();

    draw_bricks(g);

    // --- VAUS PROPORCIONAL ---
    int cx = map_x(g->vaus.x);
    int vy = map_y(g->vaus.y);

    // calculo ancho proporcional
    float vaus_width_ratio = g->vaus.half / (ANCHO * 0.5f); // 0..1
    int len = (int)lroundf(vaus_width_ratio * 15.0f);        // escala 0..15

    if (len < 2) len = 2;
    if (len > 16) len = 16;

    int half = len / 2;
    for (int dx = -half; dx <= half; dx++)
        led_on(cx + dx, vy);

    // --- BOLA ---
    led_on(map_x(g->ball.p.x), map_y(g->ball.p.y));

    // --- PAUSA ---
    if (g->paused){
        draw_pause();
    }

    disp_update();
}

// -----------------------------------
// EVENTS (Words)
// -----------------------------------
void events(game_t *g){
    static int initialized = 0;
    static int prev_lives = 0;
    static stage_t prev_stage = STAGE_TITLE;

    if (!initialized){
        prev_lives = g->vidas;
        prev_stage = g->stage;
        initialized = 1;

        if (g->stage == STAGE_TITLE){
            show_menu();               // muestra “START” y espera tap
            stage_request_start(g);    // <<< dispara READY (3-2-1) y luego PLAY
            g->paused = 0;
        }
    }
    else{
        if (g->vidas < prev_lives){
            int v = g->vidas;
            if (v < 0) v = 0;
            if (v > 3) v = 3;
            show_life_lost(v);
        }

        if (g->stage != prev_stage){
            if (g->stage == STAGE_GAME_OVER){
                show_game_over();
                show_menu();
                stage_request_start(g);   // <<< volver a arrancar tras tap
                g->paused = 0;
            }
            else if (g->stage == STAGE_WIN){
                show_win();
                show_score(g->score);
                // tras ver puntaje, mostrar menú y arrancar de nuevo
                show_menu();
                stage_request_start(g);   // <<<
                g->paused = 0;
            }
            else if (g->stage == STAGE_TITLE){
                show_menu();
                stage_request_start(g);   // <<<
                g->paused = 0;
            }
        }

        prev_lives = g->vidas;
        prev_stage = g->stage;
    }
}