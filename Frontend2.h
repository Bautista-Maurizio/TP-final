#ifndef FRONTEND2_H
#define FRONTEND2_H

#include "Backend.h"

#define FPS 60
#define DT ((1.0)/(FPS))
#define MOVE 40
#define HOLD_FOR_RESET 2000 //esos ms para reset 
#define HOLD_FOR_QUIT 5000 //esos ms para salir 


//estrcturas para el joystick
typedef struct {
    int move;
    int pause;
    int reset;
    int quit;
} joy_t;

//prototipos
void pi_init(void);
void read_joy(joy_t * p);
void sleep_frame(void);
void pi_clear(void);
int disp_w(void);
int disp_h(void);
void led_on(int x, int y);
int map_y(float y);
int map_x(float x);
void draw_bricks(game_t *g);
void draw_pause(void);
void pi_draw(game_t *g);
void events(game_t *g);

#endif 