#include "Allegro.h"
#include "Backend.h"
#include "Stage.h"
#include <stdlib.h>
#include <stdio.h>

//bitmap
ALLEGRO_BITMAP* borde_bitmap(int w, int h, ALLEGRO_COLOR fill, ALLEGRO_COLOR borde)
{
    ALLEGRO_BITMAP* bmp = al_create_bitmap(w, h); //reservo bitmap del tamaño pedido 

    //valido 
    if(bmp==NULL){
        return NULL;
    }

    ALLEGRO_BITMAP* back = al_get_target_bitmap(); //guardo donde esta dibujando 
    al_set_target_bitmap(bmp); //dibujo sobre bmp

    al_clear_to_color(fill); //pinto todo del color de relleno 
    al_draw_rectangle(0.5, 0.5, w-0.5, h-0.5, borde, 1.0); //dibujo rectangulo de borde 

    al_set_target_bitmap(back); //restauro lo que estaba dibujando antes 
    return bmp;
}

//colores de los bloques 
ALLEGRO_COLOR color_for_brick(int t)
{
    if(t == 1){
        return al_map_rgb(200, 0, 0); //rojo
    }
    if(t == 2){
        return al_map_rgb( 0, 130, 255); //celeste
    }
    if(t == 3){
        return al_map_rgb(220, 180, 0); //amarillo
    }
    if(t == 4){
        return al_map_rgb(255, 120, 200); //rosa
    }

    return al_map_rgb(0, 200, 60); //verde
}

//bitmap para la pelota
ALLEGRO_BITMAP* ball_bitmap(int r, ALLEGRO_COLOR fill, ALLEGRO_COLOR border){
    int d = 2*r; //diametro de la pelota 
    ALLEGRO_BITMAP* bmp = al_create_bitmap(d, d); //creo bitmap para dibujar la pelota 
    if(bmp==NULL){
        return NULL;
    }

    ALLEGRO_BITMAP* back = al_get_target_bitmap(); //guardo bitmap actual donde se esta dibujando 
    al_set_target_bitmap(bmp); //dibujo

    al_clear_to_color(al_map_rgba(0,0,0,0)); //para que afuera del circulo no tenga el color
    al_draw_filled_circle(r, r, r, fill); //dibuja circulo relleno
    al_draw_circle(r, r, r - 0.5, border, 1.0); //borde del circulo

    al_set_target_bitmap(back); //vuelvo a dejarlo como el orginal
    return bmp;
}

//bitmap para el vaus
ALLEGRO_BITMAP* paddle_bitmap(int w, int h){
    
    ALLEGRO_BITMAP* bmp = al_create_bitmap(w, h); //creo bitmap para el vaus 
    if(bmp==NULL){
        return NULL;
    }

    ALLEGRO_BITMAP* back = al_get_target_bitmap();
    al_set_target_bitmap(bmp);

    al_clear_to_color(al_map_rgba(0,0,0,0));  

    //cuerpo
    al_draw_filled_rounded_rectangle(1, 1, w-1, h-1, 4, 4, al_map_rgb(180,180,180)); //hago un rectangulo redondeado relleno
    al_draw_rounded_rectangle(1.5, 1.5, w-1.5, h-1.5, 4, 4,al_map_rgb(90,90,90), 1.0); //el borde 

    //detalles de los bordes 
    int bord = h; 
    al_draw_filled_rectangle(0, 0, bord, h, al_map_rgb(230,100,30));
    al_draw_filled_rectangle(w-bord, 0, w, h, al_map_rgb(230,100,30));

    al_set_target_bitmap(back);
    return bmp;
}

//incializo todo 
int allegro_init(allegro_t* p,  game_t* g, int pixel_1, int pixel_2) {
    p->display= NULL;
    p->queue= NULL;
    p->timer= NULL;
    p->pixel_1=pixel_1;
    p->pixel_2=pixel_2;
    p->sx=pixel_1/ANCHO;
    p->sy=pixel_2/ALTO;

    //valido los pixeles 
    if (p->sx <= 0 || p->sy <= 0){
        printf("ventana muy chica");
        return 0;
    }

    //valido y inicializo
    if (!al_init()) {
        fprintf(stderr, "failed to initialize allegro!\n");
        return 0;
    }
    if (!al_install_keyboard()) {
        fprintf(stderr, "falla instalando el teclado\n");
        return 0;
    }
    if (!al_install_mouse()) {
        fprintf(stderr, "falla instalando el mouse\n");
        return 0;
    }

    //creo timer
    p->timer= al_create_timer(1.0/FPS);
    if (!p->timer) { 
        fprintf(stderr, "falla creando timer\n"); 
        return 0;
    }

    //creo cola de eventos
    p->queue= al_create_event_queue();
    if (!p->queue) { 
        fprintf(stderr, "falla creando cola de eventos\n"); 
        al_destroy_timer(p->timer); //destruyo el timer porque hubo un error 
        return 0;
    }

    //creo display
    p->display= al_create_display(p->pixel_1, p->pixel_2);
    if (!p->display) { 
        fprintf(stderr, "falla creando display\n"); 
        al_destroy_timer(p->timer);
        al_destroy_event_queue(p->queue);
        return 0;
    }

    //bitmaps
    al_init_primitives_addon();              

    //fondo 
    ALLEGRO_COLOR bg_fill= al_map_rgb(10, 10, 30); //relleno
    ALLEGRO_COLOR bg_border = al_map_rgb(20, 20, 50); //borde 
    p->bmp_background = borde_bitmap(ANCHO * p->sx, ALTO * p->sy, bg_fill, bg_border);
    if (!p->bmp_background){
        fprintf(stderr, "fallo creando fondo\n");
        al_destroy_display(p->display);
        al_destroy_event_queue(p->queue);
        al_destroy_timer(p->timer);
        return 0;
    }

    //bloques
    for (int t = 1; t <= 5; t++){ // recorro los tipos de bloque 
    ALLEGRO_COLOR fill= color_for_brick(t); //color de relleno
        ALLEGRO_COLOR border = al_map_rgb(60,60,60); //borde
        p->bmp_brick[t] = borde_bitmap(BR_W - 2, BR_H - 2, fill, border);
        if(!p->bmp_brick[t]){ 
            fprintf(stderr, "sin memoria ladrillo\n"); 
            for(int k = 1; k < t; ++k){ 
                if (p->bmp_brick[k]){
                    al_destroy_bitmap(p->bmp_brick[k]); 
                }
            }
            al_destroy_bitmap(p->bmp_background);
            al_destroy_display(p->display);
            al_destroy_event_queue(p->queue);
            al_destroy_timer(p->timer);
            return 0;
        }
    }

    //bola
    int rpx = (g->ball.radio * p->sx); // radio de la bola en píxeles
    p->bmp_ball = ball_bitmap(rpx, al_map_rgb(255,120,200), al_map_rgb(180,60,120)); //bola rosa borde gris 
    if(!p->bmp_ball){ 
        fprintf(stderr, "sin memoria bola\n"); 
        al_destroy_bitmap(p->bmp_paddle);
        for(int k = 1; k <= 5; k++){ 
            if(p->bmp_brick[k]){
                al_destroy_bitmap(p->bmp_brick[k]); 
            }
        }
        al_destroy_bitmap(p->bmp_background);
        al_destroy_display(p->display);
        al_destroy_event_queue(p->queue);
        al_destroy_timer(p->timer);
        return 0; 
    }

    //vaus
    int paddle_w = (2*g->vaus.half + 1) * p->sx; //ancho del vaus en píxeles
    int paddle_h = 12; //alto del vaus en píxeles
    p->bmp_paddle = paddle_bitmap(paddle_w, paddle_h); //creo el bitmap 
    if(!p->bmp_paddle){ 
        fprintf(stderr, "sin memoria vaus\n"); 
        for(int k = 1; k <= 5; k++){ 
            if(p->bmp_brick[k]){
                al_destroy_bitmap(p->bmp_brick[k]); 
            }
        }
        al_destroy_bitmap(p->bmp_background);
        al_destroy_display(p->display);
        al_destroy_event_queue(p->queue);
        al_destroy_timer(p->timer);
        return 0; 
    }

    //fuente
    al_init_font_addon();
    al_init_ttf_addon();

    p->font_big= al_load_ttf_font(FONT_ASSET_PATH, 64, 0);  //títulos y countdown
    p->font_small= al_load_ttf_font(FONT_ASSET_PATH, 24, 0);  //info

    if (!p->font_big || !p->font_small) {
        if (p->font_big)   { 
            al_destroy_font(p->font_big);   
        }
        if (p->font_small) { 
            al_destroy_font(p->font_small); 
        }

        p->font_big   = al_create_builtin_font();
        p->font_small = al_create_builtin_font();

        if (!p->font_big || !p->font_small) {
            fprintf(stderr, "Error: no pude crear fuentes (ni TTF ni builtin).\n");
          
            if (p->display){
                al_destroy_display(p->display);
            }
            if (p->queue){
                al_destroy_event_queue(p->queue);
            }
            if (p->timer){
                al_destroy_timer(p->timer);
            }
            return 0;
        }
    }

    //registro en la cola
    al_register_event_source(p->queue, al_get_display_event_source(p->display));
    al_register_event_source(p->queue, al_get_timer_event_source(p->timer));
    al_register_event_source(p->queue, al_get_keyboard_event_source());
    al_register_event_source(p->queue, al_get_mouse_event_source());

    // Audio initialization
    if (!al_install_audio()) {
        fprintf(stderr, "failed to initialize audio!\n");
    } else if (!al_init_acodec_addon()) {
        fprintf(stderr, "failed to initialize audio codecs!\n");
    } else {
        if (!al_reserve_samples(1)) {
            fprintf(stderr, "failed to reserve samples!\n");
        }
        
        p->music = al_load_audio_stream(MUSIC_ASSET_PATH, 4, 2048);
        if (!p->music) {
            fprintf(stderr, "Audio clip sample not loaded! Path: %s\n", MUSIC_ASSET_PATH);
        } else {
            al_set_audio_stream_playmode(p->music, ALLEGRO_PLAYMODE_LOOP);
            al_attach_audio_stream_to_mixer(p->music, al_get_default_mixer());
        }
    }

    al_start_timer(p->timer); //arranco el timer
    return 1; 
    
}

int redraw=0; //1 hay que dibujar, 0 no hay que dibujar

void allegro_read_input(allegro_t* p, game_t *g, int* move, int* pause, int* reset, int* quit){
    ALLEGRO_EVENT ev; 
    static int left_down  = 0;
    static int right_down = 0;
    
    if (move) {
        *move  = 0;
    }

    //saco eventos de la cola 
    while (al_get_next_event(p->queue, &ev)){ 

        //timer
        if (ev.type == ALLEGRO_EVENT_TIMER){
            redraw=1;
        }
        //tecla presionada
        else if(ev.type == ALLEGRO_EVENT_KEY_DOWN){
            int k= ev.keyboard.keycode; //veo que tecla se apreto

            if (g->stage == STAGE_TITLE){
                //para el menu 
                if (k == ALLEGRO_KEY_ENTER || k == ALLEGRO_KEY_SPACE){
                    stage_request_start(g); 
                }
            }
            else if (k == ALLEGRO_KEY_LEFT){
                //si la tecla es mover a la izquierda
                left_down=1;
            }
            else if (k == ALLEGRO_KEY_RIGHT){
                //si la tecla es mover a la derecha 
                right_down=1; //mover a la derecha 
            }
            else if (k == ALLEGRO_KEY_SPACE){
                //pausar (con el espacio)
                *pause=(!*pause);
            }
            else if (k == ALLEGRO_KEY_M){
                //para el reset (con la letra M)
                *reset=1;            
            }
            else if(k == ALLEGRO_KEY_ESCAPE){
                //salir del juego con esc 
                *quit=1;
            }
            

        }
        //para el boton del menu
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (g->stage == STAGE_TITLE && ev.mouse.button == 1) {
                float mx = ev.mouse.x;
                float my = ev.mouse.y;

                char* btn = "JUGAR";
                int wb = al_get_text_width(p->font_big, btn);
                int lhb = al_get_font_line_height(p->font_big);

                float cx = p->pixel_1 * 0.5;
                float cy = p->pixel_2 * 0.5;

                int padx= 30;
                int pady = 12;
                int bw = wb + 2*padx;
                int bh = lhb + 2*pady;
                int bx = cx - bw/2;
                int by = cy + 30;

                if (mx >= bx && mx <= bx + bw && my >= by && my <= by + bh) {
                    stage_request_start(g);
                }
            }
        }
        //tecla fue soltada 
        else if(ev.type == ALLEGRO_EVENT_KEY_UP){
            int k= ev.keyboard.keycode;
            //veo si era la que hacia que se mueva 
            if (k == ALLEGRO_KEY_LEFT) {
                left_down=0;
            }
            if (k == ALLEGRO_KEY_RIGHT) {
                right_down=0;
            }
        }
        //ventana cerrada
        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
            *quit=1;
        }
        //mouse
        else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if (g->stage == STAGE_PLAY && !g->paused){
                float x_celda = ev.mouse.x / p->sx;
                vaus_set_x(g, x_celda);
            }        
        }     

    }
    if (move) {
        int m = 0;
        if (left_down && !right_down) {
            m = -1; //izquierda
        } else if (right_down && !left_down) {
            m = 1;  //derecha
        } else {
            m = 0; //ninguna o ambas
        }
        *move = m;
    }
}

int allegro_should_draw(allegro_t* p)
{
    // if ((redraw==1) && (al_is_event_queue_empty(p->queue))) {
    //     redraw = 0;   
    //     return 1; //update y dibuja
    // }
    // return 0; //todavua no dibuja

    int retval = redraw;
    redraw = 0;
    return retval;
}

//convierto de celda a pixel
int celda2pixel_x(allegro_t* g, float x){ 
    return (x * g->sx); 
}
int celda2pixel_y(allegro_t* g, float y){ 
    return (y * g->sy); 
}

void allegro_draw(allegro_t* p, game_t* g){

    ALLEGRO_BITMAP* back = al_get_backbuffer(p->display); //backbuffer
    al_set_target_bitmap(back);
    al_clear_to_color(al_map_rgb(32, 197, 195)); //pongo el fondo color azul

    //borde gris 
    ALLEGRO_COLOR gris= al_map_rgb(180,180,180); //para el borde gris 
    al_draw_filled_rectangle(0, 0, ANCHO*p->sx, 4, gris); //arriba
    al_draw_filled_rectangle(0, 0, 4, ALTO*p->sy, gris); //izquirda
    al_draw_filled_rectangle(ANCHO*p->sx - 4, 0, ANCHO*p->sx, ALTO*p->sy, gris); //derecha 


    //ladrillos 
    for (int r = 0; r < BR_ROWS; r++){ //recorro filas y columnas de la matriz de ladrillos 
        for(int c = 0; c < BR_COLS; c++){
            brick_t* b = &g->bricks[r][c];
            if (b->alive != 0){ //si esta vivo
                int t = b->alive; //para ver el tipo de ladrillo entre 1 y 5
                if(t < 1){
                    t = 1;
                } 
                if(t > 5){
                    t = 5;
                }
                al_draw_bitmap(p->bmp_brick[t], b->x, b->y, 0);
        
            }
        }
    }

    //vaus
    float left_px = (g->vaus.x - g->vaus.half) * p->sx; //borde izquierdo en pixeles 
    float y_px    =  g->vaus.y * p->sy;

    float dst_w = (2.0f * g->vaus.half) * p->sx; //ancho que usa la lógica del juego
    float dst_h = al_get_bitmap_height(p->bmp_paddle); //alto del PNG

    y_px -= (dst_h - p->sy) * 0.5;

    al_draw_scaled_bitmap(p->bmp_paddle, 0, 0, al_get_bitmap_width(p->bmp_paddle), al_get_bitmap_height(p->bmp_paddle), left_px, y_px, dst_w, dst_h, 0);

    //bola 
    int rpx = al_get_bitmap_width(p->bmp_ball) / 2; //radio en píxeles
    float bx = g->ball.p.x * p->sx - rpx; //posición x en píxeles
    float by = g->ball.p.y * p->sy - rpx; //posición y en píxeles
    al_draw_bitmap(p->bmp_ball, bx, by, 0);

    //para que aparezcan vidas, puntaje, niveles
    ALLEGRO_COLOR hud = al_map_rgb(255,255,255);
    int margin = 8;  //píxeles desde la esquina
    al_draw_textf(p->font_small, hud, margin, margin, 0, "Vidas: %d   Puntos: %d   Nivel: %d", g->vidas, g->score, g->phase);

    //texto pausa 
    if (g->paused && g->stage == STAGE_PLAY) {
        char* txt = "PAUSA";

        //centro de la ventana 
        float cx = p->pixel_1 * 0.5;
        float cy = p->pixel_2 * 0.5;

        //fuente grande 
        int wt = al_get_text_width(p->font_big, txt);
        int ht = al_get_font_line_height(p->font_big);

        al_draw_text(p->font_big, al_map_rgb(255,255,255), cx - wt/2, cy - ht/2, 0, txt);
    }

    #ifdef USE_ALLEGRO
        stage_draw_overlay(p,g);
    #endif
        al_flip_display(); //muestro 

}

void allegro_shutdown(allegro_t* p){
    al_destroy_display(p->display);
    al_destroy_event_queue(p->queue);
    al_destroy_timer(p->timer);
    for (int t = 1; t <= 5; ++t){
        if(p->bmp_brick[t]){
            al_destroy_bitmap(p->bmp_brick[t]);
        }
    }
    if(p->bmp_ball){  
        al_destroy_bitmap(p->bmp_ball);
    }
    if(p->bmp_paddle){
        al_destroy_bitmap(p->bmp_paddle);
    }
    if (p->font_big){ 
        al_destroy_font(p->font_big);   
    }
    if (p->font_small){ 
        al_destroy_font(p->font_small);
    }
    if (p->music) {
        al_destroy_audio_stream(p->music);
    }
   
}
