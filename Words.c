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
static unsigned char CH_V  [GLYPH_W] = { COL(1,1,1,0,0), COL(0,0,0,1,1), COL(1,1,1,0,0) };
static unsigned char CH_I  [GLYPH_W] = { COL(0,0,0,0,1), COL(1,1,1,1,1), COL(0,0,0,0,1) };
static unsigned char CH_D  [GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(0,1,1,1,0) };
static unsigned char CH_A  [GLYPH_W] = { COL(1,1,1,0,1), COL(0,0,0,0,1), COL(1,1,1,0,1) };
static unsigned char CH_S  [GLYPH_W] = { COL(1,1,0,0,1), COL(1,1,1,1,1), COL(1,0,0,1,1) };

//digitos del 0 al 3 para las vidas

static unsigned char D0[GLYPH_W] = { COL(1,1,1,1,1), COL(1,0,0,0,1), COL(1,1,1,1,1) }; //0
static unsigned char D1[GLYPH_W] = { COL(0,0,0,0,1), COL(0,0,0,0,1), COL(0,0,0,0,1) }; //1
static unsigned char D2[GLYPH_W] = { COL(1,0,0,1,1), COL(1,1,1,1,0), COL(1,1,0,0,0) }; //2
static unsigned char D3[GLYPH_W] = { COL(1,0,0,1,1), COL(1,1,1,1,0), COL(1,0,0,1,1) }; //3

unsigned char* glyph_for_char_vidas(char ch){
    unsigned char* g = CH_SPC;
    if (ch == ' ') {
        g = CH_SPC;
    } 
    else if (ch == 'V' || ch == 'v') {
        g = CH_V;
    } 
    else if (ch == 'I' || ch == 'i') {
        g = CH_I;
    } 
    else if (ch == 'D' || ch == 'd') {
        g = CH_D;
    }
    else if (ch == 'A' || ch == 'a') {
        g = CH_A;
    } 
    else if (ch == 'S' || ch == 's') {
        g = CH_S;
    } 
    else {
        g = CH_SPC;
    }
    return g;
}

//dibujo columna del glyph
void build_col(int X, int topY, unsigned char colbits){
    int row = 0;
    while (row < GLYPH_H) {
        int bit = (colbits >> row) & 1;
        if (bit == 1) {
            led_on(X, topY + row);
        } 
        row = row + 1;
    }
}

//scrollea texto vida
void scroll_text_vidas(const char* s, int step, int pad_cols){
    int y0 = top_y(); //fila superior
    int x_off = DISP_MAX_X + 1; //posicion de arranque horizontal 

    int i = 0;
    while (s[i] != '\0') { //recorro el string

        unsigned char* G = glyph_for_char_vidas(s[i]);

        int gx = 0;
        while (gx < GLYPH_W) { //columnas
            disp_clear();

            int X = x_off; //x donde quiero dibujar la columna del glyph 
            if (X >= DISP_MIN) {
                if (X <= DISP_MAX_X) {
                    //prende los leds que quiero para formar la letra 
                    build_col(X, y0, G[gx]);
                }
            }

            disp_update(); //mando el buffer al hardware 
            usleep(step * 1000);
            x_off = x_off - 1; //desplazo todo una columna a la izquierda (scroll)
            gx = gx + 1;
        }

        //espacios entre letras (columnas en blanco)
        int pc = 0;
        while (pc < pad_cols) {
            disp_clear();
            disp_update();
            usleep(step * 1000);
            x_off = x_off - 1; //corro a la izquierda 
            pc = pc + 1;
        }

        i = i + 1;
    }

    disp_clear();
    disp_update();
}

//scrolleo digitos de vidas que quedan 
void scroll_digit(int d, int step, int pad_cols){
  
    if (d < 0) {
        d = 0;
    } 
    else if (d > 3) {
        d = 3;
    } 

    //elijo el glyph segun el valor 
    const unsigned char* G = CH_SPC;
    if (d == 0) {
        G = D0;
    } 
    else if (d == 1) {
        G = D1;
    } 
    else if (d == 2) {
        G = D2;
    } 
    else {
        G = D3;
    }

    //posiciones de dibujo 
    int y0 = top_y();
    int x_off = DISP_MAX_X + 1;

    //scrolleo las tres columnas del digito
    int gx = 0;
    while (gx < GLYPH_W) {
        disp_clear();

        /* dibujo esta columna si entra en la pantalla */    
        int X = x_off;
        if (X >= DISP_MIN) {
            if (X <= DISP_MAX_X) {
                build_col(X, y0, G[gx]);
            }
        }
    

        disp_update();
        usleep(step * 1000);
        x_off = x_off - 1;
        gx = gx + 1;
    }

    //columnas de espacio
    int pc = 0;
    while (pc < pad_cols) {
        disp_clear();
        disp_update();
        usleep(step * 1000);
        x_off = x_off - 1;
        pc = pc + 1;
    }
    
    //limpio
    disp_clear();
    disp_update();
}

void show_life_lost(int lives_left){
    if (lives_left < 0) {
        lives_left = 0;
    } 
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

//mapeo para palabras generales
unsigned char* glyph_for_char_general(char ch){
    unsigned char* g = CH_SPC;
    if (ch == ' ') {
        g = CH_SPC;
    } 
    else if (ch == 'A' || ch == 'a'){ 
        g = CH_A; 
    }
    else if (ch == 'C' || ch == 'c') {
        g = CH_C; 
    }
    else if (ch == 'E' || ch == 'e') { 
        g = CH_E; 
    }
    else if (ch == 'G' || ch == 'g'){ 
        g = CH_G; 
    }
    else if (ch == 'M' || ch == 'm') { 
        g = CH_M; 
    }
    else if (ch == 'N' || ch == 'n') { 
        g = CH_N; 
    }
    else if (ch == 'O' || ch == 'o') { 
        g = CH_O; 
    }
    else if (ch == 'R' || ch == 'r') { 
        g = CH_R; 
    }
    else if (ch == 'S' || ch == 's') { 
        g = CH_S; 
    }
    else if (ch == 'T' || ch == 't') { 
        g = CH_T; 
    }
    else if (ch == 'V' || ch == 'v') { 
        g = CH_V; 
    }
    return g;
}

//para scrollear (misma logica que en la de pausa)
void scroll_text(const char* s, int step, int pad_cols){
    int y0 = top_y();
    int x_off = DISP_MAX_X + 1;

    int i = 0;
    while (s[i] != '\0') {
        const unsigned char* G = glyph_for_char_general(s[i]);

        int gx = 0;
        while (gx < GLYPH_W) {
            disp_clear();
            {
                int X = x_off;
                if (X >= DISP_MIN) {
                    if (X <= DISP_MAX_X) {
                        build_col(X, y0, G[gx]);
                    }
                }
            }
            disp_update();
            usleep(step * 1000);
            x_off = x_off - 1;
            gx = gx + 1;
        }

        //separacion entre letras
        {
            int pc = 0;
            while (pc < pad_cols) {
                disp_clear();
                disp_update();
                usleep(step * 1000);
                x_off = x_off - 1;
                pc = pc + 1;
            }
        }

        i = i + 1;
    }

    disp_clear();
    disp_update();
}

//cuando apretas el boton que empiece el juego
void tap_button(void){
    int was_down = 0;  // 0 = suelto, 1 = apretado

    int loop = 1;
    while (loop == 1) {
        joyinfo_t j = joy_read();
        int pressed;
        if (j.sw != J_NOPRESS) {
            pressed = 1;
        } else {
            pressed = 0;
        }

        if (was_down == 0) {
            if (pressed == 1) {
                was_down = 1; //transición suelto a apretado
            }
        } 
        else {
            if (pressed == 0) {
                //transición apretado a suelto 
                loop = 0;
            }
        }

        usleep(10 * 1000); //10 ms 
    }
}

//digitos para el puntaje 
//desde el 4 porque 1,2,3,0 lo defini para las vidas  
static unsigned char D4[GLYPH_W] = { COL(1,1,1,0,1), COL(0,0,0,0,1), COL(0,0,0,0,1) };
static unsigned char D5[GLYPH_W] = { COL(1,1,0,0,0), COL(1,1,1,1,0), COL(1,0,0,1,1) };
static unsigned char D6[GLYPH_W] = { COL(1,1,1,1,1), COL(1,1,1,1,0), COL(1,0,0,1,1) };
static unsigned char D7[GLYPH_W] = { COL(1,0,0,0,0), COL(1,0,0,1,1), COL(1,1,1,0,0) };
static unsigned char D8[GLYPH_W] = { COL(1,1,1,1,1), COL(1,1,1,1,0), COL(1,1,1,1,1) };
static unsigned char D9[GLYPH_W] = { COL(1,0,0,1,1), COL(1,1,1,1,1), COL(1,1,1,1,1) };

unsigned char* digit_glyph(int d){
    unsigned char* G = D0;
    if (d == 0) { 
        G = D0; 
    }
    else if (d == 1) { 
        G = D1; 
    }
    else if (d == 2) { 
        G = D2; 
    }
    else if (d == 3) { 
        G = D3; 
    }
    else if (d == 4) { 
        G = D4; 
    }
    else if (d == 5) { 
        G = D5; 
    }
    else if (d == 6) { 
        G = D6; 
    }
    else if (d == 7) { 
        G = D7; 
    }
    else if (d == 8) { 
        G = D8; 
    }
    else { 
        G = D9; 
    }
    return G;
}

//para scrollear numeros
void scroll_number(int value, int step, int pad_cols){
    
    //convierto a string 
    char buf[16];
    if (value == 0) {
        buf[0] = '0'; //si es cero
        buf[1] = '\0'; //fin de cadena 
    } 
    else {
        char tmp[16];
        int tlen = 0;
        int v = value;
        //descompongo digitos desde el final y los guardo en orden inverso
        while (v > 0) {
            int d = v % 10; //ult digito
            tmp[tlen] = (char)('0' + d); //lo guardo como caracter
            tlen = tlen + 1; //avanzo la longitud de tmp
            v = v / 10; //saco ult digito
        }
        int n = 0;
        while (tlen > 0) {
            tlen = tlen - 1; //para leer el ultimo cargado en tmp
            buf[n] = tmp[tlen];
            n = n + 1;
        }
        buf[n] = '\0'; //cierro la cadena 
    }

    //recorro buff y scrolleo cada digito 
    int y0 = top_y();
    int x_off = DISP_MAX_X + 1;

    int i = 0;
    while (buf[i] != '\0') {
        int d = buf[i] - '0'; //convierto a caracter 
        //valido 
        if (d < 0) { 
            d = 0; 
        }
        else if (d > 9) { 
            d = 0; 
        }

        unsigned char* G = digit_glyph(d); //pido el digito 

        int gx = 0;
        while (gx < GLYPH_W) { //dibujo columna
            disp_clear();
            int X = x_off;
            if (X >= DISP_MIN) {
                if (X <= DISP_MAX_X) {
                    build_col(X, y0, G[gx]);
                }
            }
        
            disp_update();
            usleep(step * 1000);
            x_off = x_off - 1;
            gx = gx + 1;
        }

        //espacio entre digitos 
        {
            int pc = 0;
            while (pc < pad_cols) {
                disp_clear();
                disp_update();
                usleep(step * 1000);
                x_off = x_off - 1;
                pc = pc + 1;
            }
        }

        i = i + 1;
    }

    disp_clear();
    disp_update();
}

//----MENU INCIAL--------
void show_menu(void){
    scroll_text(" START ", STEP_MS, COL_PAD);
    tap_button(); //con un tap arranca 
}

//---GAME OVER--------
void show_game_over(void){
    scroll_text(" GAME OVER ", 120, 1); 
    tap_button(); //con tap se vuelve al menu
    show_menu();
}

//-----GANASTE----
void show_win(void){
    scroll_text(" GANASTE ", 120, 1);
    tap_button(); //con tap se vuelve al menu
    show_menu();
}

//-----SCORE-----   
void show_score(int score){
    scroll_text(" SCORE ", 100, 1);
    scroll_number(score, 100, 1);
    tap_button(); //con tap se veulve al menu
    show_menu();
}