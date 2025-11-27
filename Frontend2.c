// frontend2.c  --- versión corregida para display 16x16
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "Backend.h"
#include "Frontend2.h"
#include "disdrv.h"
#include "joydrv.h"
#include "Words.h"

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

    joyinfo_t j = joy_read();

    // Movimiento horizontal
    if (j.x > MOVE)
        p->move = 1;
    else if (j.x < -MOVE)
        p->move = -1;

    // Botón
    int pressed = (j.sw != J_NOPRESS);

    if (pressed)
        hold += (int)(DT * 1000.0);

    if (!pressed_prev && pressed)
        hold = 0;

    if (pressed_prev && !pressed){
        if (hold >= HOLD_FOR_QUIT)
            p->quit = 1;
        else if (hold >= HOLD_FOR_RESET)
            p->reset = 1;
        else
            p->pause = 1;

        hold = 0;
    }

    pressed_prev = pressed;
}

// -----------------------------------
// FRAME
// -----------------------------------
void sleep_frame(void){
    usleep((useconds_t)(DT * 1000000));
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

    dcoord_t p = { x, y };
    disp_write(p, D_ON);
}

// -----------------------------------
// MAPEO de coordenadas del mundo → display 16x16
// -----------------------------------
// NOTA: ANCHO y ALTO son dimensiones de tu backend.
// Se normaliza a 0–15 para que quede centrado y proporcional.
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
    for (int r = 0; r < BR_ROWS; r++){
        for (int c = 0; c < BR_COLS; c++){
            brick_t *b = &g->bricks[r][c];

            if (b->alive){
                float cx = b->x + BR_W * 0.5f;
                float cy = b->y + BR_H * 0.5f;

                led_on(map_x(cx), map_y(cy));
            }
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

    // --- VAUS ---
    float vaus_px = 2.0f * g->vaus.half;

    int cx = map_x(g->vaus.x);
    int vy = map_y(g->vaus.y);

    int len = (int)lroundf((vaus_px / ANCHO) * 15.0f);

    if (len < 2) len = 2;
    if (len > 16) len = 16;

    int half = len / 2;
    for (int dx = -half; dx <= half; dx++)
        led_on(cx + dx, vy);

    // --- BOLA ---
    led_on(map_x(g->ball.p.x), map_y(g->ball.p.y));

    // --- PAUSA ---
    if (g->paused)
        draw_pause();

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

        if (g->stage == STAGE_TITLE)
            show_menu();
    }
    else{
        if (g->vidas < prev_lives){
            int v = g->vidas;
            if (v < 0) v = 0;
            if (v > 3) v = 3;
            show_life_lost(v);
        }

        if (g->stage != prev_stage){
            if (g->stage == STAGE_GAME_OVER)
                show_game_over();
            else if (g->stage == STAGE_WIN){
                show_win();
                show_score(g->score);
            }
            else if (g->stage == STAGE_TITLE)
                show_menu();
        }

        prev_lives = g->vidas;
        prev_stage = g->stage;
    }
}