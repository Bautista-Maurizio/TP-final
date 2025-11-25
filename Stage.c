#include "Stage.h"
#include <stdio.h>

void ball_hold_center(game_t* g){
    g->ball.p.x= ANCHO * 0.5;
    g->ball.p.y= ALTO  * 0.4;
    g->ball.vel.x = 0.0;
    g->ball.vel.y = 0.0;
}

void ball_baja(game_t* g){ 
    g->ball.vel.x = 0.0f;
    g->ball.vel.y = +g->ball.speed; //cae hacia abajo
}


void stage_init(game_t* g){
    g->game_over= 0;
    g->game_won= 0;
    g->stage= STAGE_COUNTDOWN; //cuenta regresiva 
    g->stage_tsec= 3.0;
    ball_hold_center(g); //bola quieta y centrada mientras esta el countdown 
    stage_show_title(g); 
}

void stage_show_title(game_t* g){
    g->stage= STAGE_TITLE;
    g->stage_tsec = 0.0; //sin timer 
    ball_hold_center(g); //bola quieta 
}

void stage_request_start(game_t* g){
    if (g->stage == STAGE_TITLE){
        g->start_requested= 1;
    }
}

void stage_set(game_t* g, stage_t s, float seconds){
    g->stage= s; //nuevo stage
    g->stage_tsec= seconds; //tiempo que va a durar el stage
    if (s == STAGE_PLAY) {
        g->vaus.speed = 420.0; 
    } 
    else {
        g->vaus.speed = 0.0;  
    }
    if (s == STAGE_COUNTDOWN || s == STAGE_LEVEL_UP || s == STAGE_GAME_OVER || s == STAGE_WIN){
        ball_hold_center(g); //bola queda quieta 
        g->vaus.x = ANCHO * 0.5; //vaus centrado
        g->vaus.speed = 0.0;   
    }
}

void stage_update(game_t* g, float dt){

    if (g->stage == STAGE_TITLE){
        if (g->start_requested){
            g->start_requested = 0;
            //arranca el juego
            stage_set(g, STAGE_COUNTDOWN, 3.0);
        }
        return; //espera en el menu 
    } 
    if (g->stage == STAGE_PLAY){
        return;
    }
    
    //voy descontando tiempo del estado 
    if (g->stage_tsec > 0.0){
        g->stage_tsec -= dt;
        if (g->stage_tsec < 0.0){
            g->stage_tsec = 0.0;
        }
    }

    //transiciones 
    if (g->stage == STAGE_LEVEL_UP && g->stage_tsec == 0.0){

        //termino el cartel que muestra el nivel y empieza el countdown
        stage_set(g, STAGE_COUNTDOWN, 3.0);
    }
    else if (g->stage == STAGE_COUNTDOWN && g->stage_tsec == 0.0){
        ball_baja(g);
        stage_set(g, STAGE_PLAY, 0.0);
    }
    //STAGE_GAME_OVER / STAGE_WIN: queda congelado
}

void stage_on_life_lost(game_t* g){
    if (g->game_over){
        return;
    }
    if (g->vidas > 0){
        g->vidas--; //resto vida 
    }
    if (g->vidas <= 0){
        g->vidas= 0; //se quedo sin vidas 
        g->game_over= 1;
        g->game_won= 0;
        stage_set(g, STAGE_GAME_OVER, 0.0); //cartel de game over 
    } 
    else {
        stage_set(g, STAGE_COUNTDOWN, 3.0); //si todavia hay vidas seguimos en el mismo nivel 
    }
}

void stage_on_bricks_cleared(game_t* g){
    if (g->game_over){
        return;
    }
    if (g->phase < 3){
        g->phase++; //pasamos al siguiente nivel 
        bricks_reset_level(g); //recontruye la pared del nivel
        ball_restart(g, 1);  
        stage_set(g, STAGE_LEVEL_UP, 1.0); //cartel que muestra en q nivel esta 
    } 
    else { //ya supero el nivel 3
        g->game_over= 1;
        g->game_won = 1;
        stage_set(g, STAGE_WIN, 0.0); //cartel de que gano 
    }
}

void stage_draw_overlay(allegro_t* ui, game_t* g){
    if (!ui->font_big){
        return;
    }
    float cx = ui->pixel_1 * 0.5;
    float cy = ui->pixel_2 * 0.5;

    if (g->stage == STAGE_TITLE) {

        //colores
        ALLEGRO_COLOR col_back= al_map_rgba(0,0,0,180); //panel negro translúcido
        ALLEGRO_COLOR col_text= al_map_rgb(200,200,200); //gris claro
        ALLEGRO_COLOR col_btn= al_map_rgba(10,30,70,230); //azul
        ALLEGRO_COLOR col_bord= al_map_rgb(255,255,255); //borde blanco

        //titulo
        char* title = "ARKANOID";
        int wt= al_get_text_width(ui->font_big, title);
        int lhb= al_get_font_line_height(ui->font_big);

        //instrucciones 
        char* lines[] = {"ENTER / ESPACIO: Jugar", "FLECHAS o MOUSE: Mover el vaus", "ESPACIO: Pausar / Reanudar",  "M: Volver al menu"};
        int nlines = sizeof(lines)/sizeof(lines[0]);
        int lhs = al_get_font_line_height(ui->font_small);
        int sep = 6;

        //ancho de instrucciones
        int wi = 0;
        for(int i = 0; i < nlines; ++i){
            int w = al_get_text_width(ui->font_small, lines[i]);
            if (w > wi) wi = w;
        }

        //boton
        char* btn = "JUGAR";
        int wb= al_get_text_width(ui->font_big, btn);
        int padX= 30;
        int padY = 12; //padding interno del botón
        int bw= wb + 2*padX; //ancho
        int bh= lhb + 2*padY; //alto 

        //panel 
        int maxw = wt;
        if (wi > maxw){
            maxw = wi;
        }
        if (bw > maxw){
            maxw = bw;
        }

        int pad = 24;         
        int content_h = lhb + 14 + nlines*lhs + (nlines-1)*sep + 18 + bh; 
        int panel_w= maxw + 2*pad;
        int panel_h= content_h + 2*pad;

        //fondo panel negro transucido
        al_draw_filled_rectangle(cx - panel_w / 2.0, cy - panel_h / 2.0, cx + panel_w / 2.0, cy + panel_h / 2.0, col_back);
        //titulo 
        al_draw_text(ui->font_big, col_bord, cx - wt / 2.0, cy - panel_h / 2.0 + pad, 0, title);

        //instrucciones
        float y = cy - panel_h / 2.0 + pad + lhb + 14.0;
        int i;
        for (i = 0; i < nlines; i++){
            int w = al_get_text_width(ui->font_small, lines[i]);
            al_draw_text(ui->font_small, col_text, cx - w / 2.0f, y, 0, lines[i]);
            y += lhs + sep;
        }

        //boton
        float bx0 = cx - bw / 2.0;
        float by0 = y + 18.0;
        float bx1 = bx0 + bw;
        float by1 = by0 + bh;
        al_draw_filled_rectangle(bx0, by0, bx1, by1, col_btn);
        al_draw_rectangle(bx0, by0, bx1, by1, col_bord, 2.0);
        al_draw_text(ui->font_big, col_bord, cx - wb / 2.0, by0 + padY, 0, btn);
    }
    else if (g->stage == STAGE_COUNTDOWN){
        //si esta en pausa muestro pausa y no el numero 
        if (g->paused){
            int lhb = al_get_font_line_height(ui->font_big);
            char* txt = "PAUSA";

            int wt = al_get_text_width(ui->font_big, txt);
            al_draw_text(ui->font_big, al_map_rgb(255,255,255), cx - wt/2, cy - lhb/2, 0, txt);
        } 
        else {
            //contdown sin fondo 
            int n = ceilf(g->stage_tsec); // 3..1
            if (n < 1){
                n = 1;
            }
            if (n > 3){
                n = 3;
            }
            char buf[8];
            snprintf(buf, sizeof(buf), "%d", n);
            int w = al_get_text_width(ui->font_big, buf);
            int h = al_get_font_line_height(ui->font_big);
            al_draw_text(ui->font_big, al_map_rgb(255,255,255), cx - w/2, cy - h/2, 0, buf);
        }
    }
    else if (g->stage == STAGE_LEVEL_UP){
        //cartel que dice el nivel 
        char txt[32];
        snprintf(txt, sizeof(txt), "Nivel %d", g->phase);

        int wt  = al_get_text_width(ui->font_big, txt);
        int lhb = al_get_font_line_height(ui->font_big);

        int pad = 24;
        int w = wt + 2*pad;
        int h = lhb + 2*pad;

        //panel
        al_draw_filled_rectangle(cx - w/2, cy - h/2, cx + w/2, cy + h/2, al_map_rgba(32,197,195,200));

        //texto centrado 
        al_draw_text(ui->font_big, al_map_rgb(255,255,255), cx - wt/2, cy - lhb/2, 0, txt);
    }
    else if (g->stage == STAGE_GAME_OVER || g->stage == STAGE_WIN){
        char* title;
        if (g->stage == STAGE_WIN) {
            title = "GANASTE!";
        } 
        else {
            title = "GAME OVER";
        }

        char info[128];
        snprintf(info, sizeof(info), "Puntaje: %d - M para volver al menu", g->score);

        int wt = al_get_text_width(ui->font_big, title);
        int wi = al_get_text_width(ui->font_small, info);
        int lhb = al_get_font_line_height(ui->font_big);
        int lhs = al_get_font_line_height(ui->font_small);

        //tamaño del contenido
        int pad = 24;
        int maxw=wt;
        if (wi > maxw) {
            maxw = wi;
        }
        int panel_w = maxw + 2*pad;
        int panel_h = (lhb + 10 + lhs) + 2*pad;

        al_draw_filled_rectangle(cx - panel_w/2.0, cy - panel_h/2.0, cx + panel_w/2.0, cy + panel_h/2.0, al_map_rgba(0, 0, 0, 210));

        // título e info
        al_draw_text(ui->font_big,   al_map_rgb(200,200,200), cx - wt/2.0,  cy - panel_h/2.0 + pad, 0, title);
        al_draw_text(ui->font_small, al_map_rgb(200,200,200), cx - wi/2.0,  cy - panel_h/2.0 + pad + lhb + 10, 0, info);
    }        
    
}