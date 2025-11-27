#include <unistd.h>    
#include "disdrv.h"
#include "Words.h"
#include "joydrv.h"

//-------VIDAS----------

//centrado vertical del bloque de text
int top_y(void){
    int h = (DISP_MAX_Y - DISP_MIN) + 1;
    int off = (h - GLYPH_H) / 2;
    if (off < 0) {
        off = 0;
    } 
    return DISP_MIN + off;
}

//para "vidas" y el esspacio
static unsigned char CH_SPC[GLYPH_W] = { COL(0,0,0,0,0), COL(0,0,0,0,0), COL(0,0,0,0,0) };
static unsigned char CH_V[GLYPH_W]  = { COL(1,1,1,0,0), COL(0,0,0,1,1), COL(1,1,1,0,0) };
static unsigned char CH_I[GLYPH_W]  = { COL(0,0,0,0,1), COL(1,1,1,1,1), COL(0,0,0,0,1) };
static unsigned char CH_D[GLYPH_W]  = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(0,1,1,1,0) };
static unsigned char CH_A[GLYPH_W]  = { COL(1,1,1,0,1), COL(0,0,0,0,1), COL(1,1,1,0,1) };
static unsigned char CH_S[GLYPH_W]  = { COL(1,1,0,0,1), COL(1,1,1,1,1), COL(1,0,0,1,1) };

//digitos del 0 al 3 para las vidas
static unsigned char D0[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,1,1,1,1) };
static unsigned char D1[GLYPH_W] = { COL(0,0,0,0,1), COL(0,0,0,0,1), COL(0,0,0,0,1) };
static unsigned char D2[GLYPH_W] = { COL(1,0,0,1,1), COL(1,1,1,1,0), COL(1,1,0,0,0) };
static unsigned char D3[GLYPH_W] = { COL(1,0,0,1,1), COL(1,1,1,1,0), COL(1,0,0,1,1) };

unsigned char* glyph_for_char_vidas(char ch){
    switch(ch){
        case 'V': case 'v': return CH_V;
        case 'I': case 'i': return CH_I;
        case 'D': case 'd': return CH_D;
        case 'A': case 'a': return CH_A;
        case 'S': case 's': return CH_S;
        case ' ': return CH_SPC;
        default: return CH_SPC;
    }
}

//dibujo columna del glyph
void build_col(int X, int topY, unsigned char colbits){
    for(int row=0; row<GLYPH_H; row++){
        if ((colbits >> row) & 1){
            led_on(X, topY + row);
        }
    }
}

//------ NUEVO: scroll con buffer de pantalla ------
void scroll_text_vidas(const char* s, int step, int pad_cols){
    int y0 = top_y();
    int x_off = DISP_MAX_X + 1;

    //Armo buffer de columnas de todo el texto
    unsigned char text_cols[256]; //máx 256 columnas
    int n_cols = 0;
    for(int i=0; s[i]!='\0'; i++){
        unsigned char* G = glyph_for_char_vidas(s[i]);
        for(int gx=0; gx<GLYPH_W; gx++){
            text_cols[n_cols++] = G[gx];
        }
        for(int pc=0; pc<pad_cols; pc++){
            text_cols[n_cols++] = CH_SPC[0];
        }
    }

    //scroll: desplazo columna por columna
    for(int off=0; off<n_cols + DISP_MAX_X + 1; off++){
        disp_clear();
        for(int c=0; c<n_cols; c++){
            int X = DISP_MAX_X - off + c;
            if(X >= DISP_MIN && X <= DISP_MAX_X){
                build_col(X, y0, text_cols[c]);
            }
        }
        disp_update();
        usleep(step * 1000);
    }
}

//scroll dígito de vidas (igual idea)
void scroll_digit(int d, int step, int pad_cols){
    if(d<0) d=0; else if(d>3) d=3;

    unsigned char* G = (d==0)?D0:(d==1)?D1:(d==2)?D2:D3;

    unsigned char text_cols[GLYPH_W + COL_PAD];
    for(int gx=0; gx<GLYPH_W; gx++) text_cols[gx] = G[gx];
    for(int pc=0; pc<pad_cols; pc++) text_cols[GLYPH_W + pc] = CH_SPC[0];

    int y0 = top_y();
    for(int off=0; off<GLYPH_W + pad_cols + DISP_MAX_X + 1; off++){
        disp_clear();
        for(int c=0; c<GLYPH_W + pad_cols; c++){
            int X = DISP_MAX_X - off + c;
            if(X >= DISP_MIN && X <= DISP_MAX_X){
                build_col(X, y0, text_cols[c]);
            }
        }
        disp_update();
        usleep(step * 1000);
    }
}

void show_life_lost(int lives_left){
    if(lives_left < 0) lives_left = 0;
    scroll_text_vidas(" VIDAS ", STEP_MS, COL_PAD);
    scroll_digit(lives_left, STEP_MS, COL_PAD);
}

//-------GENERICO---------
//letras necesarias
static unsigned char CH_G[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,1,0,1), COL(1,1,1,0,1) };
static unsigned char CH_M[GLYPH_W] = { COL(1,1,1,0,1), COL(0,0,0,0,1), COL(1,1,1,0,1) };
static unsigned char CH_E[GLYPH_W] = { COL(1,1,1,1,1), COL(1,1,1,1,1), COL(1,1,0,0,0) };
static unsigned char CH_O[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,1,1,1,1) };
static unsigned char CH_R[GLYPH_W] = { COL(1,1,1,1,1), COL(0,1,1,0,1), COL(1,0,1,1,1) };
static unsigned char CH_T[GLYPH_W] = { COL(1,0,0,0,0), COL(1,1,1,1,1), COL(1,0,0,0,0) };
static unsigned char CH_N[GLYPH_W] = { COL(1,1,1,0,1), COL(0,0,1,0,1), COL(1,1,1,0,1) };
static unsigned char CH_C[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,0,0,0,0) };

unsigned char* glyph_for_char_general(char ch){
    switch(ch){
        case 'A': case 'a': return CH_A;
        case 'C': case 'c': return CH_C;
        case 'E': case 'e': return CH_E;
        case 'G': case 'g': return CH_G;
        case 'M': case 'm': return CH_M;
        case 'N': case 'n': return CH_N;
        case 'O': case 'o': return CH_O;
        case 'R': case 'r': return CH_R;
        case 'S': case 's': return CH_S;
        case 'T': case 't': return CH_T;
        case 'V': case 'v': return CH_V;
        case ' ': return CH_SPC;
        default: return CH_SPC;
    }
}

void scroll_text(const char* s, int step, int pad_cols){
    int y0 = top_y();
    int x_off = DISP_MAX_X + 1;

    unsigned char text_cols[256];
    int n_cols=0;

    for(int i=0; s[i]!='\0'; i++){
        unsigned char* G = glyph_for_char_general(s[i]);
        for(int gx=0; gx<GLYPH_W; gx++) text_cols[n_cols++] = G[gx];
        for(int pc=0; pc<pad_cols; pc++) text_cols[n_cols++] = CH_SPC[0];
    }

    for(int off=0; off<n_cols + DISP_MAX_X + 1; off++){
        disp_clear();
        for(int c=0; c<n_cols; c++){
            int X = DISP_MAX_X - off + c;
            if(X >= DISP_MIN && X <= DISP_MAX_X){
                build_col(X, y0, text_cols[c]);
            }
        }
        disp_update();
        usleep(step * 1000);
    }
}

//----BOTÓN----
void tap_button(void){
    int was_down = 0;
    int loop = 1;
    while(loop){
        joyinfo_t j = joy_read();
        int pressed = (j.sw != J_NOPRESS) ? 1 : 0;
        if(!was_down && pressed) was_down = 1;
        else if(was_down && !pressed) loop = 0;
        usleep(10 * 1000);
    }
}

//-----DIGITOS PARA SCORE-----
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
        default: return D9;
    }
}

void scroll_number(int value, int step, int pad_cols){
    char buf[16];
    if(value == 0){
        buf[0]='0'; buf[1]='\0';
    } else {
        char tmp[16]; int tlen=0, v=value;
        while(v>0){
            tmp[tlen++] = '0' + (v%10);
            v/=10;
        }
        int n=0;
        while(tlen>0){
            buf[n++] = tmp[--tlen];
        }
        buf[n]='\0';
    }

    int y0=top_y();
    unsigned char text_cols[256]; int n_cols=0;

    for(int i=0; buf[i]!='\0'; i++){
        unsigned char* G = digit_glyph(buf[i]-'0');
        for(int gx=0; gx<GLYPH_W; gx++) text_cols[n_cols++] = G[gx];
        for(int pc=0; pc<pad_cols; pc++) text_cols[n_cols++] = CH_SPC[0];
    }

    for(int off=0; off<n_cols + DISP_MAX_X + 1; off++){
        disp_clear();
        for(int c=0; c<n_cols; c++){
            int X = DISP_MAX_X - off + c;
            if(X >= DISP_MIN && X <= DISP_MAX_X){
                build_col(X, y0, text_cols[c]);
            }
        }
        disp_update();
        usleep(step * 1000);
    }
}

//----MENÚ----
void show_menu(void){ scroll_text(" START ", STEP_MS, COL_PAD); tap_button(); }
void show_game_over(void){ scroll_text(" GAME OVER ", 120, 1); tap_button(); show_menu(); }
void show_win(void){ scroll_text(" GANASTE ", 120, 1); tap_button(); show_menu(); }
void show_score(int score){ scroll_text(" SCORE ", 100, 1); scroll_number(score, 100, 1); tap_button(); show_menu(); }