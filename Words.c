#include <unistd.h>    
#include "disdrv.h"
#include "Words.h"
#include "joydrv.h"

//-------VIDAS----------

//centrado vertical del bloque de texto
int top_y(void){
    int h = (DISP_MAX_Y - DISP_MIN) + 1;
    int off = (h - GLYPH_H) / 2;
    if (off < 0) {
        off = 0;
    } 
    return DISP_MIN + off;
}

//para "vidas" y el espacio
static unsigned char CH_SPC[GLYPH_W] = { COL(0,0,0,0,0), COL(0,0,0,0,0), COL(0,0,0,0,0) };
static unsigned char CH_V  [GLYPH_W] = { COL(1,1,1,0,0), COL(0,0,0,1,1), COL(1,1,1,0,0) };
static unsigned char CH_I  [GLYPH_W] = { COL(0,0,0,0,1), COL(1,1,1,1,1), COL(0,0,0,0,1) };
static unsigned char CH_D  [GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(0,1,1,1,0) };
static unsigned char CH_A  [GLYPH_W] = { COL(1,1,1,0,1), COL(0,0,0,0,1), COL(1,1,1,0,1) };
static unsigned char CH_S  [GLYPH_W] = { COL(1,1,0,0,1), COL(1,1,1,1,1), COL(1,0,0,1,1) };

//digitos del 0 al 3 para las vidas
static unsigned char D0[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,1,1,1,1) };
static unsigned char D1[GLYPH_W] = { COL(0,0,0,0,1), COL(0,0,0,0,1), COL(0,0,0,0,1) };
static unsigned char D2[GLYPH_W] = { COL(1,0,0,1,1), COL(1,1,1,1,0), COL(1,1,0,0,0) };
static unsigned char D3[GLYPH_W] = { COL(1,0,0,1,1), COL(1,1,1,1,0), COL(1,0,0,1,1) };

unsigned char* glyph_for_char_vidas(char ch){
    if (ch == 'V' || ch == 'v') return CH_V;
    else if (ch == 'I' || ch == 'i') return CH_I;
    else if (ch == 'D' || ch == 'd') return CH_D;
    else if (ch == 'A' || ch == 'a') return CH_A;
    else if (ch == 'S' || ch == 's') return CH_S;
    else return CH_SPC;
}

//dibujo columna del glyph
void build_col(int X, int topY, unsigned char colbits){
    for (int row = 0; row < GLYPH_H; row++){
        if ((colbits >> row) & 1) {
            led_on(X, topY + row);
        }
    }
}

//scrollea texto vida
void scroll_text_vidas(const char* s, int step, int pad_cols){
    int y0 = top_y();
    int x_off = DISP_MAX_X + 1;

    for (int i = 0; s[i] != '\0'; i++){
        unsigned char* G = glyph_for_char_vidas(s[i]);
        for (int gx = 0; gx < GLYPH_W; gx++){
            disp_clear();
            int X = x_off;
            if (X >= DISP_MIN && X <= DISP_MAX_X) build_col(X, y0, G[gx]);
            disp_update();
            usleep(step * 1000);
            x_off--;
        }
        for (int pc = 0; pc < pad_cols; pc++){
            disp_clear();
            disp_update();
            usleep(step * 1000);
            x_off--;
        }
    }
    disp_clear();
    disp_update();
}

//scrolleo digitos de vidas
void scroll_digit(int d, int step, int pad_cols){
    if (d < 0) d = 0; else if (d > 3) d = 3;

    const unsigned char* G;
    switch(d){
        case 0: G = D0; break;
        case 1: G = D1; break;
        case 2: G = D2; break;
        default: G = D3; break;
    }

    int y0 = top_y();
    int x_off = DISP_MAX_X + 1;

    for (int gx = 0; gx < GLYPH_W; gx++){
        disp_clear();
        int X = x_off;
        if (X >= DISP_MIN && X <= DISP_MAX_X) build_col(X, y0, G[gx]);
        disp_update();
        usleep(step * 1000);
        x_off--;
    }
    for (int pc = 0; pc < pad_cols; pc++){
        disp_clear();
        disp_update();
        usleep(step * 1000);
        x_off--;
    }
    disp_clear();
    disp_update();
}

void show_life_lost(int lives_left){
    if (lives_left < 0) lives_left = 0;
    scroll_text_vidas(" VIDAS ", STEP_MS, COL_PAD);
    scroll_digit(lives_left, STEP_MS, COL_PAD);
}

//-------LETRAS GENERALES---------
static unsigned char CH_G[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,1,0,1), COL(1,1,1,0,1) };
static unsigned char CH_M[GLYPH_W] = { COL(1,1,1,0,1), COL(0,0,0,0,1), COL(1,1,1,0,1) };
static unsigned char CH_E[GLYPH_W] = { COL(1,1,1,1,1), COL(1,1,1,1,1), COL(1,1,0,0,0) };
static unsigned char CH_O[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,1,1,1,1) };
static unsigned char CH_R[GLYPH_W] = { COL(1,1,1,1,1), COL(0,1,1,0,1), COL(1,0,1,1,1) };
static unsigned char CH_T[GLYPH_W] = { COL(1,0,0,0,0), COL(1,1,1,1,1), COL(1,0,0,0,0) };
static unsigned char CH_N[GLYPH_W] = { COL(1,1,1,0,1), COL(0,0,1,0,1), COL(1,1,1,0,1) };
static unsigned char CH_C[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,0,0,0,0) };

unsigned char* glyph_for_char_general(char ch){
    if (ch == 'A' || ch == 'a') return CH_A;
    else if (ch == 'C' || ch == 'c') return CH_C;
    else if (ch == 'E' || ch == 'e') return CH_E;
    else if (ch == 'G' || ch == 'g') return CH_G;
    else if (ch == 'M' || ch == 'm') return CH_M;
    else if (ch == 'N' || ch == 'n') return CH_N;
    else if (ch == 'O' || ch == 'o') return CH_O;
    else if (ch == 'R' || ch == 'r') return CH_R;
    else if (ch == 'S' || ch == 's') return CH_S;
    else if (ch == 'T' || ch == 't') return CH_T;
    else if (ch == 'V' || ch == 'v') return CH_V;
    else return CH_SPC;
}

//scroll general
void scroll_text(const char* s, int step, int pad_cols){
    int y0 = top_y();
    int x_off = DISP_MAX_X + 1;

    for (int i = 0; s[i] != '\0'; i++){
        unsigned char* G = glyph_for_char_general(s[i]);
        for (int gx = 0; gx < GLYPH_W; gx++){
            disp_clear();
            int X = x_off;
            if (X >= DISP_MIN && X <= DISP_MAX_X) build_col(X, y0, G[gx]);
            disp_update();
            usleep(step * 1000);
            x_off--;
        }
        for (int pc = 0; pc < pad_cols; pc++){
            disp_clear();
            disp_update();
            usleep(step * 1000);
            x_off--;
        }
    }
    disp_clear();
    disp_update();
}

//---PAUSA ESTÁTICO---
void show_pause(void) {
    const char* s = "PAUSA";
    int y0 = top_y();
    int x_off = (DISP_MAX_X - (GLYPH_W * 5 + COL_PAD * 4)) / 2;
    if (x_off < DISP_MIN) x_off = DISP_MIN;

    for (int i = 0; s[i] != '\0'; i++){
        unsigned char* G = glyph_for_char_general(s[i]);
        for (int gx = 0; gx < GLYPH_W; gx++){
            int X = x_off + i * (GLYPH_W + COL_PAD) + gx;
            if (X >= DISP_MIN && X <= DISP_MAX_X) build_col(X, y0, G[gx]);
        }
    }

    disp_update();
    usleep(1000000);
    disp_clear();
    disp_update();
}

//---JOY---
void tap_button(void){
    int was_down = 0;
    while (1){
        joyinfo_t j = joy_read();
        int pressed = (j.sw != J_NOPRESS) ? 1 : 0;
        if (!was_down && pressed) was_down = 1;
        else if (was_down && !pressed) break;
        usleep(10 * 1000);
    }
}

//---DIGITOS PUNTAJE---
static unsigned char D4[GLYPH_W] = { COL(1,1,1,0,1), COL(0,0,0,0,1), COL(0,0,0,0,1) };
static unsigned char D5[GLYPH_W] = { COL(1,1,0,0,0), COL(1,1,1,1,0), COL(1,0,0,1,1) };
static unsigned char D6[GLYPH_W] = { COL(1,1,1,1,1), COL(1,1,1,1,0), COL(1,0,0,1,1) };
static unsigned char D7[GLYPH_W] = { COL(1,0,0,0,0), COL(1,0,0,1,1), COL(1,1,1,0,0) };
static unsigned char D8[GLYPH_W] = { COL(1,1,1,1,1), COL(1,1,1,1,0), COL(1,1,1,1,1) };
static unsigned char D9[GLYPH_W] = { COL(1,0,0,1,1), COL(1,1,1,1,1), COL(1,1,1,1,1) };

unsigned char* digit_glyph(int d){
    switch(d){
        case 0: return D0;
        case 1: return D1;
        case 2: return D2;
        case 3: return D3;
        case 4: return D4;
        case 5: return D5;
        case 6: return D6;
        case 7: return D7;
        case 8: return D8;
        case 9: return D9;
        default: return D0;
    }
}

//scroll numeros
void scroll_number(int value, int step, int pad_cols){
    char buf[16];
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
    } else {
        char tmp[16];
        int tlen = 0;
        int v = value;
        while (v > 0){
            tmp[tlen++] = '0' + (v % 10);
            v /= 10;
        }
        int n = 0;
        while (tlen > 0) buf[n++] = tmp[--tlen];
        buf[n] = '\0';
    }

    int y0 = top_y();
    int x_off = DISP_MAX_X + 1;

    for (int i = 0; buf[i] != '\0'; i++){
        int d = buf[i] - '0';
        if (d < 0 || d > 9) d = 0;
        unsigned char* G = digit_glyph(d);

        for (int gx = 0; gx < GLYPH_W; gx++){
            disp_clear();
            int X = x_off;
            if (X >= DISP_MIN && X <= DISP_MAX_X) build_col(X, y0, G[gx]);
            disp_update();
            usleep(step * 1000);
            x_off--;
        }

        for (int pc = 0; pc < pad_cols; pc++){
            disp_clear();
            disp_update();
            usleep(step * 1000);
            x_off--;
        }
    }
    disp_clear();
    disp_update();
}

//---MENUS---
void show_menu(void){
    scroll_text(" START ", STEP_MS, COL_PAD);
    tap_button();
}

void show_game_over(void){
    scroll_text(" GAME OVER ", 120, 1);
    tap_button();
    show_menu();
}

void show_win(void){
    scroll_text(" GANASTE ", 120, 1);
    tap_button();
    show_menu();
}

void show_score(int score){
    scroll_text(" SCORE ", 100, 1);
    scroll_number(score, 100, 1);
    tap_button();
    show_menu();
}