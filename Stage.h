
#ifndef STAGE_H
#define STAGE_H

#include "Backend.h"
#ifdef USE_ALLEGRO
#include "Frontend1.h"
#endif 
#include <allegro5/allegro_font.h>
#include <math.h>

//prototipos
void stage_init(game_t* g); //arranca contdown y deja bola quieta        
void stage_set(game_t* g, stage_t s, float seconds);
void stage_update(game_t* g, float dt); //avanza timers y cambia de estado
void stage_on_life_lost(game_t* g);  //para cuando la bola cae al piso
void stage_on_bricks_cleared(game_t* g);  //cuando pasa de nivel 
void stage_show_title(game_t* g); //va al título
void stage_request_start(game_t* g); //pide pasar del título al juego
void ball_hold_center(game_t* g);
void ball_baja(game_t* g);
#ifdef USE_ALLEGRO
  void stage_draw_overlay(allegro_t* ui, game_t* g); //contdown
#endif

#endif 
