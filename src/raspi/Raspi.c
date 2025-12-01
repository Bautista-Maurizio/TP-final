#include <unistd.h>
#include <math.h>
#include "Backend.h"
#include "Raspi.h"
#include "disdrv.h"
#include "joydrv.h"
#include "Words.h"
#include "Stage.h"
#include <time.h>



//estado del botón
static int pressed_prev = 0; //boton apretado previamente
static int hold = 0; //contar el mantenr apretado

void pi_init(void){
    joy_init(); //inicializa joystick
    disp_init(); //inicializa disp
    disp_clear(); //limpia buffer de leds
    disp_update(); //manda buffer al disp

    pressed_prev = 0; //no estaba apretado
    hold = 0; 
}


//joystick

void read_joy(joy_t *p) {   
    p->move=0; 
	p->pause=0; 
	p->reset=0;
	p->quit= 0;

    //leer joystick
    joyinfo_t j = joy_read();

    //movimiento
    int ax = j.x;
    int ay = j.y;

	int v;
	int absx = (ax >= 0) ? ax : -ax;  //|ax|
	int absy = (ay >= 0) ? ay : -ay;  //|ay|
	
	if (absx >= absy) {
	    v = ax;//domina x
	} 
	else {
	    v = ay; //domina y
	}
	
    int dz = MOVE;                 
    	
    	if (dz < 80) {
    		dz = 80; //aseguro minimo ochenta          
    	}
    
    	if (v >  dz) {
   		p->move = +1;   //derecha
    	}
    
	   	else if (v < -dz) {
	   		p->move = -1;   //izquierda
	    }
	    
	    else {
	    	p->move =  0; //zona muerta, quieto
		}
    
    //boton, tap corto para pausa, tap largo para reset
    static int hold_ms = 0; //acumula los ms que esta presionando 

    int pressed = (j.sw != J_NOPRESS); //esta apretado?
    	
    	if (pressed) {
    		 hold_ms += (int)(DT * 1000.0); //sumo el tiempo que esta siendo presionado
    	}
    
    	if (!pressed_prev && pressed) {
    		 hold_ms = 0; //recien empezo a apretar entonces reinicio contador 
    	}

    	if (pressed_prev && !pressed) { //solto el boton 
        	if (hold_ms >= HOLD_FOR_QUIT) {
        	      	p->quit= 1; //mantener mucho tiempo salir 
        	}
        	
        	else if (hold_ms >= HOLD_FOR_RESET) {
        		p->reset = 1; //manetener un poco reset
        	}
        	
        	else {
        	        p->pause = 1;  //tap corto
       		}
       
        hold_ms = 0; //reseteo contador 
    }
    pressed_prev = pressed; //actualizo estado previo 
}

//frame

void sleep_frame (void) {
    struct timespec ts; //estructura de time.h 
    ts.tv_sec= 0; //segundos enteros a dormir 0
    long ns = (long)(DT * 1e9); //convierto dt a nanosegundos 
    
    if (ns > 999999999L) { 
    	ns = 999999999L;
    }
   
    ts.tv_nsec = ns; 
    nanosleep(&ts, NULL); //duerme el trhead actual 
}

void pi_clear (void) {
    disp_clear();
    disp_update();
}


//dimensiones fisicas del display

int disp_w (void) { 
	return 16; 
}


int disp_h (void) { 
	return 16; 
}


//LED 

void led_on (int x, int y) {

	//valido
    if (x < 0 || x > 15 || y < 0 || y > 15) {
        return;
    }

	//escribo en el buffer del display
    dcoord_t p = { x, y };
    disp_write(p, D_ON);
}


//mapeo de coordenadas 

int map_x (float x) {
    if (x < 0) {
    	x = 0;
    }
    
    if (x > ANCHO) {
     	x = ANCHO;
    }
    
    float u = x / ANCHO; //normalizo 
    
	return (int)lroundf(u * 15.0); //rendondeo  

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


//dibujo de bloques 

void draw_bricks (game_t *g) {
    int display_w = 16; //col del disp 
    
    for (int i = 0; i < BR_ROWS; i++) {
 
        int Y_final; //mapeo de filas reales del disp porque sino se superponen
        
        if (i == 4) {
     		Y_final = 3; //fila de más abajo 
        }
        
        else if (i == 3) {
        	Y_final = 2; //fila siguiente 
        }
        
        else if (i == 2) {
        	Y_final = 1; //fila medio 
        }
        
        else  {
                Y_final = 0; //filas 0 y 1 
        }
        
        
        for (int j = 0; j < BR_COLS; j++){
                
            brick_t *b = &g->bricks[i][j]; 
                
            if (b->alive) {	    
                    
                int X0 = (j * display_w) / BR_COLS; //inicio
                int X1 = ((j + 1) * display_w) / BR_COLS; //fin 
                X1 = X1 - 1; //lo cierro 

                if (X1 < X0) {
                     X1 = X0; //almenos un led de ancho 
                }
                //dibujo 
                for (int x = X0; x <= X1; x++){
                    //pintamos solo si está dentro de la pantalla
                    if (x >= 0 && x < 16 && Y_final >= 0 && Y_final < 16) {
                        led_on(x, Y_final);
                    }
                }
            }
        }
    }
}


//dibujo de pausa

void draw_pause(void) {
    for (int x = 0; x < 16; x++) { //bordes horizontales 
        led_on(x, 0);
        led_on(x, 15);
    }
    
    for (int y = 0; y < 16; y++) { //bordes verticales 
        led_on(0, y);
        led_on(15, y);
    }
}


//dibujo general

void pi_draw (game_t *g) {
    disp_clear();

    draw_bricks(g); //bloques 

    //vaus proporcional al disp
    int cx = map_x(g->vaus.x); //centro x del vaus 
    int vy = map_y(g->vaus.y); //y del vaus 

    //calculo ancho proporcional del vaus 
    float vaus_width_ratio = g->vaus.half / (ANCHO * 0.5); 
    int len = lroundf(vaus_width_ratio * 15.0);  

	//tamaño min y max razonables 
    if (len < 2) {
    	 len = 2;
    }
    
    if (len > 16) {
    	 len = 16;
    }
    
    int half = len / 2; //busco la mitad 
    
    for (int dx = -half; dx <= half; dx++) {
        led_on(cx + dx, vy);
    }
    
    //bola 
    led_on(map_x(g->ball.p.x), map_y(g->ball.p.y));

    //pausa    
    if (g->paused){
        draw_pause();
    }

    disp_update();
}

void esperar_boton_fuerte (void) {
    joy_t joy;
    //esperamos a que suelte todo para no leer un click viejo
    do {     
        read_joy(&joy);
        usleep(10000); //10 ms 
    
    } while (joy.pause || joy.reset || joy.quit);

    //esperamos click fuerte 
    int apreto = 0;
    
    while(!apreto) {
        
        read_joy(&joy);
        
        //cuaalquier boton para confirmar
        if (joy.pause || joy.reset || joy.quit) {
            apreto = 1;
        }
        
        usleep(10000);
    }
}

//eventos 

void events (game_t *g) {
    
    //para que el tiempo que tarda en caer la pelota sea mas corto 
    static stage_t prev_stage_check = STAGE_TITLE;
    
    if (g->stage == STAGE_COUNTDOWN && prev_stage_check != STAGE_COUNTDOWN) {        
        g->stage_tsec = 0.5; //medio segundo    
    }
    
    prev_stage_check = g->stage; 

	//estados previos 
    static int initialized = 0;
    static int prev_lives = 0;
    static stage_t prev_stage = STAGE_TITLE;

    //inicializacion: mostramos start y comenzar
    if (!initialized){
        
        prev_lives = g->vidas;
        prev_stage = g->stage;
        initialized = 1;

        if (g->stage == STAGE_TITLE) {
            show_menu(); //scrollea start             
            esperar_boton_fuerte(); //espera boton 
            stage_request_start(g); //arranca 
            g->paused = 0;
        }
    }
   
    else {
        //perder vida
        if (g->vidas < prev_lives) {
            int v = g->vidas; 
            	
            	if (v < 0) {
            	    v = 0;
                }
            
            show_life_lost(v);
        }

        //cambio de Etapa
        if (g->stage != prev_stage){
            
            //game over 
           
            if (g->stage == STAGE_GAME_OVER){
                
                //muestro texto
                show_game_over();     
                
                //puntaje 
                
                disp_clear();
                show_score();
                draw_score_static(g->score);
                disp_update();
                
                //pausar 2 sec para ver el puntaje 
                sleep(2);
                
                //esperar click  
                esperar_boton_fuerte(); 
                
                //reiniciar 
                game_init(g);         

				//menu 
                disp_clear();
                show_menu();          
                
                esperar_boton_fuerte(); 
                stage_request_start(g);
                g->paused = 0;
            }
          
            //ganaste     
            else if (g->stage == STAGE_WIN) {
                
                //texto  
                show_win();           
                
                //puntaje
                disp_clear(); 
                show_score();               
                draw_score_static(g->score); 
                disp_update();               
                sleep(2); 

                esperar_boton_fuerte(); 
                
                game_init(g);         
    
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
