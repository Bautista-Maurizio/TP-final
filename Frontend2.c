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



// Estado del botón

static int pressed_prev = 0;
static int hold = 0;

void pi_init (void) {
    joy_init();
    disp_init();
    disp_clear();
    disp_update();

    pressed_prev = 0;
    hold = 0;
}


// Joystick

void read_joy (joy_t *p) {
    
    p->move = p->pause = p->reset = p->quit = 0;

    //leer joystick
    joyinfo_t j = joy_read();

    //movimiento
    int ax = j.x;
    int ay = j.y;

    int v = ( (ax >= 0 ? ax : -ax) >= (ay >= 0 ? ay : -ay) ) ? ax : ay;

    int dz = MOVE;                 
    	
    	if (dz < 80) {
    		dz = 80;         
    	}
    
    	if (v >  dz) {
   		p->move = +1;   //derecha
    	}
    
   	else if (v < -dz) {
   		p->move = -1;   //izquierda
    	}
    
    	else {
    	        p->move =  0;   //zona muerta
	}
    
    //boton: tap corto para pausa, tap largo para reset
    static int hold_ms = 0;

    int pressed = (j.sw != J_NOPRESS);
    	
    	if (pressed) {
    		 hold_ms += (int)(DT * 1000.0f);
    	}
    
    	if (!pressed_prev && pressed) {
    		 hold_ms = 0;
    	}

    	if (pressed_prev && !pressed) {
        	if (hold_ms >= HOLD_FOR_QUIT) {
        	      	p->quit  = 1;
        	}
        	
        	else if (hold_ms >= HOLD_FOR_RESET) {
        		p->reset = 1;
        	}
        	
        	else {
        	        p->pause = 1;  //tap corto
       		}
       
        hold_ms = 0;
    }
    pressed_prev = pressed;
}

// Frame

void sleep_frame (void) {
    struct timespec ts;
    ts.tv_sec  = 0;
    long ns = (long)(DT * 1e9);
    
    if (ns > 999999999L) { 
    	ns = 999999999L;
    }
   
    ts.tv_nsec = ns;
    nanosleep(&ts, NULL);
}

void pi_clear (void) {
    disp_clear();
    disp_update();
}


// Dimensiones fisicas del display

int disp_w (void) { 
	return 16; 
}


int disp_h (void) { 
	return 16; 
}


// LED seguro

void led_on (int x, int y) {
    
    if (x < 0 || x > 15 || y < 0 || y > 15) {
        return;
    }
    
    dcoord_t p = { x, y };
    disp_write(p, D_ON);
}


// Mapeo de coordenadas del mundo


int map_x (float x) {
    if (x < 0) {
    	x = 0;
    }
    
    if (x > ANCHO) {
     	x = ANCHO;
    }
    
    float u = x / ANCHO;
    
return (int)lroundf(u * 15.0f);  

}

int map_y (float y) {
    if (y < 0) {
    	y = 0;
    }
    
    if (y > ALTO) {
    	y = ALTO;
    }
    
    float v = y / ALTO;
    
return (int)lroundf(v * 15.0f);

}


// Dibujo de ladrillos

void draw_bricks (game_t *g) {
    int display_w = 16; 
    
    for (int i = 0; i < BR_ROWS; i++) {
 
        int Y_final;
        
        if (i == 4) {
     		Y_final = 3; // Fila de más abajo 
        }
        
        else if (i == 3) {
        	Y_final = 2; // Fila siguiente 
        }
        
        else if (i == 2) {
        	Y_final = 1; // Fila medio 
        }
        
        else  {
                Y_final = 0; // Filas 0 y 1 
        }
        
        
     for (int j = 0; j < BR_COLS; j++){
            
     brick_t *b = &g->bricks[i][j];
            
            if (!b->alive) {
            	continue; 
	    }
            
     int X0 = (j * display_w) / BR_COLS;
     int X1 = ((j + 1) * display_w) / BR_COLS; 
           
            X1 = X1 - 1; 

            if (X1 < X0) {
            	X1 = X0; 
	    }
            
            // DIBUJO
            for (int x = X0; x <= X1; x++){
                // Pintamos solo si está dentro de la pantalla
                if (x >= 0 && x < 16 && Y_final >= 0 && Y_final < 16) {
                    led_on(x, Y_final);
                }
            }
        }
    }
}


// Dibujo de pausa

void draw_pause (void) {
    for (int x = 0; x < 16; x++) {
        led_on(x, 0);
        led_on(x, 15);
    }
    
    for (int y = 0; y < 16; y++) {
        led_on(0, y);
        led_on(15, y);
    }
}


// Dibujo general

void pi_draw (game_t *g) {
    disp_clear();

    draw_bricks(g);

    // Vaus proporcional
    int cx = map_x(g->vaus.x);
    int vy = map_y(g->vaus.y);

    // calculo ancho proporcional
    float vaus_width_ratio = g->vaus.half / (ANCHO * 0.5f); // 0..1
    int len = (int)lroundf(vaus_width_ratio * 15.0f);        // escala 0..15

    if (len < 2) {
    	 len = 2;
    }
    
    if (len > 16) {
    	 len = 16;
    }
    
    int half = len / 2;
    
    for (int dx = -half; dx <= half; dx++) {
        led_on(cx + dx, vy);
    }
    
    // bola
    
    led_on(map_x(g->ball.p.x), map_y(g->ball.p.y));

    // pausa
    
    if (g->paused){
        draw_pause();
    }

    disp_update();
}

void esperar_boton_fuerte (void) {
    joy_t joy;
    // Esperamos a que suelte todo (para no leer un click viejo)
    do {
        
        read_joy(&joy);
        usleep(10000); 
    
    } while (joy.pause || joy.reset || joy.quit);

    // Esperamos click fuerte (simulado con pause, reset o quit)
    int apreto = 0;
    
    while (!apreto) {
        
        read_joy(&joy);
        
        // Si aprieta el botón hacia adentro (pause) o botones de sistema
        if (joy.pause || joy.reset || joy.quit) {
            apreto = 1;
        }
        
        usleep(10000);
    }
}

// EVENTS (Words)


void events (game_t *g) {
    
    // velocidad rapida (Solo al entrar en countdown)
    static stage_t prev_stage_check = STAGE_TITLE;
    
    if (g->stage == STAGE_COUNTDOWN && prev_stage_check != STAGE_COUNTDOWN) {
        
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

        if (g->stage == STAGE_TITLE) {
            show_menu();            
            esperar_boton_fuerte();
            stage_request_start(g); 
            g->paused = 0;
        }
    }
   
    else {
        // Perder Vida
        if (g->vidas < prev_lives) {
            int v = g->vidas; 
            	
            	if (v < 0) {
            	    v = 0;
                }
            
            show_life_lost(v);
        }

        // Cambio de Etapa
        if (g->stage != prev_stage){
            
            // CASO A: game over (perdiste)
           
            if (g->stage == STAGE_GAME_OVER){
                
                // Texto "GAME OVER"
                show_game_over();     
                
                // Dibujar puntaje (¡NUEVO!)
                
                disp_clear();
                draw_score_static(g->score);
                disp_update(); // Foto
                
                // Pausar 2 segundos (Para ver el puntaje)
                
                sleep(2);
                
                // Esperar click
                
                esperar_boton_fuerte(); 
                
                // Reiniciar (Recién aquí se borra el score)
                
                game_init(g);         
                
                // Menu
                
                disp_clear();
                show_menu();          
                
                esperar_boton_fuerte(); 
                stage_request_start(g);
                g->paused = 0;
            }
          
            // CASO B: Ganaste
            
            else if (g->stage == STAGE_WIN) {
                
                // Texto "GANASTE"
                
                show_win();           
                
                // Dibujar Puntaje
                
                disp_clear();                
                draw_score_static(g->score); 
                disp_update();               
                
                // Pausa 2 segundos
                
                sleep(2); 
                
                // Esperar click
                esperar_boton_fuerte(); 
                
                // Reiniciar
                game_init(g);         
                
                // Menu
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