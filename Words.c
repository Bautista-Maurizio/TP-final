#include <unistd.h>
#include "disdrv.h"
#include "Words.h"
#include "joydrv.h"
#include <stdio.h> // Necesario para sprintf si lo usas, aunque usas manual

// Constantes
// Nota: La definición de los caracteres sigue la convención COL(b0, b1, b2, b3, b4)
// donde b0 es la fila superior y b4 es la fila inferior, gracias a la macro COL.

// centrado vertical del bloque de texto
int top_y(void){
    // Asumimos que DISP_MAX_Y y DISP_MIN están definidos en disdrv.h
    int h = (DISP_MAX_Y - DISP_MIN) + 1;
    int off = (h - GLYPH_H) / 2;
    if (off < 0) off = 0;
    return DISP_MIN + off; // Devuelve la coordenada Y de inicio
}

// para "vidas" y el espacio
// COL(b0,b1,b2,b3,b4) -> Fila Superior a Fila Inferior
static unsigned char CH_SPC[GLYPH_W] = { COL(0,0,0,0,0), COL(0,0,0,0,0), COL(0,0,0,0,0) };
static unsigned char CH_V[GLYPH_W]  = { COL(1,0,0,0,1), COL(1,0,0,0,1), COL(0,1,1,1,0) };
static unsigned char CH_I[GLYPH_W]  = { COL(1,1,1,1,1), COL(0,1,1,1,0), COL(1,1,1,1,1) }; // Ajuste: la I es una línea vertical
static unsigned char CH_D[GLYPH_W]  = { COL(1,1,1,1,0), COL(1,0,0,0,1), COL(1,1,1,1,0) }; // Ajuste: D
static unsigned char CH_A[GLYPH_W]  = { COL(0,1,1,1,0), COL(1,0,0,0,1), COL(1,1,1,1,1) };
static unsigned char CH_S[GLYPH_W]  = { COL(1,1,1,1,0), COL(1,0,1,0,1), COL(0,1,1,1,1) }; // Ajuste: S

// dígitos 0..3 para vidas
static unsigned char D0[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,1,1,1,1) }; // Ajuste: 0 (completo)
static unsigned char D1[GLYPH_W] = { COL(0,0,1,1,1), COL(0,0,0,1,1), COL(1,1,1,1,1) }; // Ajuste: 1 (fácil de leer)
static unsigned char D2[GLYPH_W] = { COL(1,0,1,1,1), COL(1,0,1,0,1), COL(1,1,1,0,1) }; // Ajuste: 2
static unsigned char D3[GLYPH_W] = { COL(1,0,1,0,1), COL(1,0,1,0,1), COL(0,1,1,1,0) }; // Ajuste: 3

// letras generales
static unsigned char CH_G[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,1,0,1), COL(1,1,1,1,1) }; // Ajuste: G
static unsigned char CH_M[GLYPH_W] = { COL(1,1,0,0,1), COL(1,0,1,0,1), COL(1,0,0,1,1) }; // Ajuste: M
static unsigned char CH_E[GLYPH_W] = { COL(1,1,1,1,1), COL(1,1,1,0,0), COL(1,1,1,1,1) };
static unsigned char CH_O[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,1,1,1,1) }; // Ajuste: O
static unsigned char CH_R[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,1,0,1), COL(0,1,1,1,0) }; // Ajuste: R
static unsigned char CH_T[GLYPH_W] = { COL(1,1,1,1,1), COL(0,0,1,0,0), COL(0,0,1,0,0) }; // Ajuste: T
static unsigned char CH_N[GLYPH_W] = { COL(1,1,0,0,1), COL(1,0,1,0,1), COL(1,0,0,1,1) }; // Ajuste: N
static unsigned char CH_C[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,0), COL(1,1,1,1,1) }; // Ajuste: C

// dígitos 4..9 (ajustes para consistencia)
static unsigned char D4[GLYPH_W] = { COL(1,1,1,1,1), COL(0,0,0,1,0), COL(0,0,0,1,0) };
static unsigned char D5[GLYPH_W] = { COL(1,1,1,1,1), COL(1,1,1,1,0), COL(0,0,0,1,1) };
static unsigned char D6[GLYPH_W] = { COL(1,1,1,1,1), COL(1,1,1,1,0), COL(1,1,1,1,1) };
static unsigned char D7[GLYPH_W] = { COL(1,0,0,0,0), COL(1,0,0,0,0), COL(1,1,1,1,1) };
static unsigned char D8[GLYPH_W] = { COL(1,1,1,1,1), COL(1,1,1,1,1), COL(1,1,1,1,1) };
static unsigned char D9[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,1,1,1,1) };

// ----------------- helpers -----------------

// ... (scroll_buffer y las funciones públicas se mantienen iguales) ...
static void scroll_buffer(const unsigned char *buf, int n_cols, int step){
    int y0 = top_y();
    int disp_width = DISP_MAX_X - DISP_MIN + 1;

    for(int off=0; off<n_cols+disp_width; off++){
        disp_clear();
        for(int c=0; c<n_cols; c++){
            int X = DISP_MAX_X - off + c;
            if(X>=DISP_MIN && X<=DISP_MAX_X){
                build_col(X, y0, buf[c]);
            }
        }
        disp_update();
        usleep(step*1000);
    }
    disp_clear();
    disp_update();
}

// ----------------- públicas -----------------

// ... (glyph_for_char_vidas, glyph_for_char_general, digit_glyph se mantienen) ...
unsigned char* glyph_for_char_vidas(char ch){
    switch(ch){
        case 'V': case 'v': return CH_V;
        case 'I': case 'i': return CH_I;
        case 'D': case 'd': return CH_D;
        case 'A': case 'a': return CH_A;
        case 'S': case 's': return CH_S;
        default: return CH_SPC;
    }
}

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
        default: return CH_SPC;
    }
}

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


// DIBUJO CORREGIDO: Soluciona la inversión vertical.
// La macro COL(b0...b4) mapea b0 (Superior) al Bit 4 (MSB) y b4 (Inferior) al Bit 0 (LSB).
// La función build_col debe iterar las filas (0 a 4) y buscar el bit correspondiente (4 a 0).
void build_col(int X, int topY, unsigned char colbits){
    for(int row = 0; row < GLYPH_H; row++){
        // 'row' va de 0 (Fila Superior) a 4 (Fila Inferior) en la pantalla.
        
        // Calculamos el índice del bit en 'colbits' que corresponde a esta 'row'.
        // Fila 0 de la pantalla debe tomar el Bit 4 (MSB) de 'colbits'.
        // Fila 4 de la pantalla debe tomar el Bit 0 (LSB) de 'colbits'.
        int bit_to_check = GLYPH_H - 1 - row; // Esto nos da: 4, 3, 2, 1, 0
        
        // Si el bit está encendido:
        if((colbits >> bit_to_check) & 1){
            // Encendemos el LED en la posición X y Y (topY + row).
            led_on(X, topY + row);
        }
    }
}

// ... (El resto de las funciones de scroll y menu se mantienen iguales) ...
void scroll_text_vidas(const char* s, int step, int pad_cols){
    unsigned char buf[256]; int n=0;
    for(int i=0;s[i]!='\0' && n<(int)sizeof(buf); i++){
        unsigned char *G = glyph_for_char_vidas(s[i]);
        for(int gx=0; gx<GLYPH_W && n<(int)sizeof(buf); gx++) buf[n++]=G[gx];
        for(int p=0;p<pad_cols && n<(int)sizeof(buf);p++) buf[n++]=CH_SPC[0];
    }
    if(n>0) scroll_buffer(buf,n,step);
}

void scroll_digit(int d,int step,int pad_cols){
    if(d<0) d=0; else if(d>9) d=9; // Ajuste para 0-9
    unsigned char buf[GLYPH_W+4]; int n=0;
    unsigned char *G=digit_glyph(d);
    for(int gx=0; gx<GLYPH_W; gx++) buf[n++]=G[gx];
    for(int p=0;p<pad_cols;p++) buf[n++]=CH_SPC[0];
    scroll_buffer(buf,n,step);
}

void show_life_lost(int lives_left){
    if(lives_left<0) lives_left=0;
    scroll_text_vidas(" VIDAS ", STEP_MS, COL_PAD);
    scroll_digit(lives_left, STEP_MS, COL_PAD);
}

void scroll_text(const char* s,int step,int pad_cols){
    unsigned char buf[256]; int n=0;
    for(int i=0;s[i]!='\0' && n<(int)sizeof(buf); i++){
        unsigned char *G = glyph_for_char_general(s[i]);
        for(int gx=0; gx<GLYPH_W && n<(int)sizeof(buf); gx++) buf[n++]=G[gx];
        for(int p=0;p<pad_cols && n<(int)sizeof(buf);p++) buf[n++]=CH_SPC[0];
    }
    if(n>0) scroll_buffer(buf,n,step);
}

void show_pause(void){
    const char* s="PAUSA";
    int y0=top_y();
    int total_cols = 5*GLYPH_W + 4*COL_PAD;
    int start_x = DISP_MIN + ((DISP_MAX_X-DISP_MIN+1)-total_cols)/2;
    if(start_x<DISP_MIN) start_x=DISP_MIN;
    disp_clear();
    for(int i=0;s[i]!='\0';i++){
        unsigned char *G=glyph_for_char_general(s[i]);
        for(int gx=0;gx<GLYPH_W;gx++){
            int X = start_x + i*(GLYPH_W+COL_PAD) + gx;
            if(X>=DISP_MIN && X<=DISP_MAX_X) build_col(X,y0,G[gx]);
        }
    }
    disp_update();
    usleep(1000000);
    disp_clear();
    disp_update();
}

void tap_button(void){
    int was_down=0;
    while(1){
        joyinfo_t j = joy_read();
        int pressed = (j.sw!=J_NOPRESS)?1:0;
        if(!was_down && pressed) was_down=1;
        else if(was_down && !pressed) break;
        usleep(10*1000);
    }
}

void scroll_number(int value,int step,int pad_cols){
    char buf_chr[16];
    if(value==0){ 
        buf_chr[0]='0'; 
        buf_chr[1]='\0'; 
    } else {
        // Usa snprintf para una conversión segura, si está disponible.
        // Si no, tu lógica manual está bien, pero snprintf es mejor práctica.
        snprintf(buf_chr, sizeof(buf_chr), "%d", value);
    }

    unsigned char buf[256]; int n=0;
    for(int i=0; buf_chr[i]!='\0' && n<(int)sizeof(buf); i++){
        int d = buf_chr[i]-'0';
        if(d<0 || d>9) d=0;
        unsigned char *G = digit_glyph(d);
        for(int gx=0;gx<GLYPH_W && n<(int)sizeof(buf); gx++) buf[n++]=G[gx];
        for(int p=0;p<pad_cols && n<(int)sizeof(buf);p++) buf[n++]=CH_SPC[0];
    }
    if(n>0) scroll_buffer(buf,n,step);
}

void show_menu(void){ scroll_text(" START ",STEP_MS,COL_PAD); tap_button(); }
void show_game_over(void){ scroll_text(" GAME OVER ",120,1); tap_button(); show_menu(); }
void show_win(void){ scroll_text(" GANASTE ",120,1); tap_button(); show_menu(); }
void show_score(int score){ scroll_text(" SCORE ",100,1); scroll_number(score,100,1); tap_button(); show_menu(); }