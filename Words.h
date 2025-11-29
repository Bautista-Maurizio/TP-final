#ifndef WORDS_H
#define WORDS_H

#define GLYPH_W 3    //columnas (ancho del carácter)
#define GLYPH_H 5    //filas (alto del carácter)
#define COL_PAD 1    //espacio entre letras
#define STEP_MS 70   //tiempo de scroll

// Macro para definir columnas de 5 filas.
#define COL(b4,b3,b2,b1,b0) (((((b0)&1)<<4)|(((b1)&1)<<3)|(((b2)&1)<<2)|(((b3)&1)<<1)|((b4)&1)))


//para vidas 
void scroll_text_vidas(const char* s, int step, int pad_cols);
unsigned char* glyph_for_char_vidas(char ch);
void scroll_digit(int d, int step, int pad_cols);
void show_life_lost(int lives_left);

//funciones de scroll
unsigned char* glyph_for_char_general(char ch);
void scroll_text(const char* s, int step, int pad_cols);
unsigned char* digit_glyph(int d);

void show_menu(void);
void show_game_over(void);
void show_win(void);


int top_y(void);
void build_col(int X, int topY, unsigned char colbits);
void tap_button(void);
void led_on(int x, int y); 
void draw_score_static(int score);

#endif
