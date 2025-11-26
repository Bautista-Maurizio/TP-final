#include "Backend.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#ifndef FRONTEND1_H
#define FRONTEND1_H


#define FPS 60.0

typedef struct {
    ALLEGRO_DISPLAY * display;
    ALLEGRO_EVENT_QUEUE * queue; 
    ALLEGRO_TIMER* timer;
    int pixel_1;
    int pixel_2; //para el tamaño de la ventana de allegro 
    int sx;
    int sy; //para convertir de celda a pixel 

    //para bitmap
    ALLEGRO_BITMAP* bmp_background; //fondo
    ALLEGRO_BITMAP* bmp_brick[6]; //bloque
    ALLEGRO_BITMAP* bmp_paddle; //vaus  
    ALLEGRO_BITMAP* bmp_ball; //bola 
    ALLEGRO_FONT* font_small;  //texto para info
    ALLEGRO_FONT* font_big; //texto para titulos y contdown 
    ALLEGRO_AUDIO_STREAM* musica;
    ALLEGRO_SAMPLE* efecto_rebote;
    

}allegro_t;

//prototipos 
int  allegro_init(allegro_t* p, game_t* g, int pixel_1, int pixel_2);
void allegro_read_input(allegro_t* p, game_t* g, int* move, int* pause, int* reset, int* quit);
int  allegro_should_draw(allegro_t* p);
void allegro_draw(allegro_t* p, game_t* g);
int  celda2pixel_x(allegro_t* g, float x);
int  celda2pixel_y(allegro_t* g, float y);
void allegro_shutdown(allegro_t* p);
//parqa bitmaps
ALLEGRO_BITMAP* borde_bitmap(int w, int h, ALLEGRO_COLOR fill, ALLEGRO_COLOR borde);
ALLEGRO_BITMAP* ball_bitmap (int r, ALLEGRO_COLOR fill, ALLEGRO_COLOR borde);
ALLEGRO_BITMAP* paddle_bitmap(int w, int h);
ALLEGRO_COLOR   color_for_brick(int t);


#endif 