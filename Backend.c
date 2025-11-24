#include "Backend.h"
#include <math.h>
#include <stdlib.h>


//------------ PELOTA -----------------

//para mover la pelota
void move_bola(game_t* g, float time){

    bola_t* b = &g->ball;
    b->xprev = b->p.x;    
    b->yprev = b->p.y;     
    //distancia= vel * tiempo 
    b->p.x = b->p.x + b->vel.x * time;   
    b->p.y = b->p.y + b->vel.y * time;  
}

void ball_set_dir(bola_t* b, float ang) {
    float rad = PI * ang / 180.0; //convierto de grados a radianes
    b->vel.x = cos(rad) * b->speed; //devuelve vector unitario en la direccion que busco
    b->vel.y = sin(rad) * b->speed;
}

void ball_restart(game_t* g, int update_speed){
    
    if (g->ball.speed <= 0.0) {
        g->ball.speed = 240.0; //velocidad inicial 
    }

    //aumento velocidad si subo a niv 1
    if (update_speed == 1) {
        g->ball.speed += 80.0; 

        //limite de velocidad maxima 
        if (g->ball.speed > 700.0) {
            g->ball.speed = 700.0;
        }
    }

    g->ball.radio = BALL_R;

    //centro 
    g->ball.p.x = ANCHO * 0.5;
    g->ball.p.y = ALTO  * 0.33;
    
    //direccion random 
    int dir_select = rand() % 2;
    int dir;
    if (dir_select == 0) {
        dir = 230 + rand() % 30;
    } else {  
        dir = 310 - rand() % 30;
    }

    ball_set_dir(&g->ball, dir);  
}

//rebotes con las paredes 
int bola_choque_paredes(game_t* g){
    
    bola_t* b = &g->ball;

    //paredes izquierda y derecha
    if (b->p.x - b->radio < 0.0){
        b->p.x = b->radio;
        b->vel.x = -b->vel.x;
    }

    if (b->p.x + b->radio > ANCHO){
        b->p.x = ANCHO - b->radio;
        b->vel.x = -b->vel.x;
    }

    //techo
    if (b->p.y - b->radio < 0.0f){
        b->p.y = b->radio;
        b->vel.y = -b->vel.y;
    }

    //piso
    if (b->p.y + b->radio > ALTO) {
        g->vidas--; //pierde una vida 
        if (g->vidas <= 0) {
            game_init(g); //reinicia 
            return 0; //fin de juego
        }
        //si todavia quedan vidas solo reinicio la bola

        //pongo bola en el centro quieta 
        g->ball.p.x   = ANCHO * 0.5;
        g->ball.p.y   = ALTO  * 0.33;
        g->ball.vel.x = 0.0;
        g->ball.vel.y = 0.0;

        //la bola espera unos segundoas para bajar 
        g->ball_waiting = 1;
        g->wait_time    = 1.0; //un segundo

        return 0;              
    }

    return 1; //sigue en juego
}


//------------VAUS-----------------

//Para mover la base donde rebota la pelota
void vaus_update(game_t * p, float time, int move){ 
    //-1 izquierda, 1 derecha, 0 quieto

    p->vaus.x = p->vaus.x + move * p->vaus.speed * time; //distancia=velocidad*tiempo

    //para que no se salga del margen 
    float min = p->vaus.half + MARGEN;
    float max = ANCHO - (p->vaus.half + MARGEN);

    if (p->vaus.x < min){
        p->vaus.x = min;
    } 
    else if (p->vaus.x > max) {
        p->vaus.x = max;
    }
}

//rebote con el vaus 
void bola_choque_vaus(game_t* g){
        float rw = 2.0f * g->vaus.half;
    float rh = 12.0f;
    float rx = g->vaus.x - g->vaus.half;
    float ry = g->vaus.y - rh * 0.5f;

    ball_hit(g, rx, ry, rw, rh, 1);

}



//para que no se salga del campo
void vaus_set_x(game_t* g, float x){
    float min = g->vaus.half + MARGEN;
    float max = ANCHO - (g->vaus.half + MARGEN);
    if (x < min){
        x = min;
    }
    if (x > max){
        x = max;
    }
    g->vaus.x = x;
}

//---------BLOQUES----------

//estado inicial de los bloques
void bricks_reset_level(game_t* g)
{
    //Calculo el nivel 
    int niv = g->phase % 3; //resto de dividir por 3
    if (niv == 0){
        niv = 3; //si el resto da 0 el nivel es el 3 
    }

    g->bricks_left = 0; //contador de bloques presentes en el nivel 

    //recorro grilla de bloques 
    for (int i = 0; i < BR_ROWS; i++) {       
        for (int j = 0; j < BR_COLS; j++) {   

            brick_t* b = &g->bricks[i][j];

            // posicion del bloque 
            b->x = BR_LEFT + j * BR_W;
            b->y = BR_TOP  + i * BR_H;

            //flag que se fija si hay un bloque o no 
            int present = 0;

            // --------- PATRÓN DE BLOQUES SEGÚN EL NIVEL ---------

            if (niv == 1) {
                //nivel 1, pared completa 
                present = 1;

            } 
            else if (niv == 2) {
                //nivel 2, arco 
                int post_thick= 2; //grosor 
                int top_rows= 1; //filas de arriba 
                int bottom_rows= 1; //filas de abajo
            
                int is_top= (i < top_rows);
                int is_bottom= (i >= BR_ROWS - bottom_rows);
                int is_left_post= (j < post_thick);
                int is_right_post= (j >= BR_COLS - post_thick);
            
                if (is_top || is_bottom || is_left_post || is_right_post) {
                    present = 1;   // pongo ladrillo en el borde
                } 
                else {
                    present = 0;   // dentro del arco queda vacío
                }
            } 
            else { 
                //nivel 3, piramide 
                if ((j >= i) && (j < BR_COLS - i)) {
                    present = 1;
                } 
                else {
                    present = 0;
                }
            }

            // --------- CONFIGURACIÓN DE CADA BLOQUE ---------

            if (present) {
                //haya un bloque
                int tipo = (i % 5) + 1;
                b->alive = tipo;

                //resistencia del bloque segun el nivel
                if (niv == 1) {
                    //nivel 1, se rompen todos con un golpe
                    b->impac = 1;

                } 
                else if (niv == 2) {
                    //nivel 2, dilas 1 y 3 con dos golpes 
                    if (i == 1 || i == 3) {
                        b->impac = 2; //2 golpes
                    } 
                    else {
                        b->impac = 1; //1 golpe
                    }

                } else { 
                    //nivel 3, solo fila del medio 1 golpe
                    if (i == 0 || i == BR_ROWS - 1) {
                        b->impac = 3; //3 golpes
                    }
                    else if (i == 1 || i == BR_ROWS - 2) {
                        b->impac = 2; //2 golpes
                    }
                    else {
                        b->impac = 1; //1 golpe
                    }
                }

                //puntos dependiendo de cuantos golpes necesita cada bloque 
                if (b->impac == 1) {
                    b->points = 5; //un golpe
                } 
                else if (b->impac == 2) {
                    b->points = 10;  //dos golpes
                } 
                else { 
                    b->points = 20; //tres golpes 
                }

                //aumento el contador de bloque presentes en el nivel
                g->bricks_left++;

            } 
            else {
                //no hay bloque
                b->alive  = 0;
                b->impac  = 0;
                b->points = 0;
            }
        }
    }
}


void bola_choque_bricks(game_t* g){

    int rompio = 0; 

    int r = 0;
    while (r < BR_ROWS && rompio == 0) { //recorro filas 
        int c = 0;
        while (c < BR_COLS && rompio == 0) { //recorro columnas 
            brick_t* br = &g->bricks[r][c];
            if (br->alive != 0) { //si esta vivo
                float rx = br->x, ry = br->y;
                float rw = BR_W,  rh = BR_H; //giardo medidas

                if (ball_hit(g, rx, ry, rw, rh, 0) == 1){

                    //le pegua al ladrillo, le bajo los impactos
                    if (br->impac > 0) {
                        br->impac--;
                    }

                    //si ya no le queda impactos lo rompo
                    if (br->impac <= 0){
                        br->alive = 0;
                        g->bricks_left = g->bricks_left - 1; //un bloque menos
                        g->score = g->score + br->points; //sumo puntos según el ladrillo
                    }
                    rompio = 1;
                }
            }
            c = c + 1;
        }
        r = r + 1;
    }

    //subir nivel si se acabaron lso ladrillos 
    if (g->bricks_left == 0) {
        //si esta en el nivel 3 termina 
        if (g->phase >= 3) {
            g->game_over = 1; //fin del juego
            return; //no reinicio nada
        }
        else{
        //si no esta en el nivel 3 paso de nivel
        g->phase = g->phase + 1;
        g->vidas = 3; //reinicio las vidas       
        bricks_reset_level(g); //arma pared nuevo nivel
        ball_restart(g, 1); //reinicio bola y aumento velocidad 
        //pongo la bola en el centro quieta
        g->ball.p.x   = ANCHO * 0.5;
        g->ball.p.y   = ALTO  * 0.33;
        g->ball.vel.x = 0.0;
        g->ball.vel.y = 0.0;
        //activo modo espera
        g->ball_waiting= 1;
        g->wait_time= 1.0; //1 segundo

        }
    }
}

int ball_hit(game_t* g, float rx, float ry, float rw, float rh, int is_vaus){
    bola_t* b = &g->ball;

    //hay solapamiento? 
    int overlap_now = (b->p.x + b->radio > rx) && (b->p.x - b->radio < rx + rw) && (b->p.y + b->radio > ry) && (b->p.y - b->radio < ry + rh);

    //si no hay solapamiento no hay choque 
    if (overlap_now==0){
        return 0;
    }

    //veo solapamientos previos 
    int overlap_prev_x = (b->xprev + b->radio > rx) && (b->xprev - b->radio < rx + rw);
    int overlap_prev_y = (b->yprev + b->radio > ry) && (b->yprev - b->radio < ry + rh);

    if (!overlap_prev_x && overlap_prev_y) {
        //choque lateral 
        if(b->xprev < rx){
            b->p.x = rx - b->radio; //venia de la izquierda 
        }
        else if(b->xprev > rx+rw){
            b->p.x = rx + rw + b->radio; //venia de la derecha
        }
        b->vel.x = -b->vel.x;
        return 1;
    } 
    else {
        //choque vertical 
        if (b->yprev < ry){
            b->p.y = ry - b->radio; //venia de arriba   
        }
        else if(b->yprev > ry+rh){
            b->p.y = ry + rh + b->radio; //venia de abajo
        }
        b->vel.y = -b->vel.y;

        //si es el vaus 
        if (is_vaus) {
            float cx = b->p.x;  //centro x de la bola 
        
            if (cx < rx + 22.0){ 
                ball_set_dir(b, 210.0);
            }
            else if(cx < rx + 50.0){
                ball_set_dir(b, 250.0);
            }
            else if (cx > rx + (rw-22.0)){
                ball_set_dir(b, 330.0);
            }
            else{                            
                ball_set_dir(b, 290.0);
            }
        }
        return 1;
    }
}

void bricks_init(game_t* g){
    g->phase = 1; //nivel 1
    bricks_reset_level(g); //arma pared
}

//inicializacion 
void game_init(game_t * p){

    //posicion inicial 
    //p->vaus.half= 50;  
    //PARA TESTEAR NOMAS
    p->vaus.half = (ANCHO - 2*MARGEN) / 2; 
    p->vaus.x= ANCHO * 0.5;
    p->vaus.y= ALTO - 40.0;
    p->vaus.speed= 420.0;     

    p->vidas= 3; 
    p->score = 0; 
    p->game_over = 0;
    p->ball.speed = 240.0;
    p->phase = 1; 
    p->ball_waiting=0; 
    p->wait_time= 0.0; 
    bricks_init(p); //arma la pared

    //bola en el centro, quieta
    p->ball.p.x= ANCHO * 0.5;
    p->ball.p.y= ALTO  * 0.33;
    p->ball.vel.x= 0.0;
    p->ball.vel.y= 0.0;

    //modo espera inicial
    p->ball_waiting= 1;
    p->wait_time= 1.0; 
}

void game_step(game_t* p, float time, int move, int pause, int reset){
    if (p-> game_over==1){
        return; //si el juego termino no hago nada
    }
    if (reset==1){
        game_init(p); //reseteo el juego 
        return;
    }
    if(pause==1){
        return; //si esta en pausa no hago nada
    }

    //bola en modo espera
    if (p->ball_waiting){
        p->wait_time -= time; //voy descontando el tiempo
        if (p->wait_time <= 0.0) {
            //termino la espera
            p->ball_waiting = 0;

            if (p->ball.speed <= 0.0) {
                p->ball.speed = 240.0;
            }

            //hago que baje la bola
            ball_set_dir(&p->ball, 90.0);
        }
    }

    //paso parametros a las funciones 
    vaus_update(p, time, move);
    //muevo la bola si NO está esperando
    if (!p->ball_waiting) {
        move_bola(p, time);
    }  
    bola_choque_vaus(p);   
    bola_choque_bricks(p);   
    if (p->game_over){
        return;   
    }
    if (!p->ball_waiting && (bola_choque_paredes(p) == 0)){
        return;
    }   

}
