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
    dcoord_t p = { x, y };
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
/* void draw_bricks(game_t *g){
    // Usamos 16 directo para no depender de funciones externas
    int display_w = 16; 
    
    // Recorremos las 5 filas
    for (int i = 0; i < BR_ROWS; i++){
        
        // TRUCO PARA QUE NO SE SOLAPEN VERTICALMENTE:
        // Calculamos la Y basándonos en la posición real, PERO...
        // ...le sumamos 'i' parsimoniosamente si vemos que se solapan? 
        // No, mejor: Usamos map_y normal, pero forzamos altura 1.
        
        int Y0 = map_y(g->bricks[i][0].y); // Tomamos la Y de la fila
        
        // CORRECCIÓN MANUAL:
        // Como la pantalla es tan chica, las filas 2 y 3 suelen caer en el mismo pixel.
        // Vamos a separarlas artificialmente usando el índice 'i'.
        // Fila 0 -> Y calculada
        // Fila 1 -> Y calculada + un poquito si hace falta...
        // MEJOR: Mapeo directo hardcodeado para que se vea prolijo en 16x16
        // Fila 0 a 4 las ponemos en los pixels 1 a 5 (dejando el 0 para el techo)
        int Y_final = 1 + i; 

        for (int j = 0; j < BR_COLS; j++){
            brick_t *b = &g->bricks[i][j];
            
            // Si está muerto, saltamos (el pixel quedará apagado si nadie más lo usa)
            if (!b->alive) continue; 

            // MATEMÁTICA HORIZONTAL (La que te funcionaba bien para la pared sólida)
            // Divide los 16 pixels entre 12 bloques equitativamente
            int X0 = (j * display_w) / BR_COLS;
            int X1 = ((j + 1) * display_w) / BR_COLS; 
            X1 = X1 - 1; // Restamos 1 para no invadir al vecino

            // Seguridad por si X1 quedó menor a X0 (bloques muy finitos)
            if (X1 < X0) X1 = X0; 

            // DIBUJO (Solo una línea horizontal)
            for (int x = X0; x <= X1; x++){
                // Verificamos que esté en pantalla (0-15) antes de pintar
                if (x >= 0 && x < 16 && Y_final >= 0 && Y_final < 16){
                    led_on(x, Y_final);
                }
            }
        }
    }
}
*/

void draw_bricks(game_t *g){
    int display_w = 16; 
    
    for (int i = 0; i < BR_ROWS; i++){
        
        // --- TRUCO DE COMPRESIÓN INTELIGENTE ---
        // Objetivo: Que ocupe poco espacio vertical (0 a 3) pero que las
        // filas de abajo (3 y 4) no se toquen para que los golpes se vean bien.
        
        int Y_final;
        
        if (i == 4)      Y_final = 3; // Fila de más abajo (se rompe primero) -> Pixel 3 (Exclusivo)
        else if (i == 3) Y_final = 2; // Fila siguiente -> Pixel 2 (Exclusivo)
        else if (i == 2) Y_final = 1; // Fila medio -> Pixel 1 (Exclusivo)
        else             Y_final = 0; // Filas 0 y 1 (Techo) -> Pixel 0 (Compartido)
        
        // Resultado: Pared de solo 4 pixels de alto (Y=0 a Y=3).
        // Te quedan 12 pixels libres para jugar.

        for (int j = 0; j < BR_COLS; j++){
            brick_t *b = &g->bricks[i][j];
            
            if (!b->alive) continue; 

            // MATEMÁTICA HORIZONTAL (La que funciona bien)
            int X0 = (j * display_w) / BR_COLS;
            int X1 = ((j + 1) * display_w) / BR_COLS; 
            X1 = X1 - 1; 

            if (X1 < X0) X1 = X0; 

            // DIBUJO
            for (int x = X0; x <= X1; x++){
                // Pintamos solo si está dentro de la pantalla
                if (x >= 0 && x < 16 && Y_final >= 0 && Y_final < 16){
                    led_on(x, Y_final);
                }
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

/*void esperar_toque(void){
    joy_t joy;
    // 1. Esperamos a que el jugador SUELTE todo (para no detectar el click anterior)
    do {
        read_joy(&joy);
        usleep(10000); // Espera 10ms
    } while (joy.move != 0 || joy.pause || joy.reset || joy.quit);

    // 2. Esperamos a que el jugador APRIETE algo (cualquier botón o movimiento)
    do {
        read_joy(&joy);
        usleep(10000); 
    } while (joy.move == 0 && !joy.pause && !joy.reset && !joy.quit);
}
    */

void esperar_boton_fuerte(void){
    joy_t joy;
    // 1. Esperamos a que suelte todo (para no leer un click viejo)
    do {
        read_joy(&joy);
        usleep(10000); 
    } while (joy.pause || joy.reset || joy.quit);

    // 2. Esperamos CLICK FUERTE (simulado con pause, reset o quit)
    int apreto = 0;
    while(!apreto){
        read_joy(&joy);
        // Si aprieta el botón hacia adentro (pause) o botones de sistema
        if (joy.pause || joy.reset || joy.quit) {
            apreto = 1;
        }
        usleep(10000);
    }
}

// -----------------------------------
// EVENTS (Words)
// -----------------------------------
/*
void events(game_t *g){
    
    // --- IMPORTANTE: BORRÉ EL TRUCO QUE ESTABA ACÁ ARRIBA ---
    // Ya no forzamos el tiempo en cada frame para evitar que se congele.

    static int initialized = 0;
    static int prev_lives = 0;
    static stage_t prev_stage = STAGE_TITLE;
    if (g->stage == STAGE_COUNTDOWN && prev_stage != STAGE_COUNTDOWN){
        g->stage_tsec = 0.5; // Medio segundo y arranca (Seguro)
    }

    // 1. INICIALIZACIÓN (Primera vez que corres el juego)
    if (!initialized){
        prev_lives = g->vidas;
        prev_stage = g->stage;
        initialized = 1;

        if (g->stage == STAGE_TITLE){
            show_menu();            
            esperar_boton_fuerte();
            
            stage_request_start(g); 
            g->stage_tsec = 0.1; // <--- VELOCIDAD (Se asigna UNA sola vez)
            
            g->paused = 0;
        }
    }
    else{
        // 2. PERDIDA DE VIDA
        if (g->vidas < prev_lives){
            int v = g->vidas; if (v < 0) v = 0;
            show_life_lost(v);
        }

        // 3. CAMBIOS DE ESTADO (Ganar o Perder)
        if (g->stage != prev_stage){
            
            // --- GAME OVER ---
            if (g->stage == STAGE_GAME_OVER){
                show_game_over();     
                esperar_boton_fuerte(); 
                
                game_init(g);         
                show_menu();          
                esperar_boton_fuerte(); 
                
                stage_request_start(g);
                g->stage_tsec = 0.1; // <--- VELOCIDAD (Una sola vez)
                
                g->paused = 0;
            }
            // --- GANASTE ---
            else if (g->stage == STAGE_WIN){
                show_win();           
                
                disp_clear(); 
                draw_score_static(g->score); 
                disp_update(); 
                
                esperar_boton_fuerte(); 
                
                game_init(g);         
                disp_clear();
                show_menu();
                
                esperar_boton_fuerte();
                
                stage_request_start(g);
                g->stage_tsec = 0.1; // <--- VELOCIDAD (Una sola vez)
                
                g->paused = 0;
            }
        }
    }
    prev_lives = g->vidas;
    prev_stage = g->stage;
}
*/
void events(game_t *g){
    
    // --- VELOCIDAD RÁPIDA (Solo al entrar en countdown) ---
    static stage_t prev_stage_check = STAGE_TITLE;
    if (g->stage == STAGE_COUNTDOWN && prev_stage_check != STAGE_COUNTDOWN){
        #ifdef PI_16x16
            g->stage_tsec = 0.5; // Medio segundo
        #endif
    }
    prev_stage_check = g->stage; 

    static int initialized = 0;
    static int prev_lives = 0;
    static stage_t prev_stage = STAGE_TITLE;

    // Inicialización
    if (!initialized){
        prev_lives = g->vidas;
        prev_stage = g->stage;
        initialized = 1;

        if (g->stage == STAGE_TITLE){
            show_menu();            
            esperar_boton_fuerte();
            stage_request_start(g); 
            g->paused = 0;
        }
    }
    else{
        // Perder Vida
        if (g->vidas < prev_lives){
            int v = g->vidas; if (v < 0) v = 0;
            show_life_lost(v);
        }

        // Cambio de Etapa
        if (g->stage != prev_stage){
            
            // ---------------------------------------------
            // CASO A: GAME OVER (PERDISTE)
            // ---------------------------------------------
            if (g->stage == STAGE_GAME_OVER){
                
                // 1. Texto "GAME OVER"
                show_game_over();     
                
                // 2. DIBUJAR PUNTAJE (¡NUEVO!)
                disp_clear();
                draw_score_static(g->score);
                disp_update(); // Foto
                
                // 3. PAUSA 2 SEGUNDOS (Para ver el puntaje)
                sleep(2);
                
                // 4. ESPERAR CLICK
                esperar_boton_fuerte(); 
                
                // 5. REINICIAR (Recién aquí se borra el score)
                game_init(g);         
                
                // 6. MENÚ
                disp_clear();
                show_menu();          
                
                esperar_boton_fuerte(); 
                stage_request_start(g);
                g->paused = 0;
            }
            
            // ---------------------------------------------
            // CASO B: GANASTE
            // ---------------------------------------------
            else if (g->stage == STAGE_WIN){
                
                // 1. Texto "GANASTE"
                show_win();           
                
                // 2. DIBUJAR PUNTAJE
                disp_clear();                
                draw_score_static(g->score); 
                disp_update();               
                
                // 3. PAUSA 2 SEGUNDOS
                sleep(2); 
                
                // 4. ESPERAR CLICK
                esperar_boton_fuerte(); 
                
                // 5. REINICIAR
                game_init(g);         
                
                // 6. MENÚ
                disp_clear();
                show_menu();
                
                esperar_boton_fuerte();
                stage_request_start(g);
                g->paused = 0;
            }
        }
    }
    prev_lives = g->vidas;
    prev_stage = g->stage;
}