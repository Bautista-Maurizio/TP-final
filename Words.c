#include <unistd.h>
#include "disdrv.h"
#include "Words.h"
#include "joydrv.h"

// vidas

// centrado vertical del bloque de texto
int top_y (void) {
    int h = (DISP_MAX_Y - DISP_MIN) + 1;
    int off = (h - GLYPH_H) / 2;
    
    if (off < 0) {
    	 off = 0;
    }
   
return DISP_MIN + off;

}

// glyphs para vidas y espacios
static unsigned char CH_SPC[GLYPH_W] = { COL(0,0,0,0,0), COL(0,0,0,0,0), COL(0,0,0,0,0) };
static unsigned char CH_V[GLYPH_W] = { COL(1,1,1,0,0), COL(0,0,0,1,1), COL(1,1,1,0,0) };
static unsigned char CH_I[GLYPH_W] = { COL(1, 0, 0, 0, 1), COL(1, 1, 1, 1, 1), COL(1, 0, 0, 0, 1) };
static unsigned char CH_D[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(0,1,1,1,0) };
static unsigned char CH_A[GLYPH_W] = { COL(0, 1, 1, 1, 1), COL(1, 0, 1, 0, 0), COL(0, 1, 1, 1, 1) };
static unsigned char CH_S[GLYPH_W] = { COL(1,1,0,0,1), COL(1,1,1,1,1), COL(1,0,0,1,1) };

// dígitos 0..3 para vidas
static unsigned char D0[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,1,1,1,1) };
static unsigned char D1[GLYPH_W] = { COL(0, 0, 0, 0, 0), COL(1, 1, 1, 1, 1), COL(0, 0, 0, 0, 0) };
static unsigned char D2[GLYPH_W] = { COL(1, 0, 1, 1, 1), COL(1, 0, 1, 0, 1), COL(1, 1, 1, 0, 1) };
static unsigned char D3[GLYPH_W] = { COL(1, 0, 1, 0, 1), COL(1, 0, 1, 0, 1), COL(1, 1, 1, 1, 1) };

// letras generales 
static unsigned char CH_G[GLYPH_W] = { COL(0, 1, 1, 1, 0), COL(1, 0, 0, 0, 1), COL(1, 0, 1, 1, 1) };
static unsigned char CH_M[GLYPH_W] = { COL(1, 1, 1, 1, 1), COL(0, 0, 0, 1, 0), COL(1, 1, 1, 1, 1) };
static unsigned char CH_E[GLYPH_W] = { COL(1, 1, 1, 1, 1), COL(1, 0, 1, 0, 1), COL(1, 0, 1, 0, 1) };
static unsigned char CH_O[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,1,1,1,1) };
static unsigned char CH_R[GLYPH_W] = { COL(1, 1, 1, 1, 1), COL(1, 0, 1, 0, 0), COL(0, 1, 0, 1, 1) };
static unsigned char CH_T[GLYPH_W] = { COL(1,0,0,0,0), COL(1,1,1,1,1), COL(1,0,0,0,0) };
static unsigned char CH_N[GLYPH_W] = { COL(1, 1, 1, 1, 1), COL(0, 1, 1, 0, 0), COL(1, 1, 1, 1, 1) };
static unsigned char CH_C[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,0,0,0,0) };

// dígitos 4 a 9
static unsigned char D4[GLYPH_W] = { COL(1, 1, 1, 0, 0), COL(0, 0, 1, 0, 0), COL(1, 1, 1, 1, 1) };
static unsigned char D5[GLYPH_W] = { COL(1, 1, 1, 0, 1), COL(1, 0, 1, 0, 1), COL(1, 0, 1, 1, 1) };
static unsigned char D6[GLYPH_W] = { COL(1, 1, 1, 1, 1), COL(1, 0, 1, 0, 1), COL(1, 0, 1, 1, 1) };
static unsigned char D7[GLYPH_W] = { COL(1, 0, 0, 0, 0), COL(1, 0, 0, 1, 1), COL(1, 1, 1, 0, 0) };
static unsigned char D8[GLYPH_W] = { COL(1, 1, 1, 1, 1), COL(1, 0, 1, 0, 1), COL(1, 1, 1, 1, 1) };
static unsigned char D9[GLYPH_W] = { COL(1, 1, 1, 0, 1), COL(1, 0, 1, 0, 1), COL(1, 1, 1, 1, 1) };

// helpers

// función interna: desplaza y dibuja un buffer de columnas (n_cols) en la pantalla

static void scroll_buffer (const unsigned char *buf, int n_cols, int step) {
    int y0 = top_y();
    int disp_width = DISP_MAX_X - DISP_MIN + 1;

    // desplazamiento: inicio con todo el buffer fuera a la derecha
    
    for (int off = 0; off < n_cols + disp_width; off++) {
        disp_clear();
        // dibujo todas las columnas del buffer que caen dentro de la pantalla
        for (int c = 0; c < n_cols; c++) {
            int X = DISP_MAX_X - off + c;
            
            if (X >= DISP_MIN && X <= DISP_MAX_X) {
                build_col(X, y0, buf[c]);
            }
        }
        
        disp_update();
        usleep(step * 1000);
    }

    // limpio al final
    disp_clear();
    disp_update();
}

// funciones públicas 

unsigned char* glyph_for_char_vidas(char ch) {
    if (ch == 'V' || ch == 'v') {
   	return CH_V;
    }
   
    else if (ch == 'I' || ch == 'i') {
    	return CH_I;
    }
    
    else if (ch == 'D' || ch == 'd') {
    	return CH_D;
    }
    
    else if (ch == 'A' || ch == 'a') {
    	return CH_A;
    }
    
    else if (ch == 'S' || ch == 's') {
    	return CH_S;
    }
    
    else {
    	return CH_SPC;
    }

}

unsigned char* glyph_for_char_general (char ch) {
    
    if (ch == 'A' || ch == 'a') {
    	return CH_A;
    }
    
    else if (ch == 'C' || ch == 'c') {
    	return CH_C;
    }
    
    else if (ch == 'E' || ch == 'e') {
    	return CH_E;
    }
   
    else if (ch == 'G' || ch == 'g') {
    	return CH_G;
    }
    
    else if (ch == 'M' || ch == 'm') {
    	return CH_M;
    }
    
    else if (ch == 'N' || ch == 'n') {
    	return CH_N;
    }
    
    else if (ch == 'O' || ch == 'o') {
    	return CH_O;
    }
    
    else if (ch == 'R' || ch == 'r') {
    	return CH_R;
    }
   
    else if (ch == 'S' || ch == 's') {
    	return CH_S;
    }
    
    else if (ch == 'T' || ch == 't') {
    	return CH_T;
    }
    
    else if (ch == 'V' || ch == 'v') {
    	return CH_V;
    }
    
    else {
     	return CH_SPC;
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

// dibuja una columna 
void build_col(int X, int topY, unsigned char colbits) {
    
    for (int row = 0; row < GLYPH_H; row++) {
        
        if ((colbits >> row) & 1) {
            led_on(X, topY + row);
        }
    
    }

}

// Scroll para texto de vidas
void scroll_text_vidas (const char* s, int step, int pad_cols) {
    unsigned char buf[256];
    int n = 0;
    
    for (int i = 0; s[i] != '\0' && n < (int)sizeof(buf); i++) {
        unsigned char *G = glyph_for_char_vidas(s[i]);
        
        for (int gx = 0; gx < GLYPH_W && n < (int)sizeof(buf); gx++) {
        	buf[n++] = G[gx];
        }
        
        for (int p = 0; p < pad_cols && n < (int)sizeof(buf); p++) {
        	buf[n++] = CH_SPC[0];
    	}
    
    if (n > 0) {
    	scroll_buffer(buf, n, step);
    }

}

// Scroll para un solo dígito (vidas)
void scroll_digit (int d, int step, int pad_cols) {
    
    if (d < 0) {
    	d = 0;
    }
    
    else if (d > 3) {
    	d = 3;
    }
    
    unsigned char buf[GLYPH_W + 4];
    int n = 0;
    unsigned char *G = digit_glyph(d);
    
    for (int gx = 0; gx < GLYPH_W; gx++) {
    	buf[n++] = G[gx];
    }
    
    for (int p = 0; p < pad_cols; p++) {
    	buf[n++] = CH_SPC[0];
    }
    
    scroll_buffer(buf, n, step);
}

void show_life_lost (int lives_left) {
    if (lives_left < 0) {
    	lives_left = 0;
    }
    
    scroll_text_vidas(" VIDAS ", STEP_MS, COL_PAD);
    scroll_digit(lives_left, STEP_MS, COL_PAD);
}

// Scroll general para texto
void scroll_text (const char* s, int step, int pad_cols) {
    unsigned char buf[256];
    int n = 0;
    
    for (int i = 0; s[i] != '\0' && n < (int)sizeof(buf); i++) {
        unsigned char *G = glyph_for_char_general(s[i]);
        
        for (int gx = 0; gx < GLYPH_W && n < (int)sizeof(buf); gx++) {
        	buf[n++] = G[gx];
        }
        
        for (int p = 0; p < pad_cols && n < (int)sizeof(buf); p++) {
        	buf[n++] = CH_SPC[0];
        }
    
    }
    
    if (n > 0) {
	scroll_buffer(buf, n, step);
    }
}

// Pausa estatica (muestro la palabra centrada durante 1s)
void show_pause (void) {
    const char* s = "PAUSA";
    int y0 = top_y();

    // ancho total en columnas = 5 letras * GLYPH_W + 4 * COL_PAD
    
    int total_cols = 5 * GLYPH_W + 4 * COL_PAD;
    int start_x = DISP_MIN + ((DISP_MAX_X - DISP_MIN + 1) - total_cols) / 2;
    
    if (start_x < DISP_MIN) {
    	start_x = DISP_MIN;
    }
    
    disp_clear();
    
    for (int i = 0; s[i] != '\0'; i++) {
       
        unsigned char *G = glyph_for_char_general(s[i]);
        
        for (int gx = 0; gx < GLYPH_W; gx++) {
            int X = start_x + i * (GLYPH_W + COL_PAD) + gx;
            
            if (X >= DISP_MIN && X <= DISP_MAX_X) {
            	build_col(X, y0, G[gx]);
            }
        }
    }
    
    disp_update();
    usleep(1000000);
    disp_clear();
    disp_update();

}

// Tap button
void tap_button (void) {
    int was_down = 0;
    
    while (1) { 
        
        joyinfo_t j = joy_read();
        int pressed = (j.sw != J_NOPRESS) ? 1 : 0;
        
        if (!was_down && pressed) {
        	was_down = 1;
        }
        
        else if (was_down && !pressed) {
        	break;
        }
        
        usleep(10 * 1000);
    }
}

// Digitos para score 
unsigned char* digit_glyph_for_score (int d) {
    return digit_glyph(d);
}

// Scroll para números (puntaje)
void scroll_number (int value, int step, int pad_cols) {
    char buf_chr[16];
    
    if (value == 0){
        
        buf_chr[0] = '0';
        buf_chr[1] = '\0';
    } 
    
    else {
        char tmp[16];
        int tlen = 0;
        int v = value;
        
        while (v > 0 && tlen < (int)sizeof(tmp)) {
            tmp[tlen++] = '0' + (v % 10);
            v /= 10;
        }
        
        int n = 0;
        
        while (tlen > 0) buf_chr[n++] = tmp[--tlen] {
             buf_chr[n] = '\0';
   	}
    }

    unsigned char buf[256];
    int n = 0;
    
    for (int i = 0; buf_chr[i] != '\0' && n < (int)sizeof(buf); i++) {
        int d = buf_chr[i] - '0';
        
        if (d < 0 || d > 9) {
        	d = 0;
        }
       
        unsigned char *G = digit_glyph(d);
        
        for (int gx = 0; gx < GLYPH_W && n < (int)sizeof(buf); gx++) {
        	buf[n++] = G[gx];
        }
        
        for (int p = 0; p < pad_cols && n < (int)sizeof(buf); p++) {
        	buf[n++] = CH_SPC[0];
        }
    }
    
    if (n > 0) {
    	scroll_buffer(buf, n, step);
    }

}

// Menus

void show_menu (void) {
    
    scroll_text(" START ", STEP_MS, COL_PAD);
    tap_button();
}

void show_game_over (void) {
    
    scroll_text(" GAME OVER ", 120, 1);
    tap_button();

}

void show_win (void) {
    
    scroll_text("GANASTE ", 70, 1);
    tap_button();

}

void show_score (int score) {
    
    scroll_text(" SCORE ", 100, 1);
    scroll_number(score, 100, 1);
    tap_button();

}

void draw_score_static (int score) {
    disp_clear();
    
    // Tope visual
    
    if (score > 9999) {
    	score = 9999; 
    }
   
    // Convertir int a string
    
    char buf[16];
    int len = 0;
    int temp = score;
    
    if (temp == 0) {
        buf[0] = '0'; len = 1;
    } 
    
    else {
        int val = temp;
        while (val > 0) {
        	val /= 10; len++;
        }
        
        val = temp;
        
        for(int i=0; i<len; i++) {
            buf[len - 1 - i] = '0' + (val % 10);
            val /= 10;
        }
    }
    
    buf[len] = '\0';

    // Calcular Centro
    // Ancho = (letras * 3) + (espacios)
    
    int ancho_total = (len * 3) + (len - 1);
    int start_x = (16 - ancho_total) / 2;
    
    if (start_x < 0) {
    	start_x = 0;
    }
   
    int top_y_val = top_y(); 

    // Dibujar
    int current_x = start_x;
    
    for (int i=0; i<len; i++) {
        int d = buf[i] - '0';
        unsigned char* G = digit_glyph(d);
        
        for (int col=0; col<3; col++) {
            if (current_x < 16) {
                build_col(current_x, top_y_val, G[col]);
            }
            current_x++;
        }
        current_x++; // Espacio
    }

    // actualizo pantalla
    disp_update();
}