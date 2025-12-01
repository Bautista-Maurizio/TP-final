#ifndef BACKEND_H
#define BACKEND_H

//dimensiones del campo
#define ANCHO 800
#define ALTO 600
#define MARGEN 1 // margen para que rebote  

#define BR_LEFT 4 //margen izquierdo de la pared
#ifdef PI_16x16
    #define BR_TOP 0
    #define BR_H 40
#else
    #define BR_TOP 64 //distancia desde el techo
    #define BR_H  18 // alto de cada bloque
#endif
#define BR_COLS 12
#define BR_ROWS 5
#define BR_W ((ANCHO - 2*BR_LEFT) / BR_COLS)
#define BALL_R 6 //radio de la bola

#ifndef PI
#define PI 3.14159265358979323846
#endif

//prototipos de estructuras
//vector para ubicar en x,y
typedef struct { 
    float x;
    float y; 
} vec_t;

//estados del juego
typedef enum {
    STAGE_PLAY,  //jugando normal
    STAGE_COUNTDOWN, //3,2,1 antes de caer la bola
    STAGE_LEVEL_UP, //nivel
    STAGE_GAME_OVER, //perdiste
    STAGE_WIN, //ganaste 
    STAGE_TITLE //inicio
} stage_t;

//vaus (la plataforma donde rebota la pelota)
typedef struct {
    float x; // centro en x en celda
    float y; // fila fija 
    int half; // la mitad del ancho de la plataforma
    float speed; // velocidad 
} vaus_t;

//estado de la bola
typedef struct {
    vec_t p; // posición
    vec_t vel; // velocidad de la bola
    float radio; // radio en celdas
    float speed; 
    float xprev;
    float yprev; //posicion previa
}bola_t;

//bloques de arriba 
typedef struct {
    //0 =roto; >0 =vivo
    int alive;
    float x;
    float y; 
    int impac; //para los que necesitan mas de un golpe para romperse
    int points; //puntaje al romperlo 
} brick_t;

//estado del juego 
typedef struct {
    vaus_t vaus; 
    bola_t ball;
    int vidas;
    int score;
    int game_over; //0= no terminó, 1= terminó
    brick_t bricks[BR_ROWS][BR_COLS]; 
    int bricks_left;
    int phase; 
    int ball_waiting; 
    float wait_time; //para cuando pierde una vida 
    int game_won; //cuando termina el juego y gana
    stage_t stage; //estado actual
    float stage_tsec; //tiempo restante del estado actual
    int paused; //para texto de pausa 
    int start_requested; //para menu de inicio 
}game_t;

//ordenes del jugador
typedef struct { 
    int move; 
    int pause; 
    int reset; 
    int quit; 
}input_t;

//prototipos de funciones
void game_init(game_t * p);
void vaus_update(game_t * p, float time, int move);
void move_bola(game_t* g, float time);
int bola_choque_paredes(game_t* p);
void bola_choque_vaus(game_t* g);
void game_step(game_t* p, float time, int move, int pause, int reset);
void vaus_set_x(game_t* g, float x);
void bola_choque_bricks(game_t* g);
void ball_set_dir(bola_t* b, float ang); 
void ball_restart(game_t* g, int update_speed);
void bricks_init(game_t* g);
void bricks_reset_level(game_t* g);
int ball_hit(game_t* g, float rx, float ry, float rw, float rh, int is_vaus);

#endif 