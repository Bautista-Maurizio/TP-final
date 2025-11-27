#ifndef WORDS_H
#define WORDS_H

//parametros 
#define GLYPH_W 3  //col
#define GLYPH_H 5  //fil
#define COL_PAD 1  //entre letras
#define STEP_MS 70  //tiempo de scroll
#define COL(b4,b3,b2,b1,b0)  ((unsigned char)((((b4)&1)<<4)|(((b3)&1)<<3)|(((b2)&1)<<2)|(((b1)&1)<<1)|((b0)&1))) //columna de 5 filas

void scroll_text_vidas(const char* s, int step, int pad_cols);
void build_col(int X, int topY, unsigned char colbits);
unsigned char* glyph_for_char_vidas(char ch);
int top_y(void);
void show_life_lost(int lives_left);
void show_menu(void); //start
void show_game_over(void); //game over 
void show_win(void); //ganaste
void show_score(int score);  //puntaje 
unsigned char* glyph_for_char_general(char ch);
void scroll_text(const char* s, int step, int pad_cols);
void tap_button(void);
unsigned char* digit_glyph(int d);
void scroll_number(int value, int step, int pad_cols);
void show_menu(void);
void show_game_over(void);
void show_win(void);
void show_score(int score);
void led_on(int x, int y);

#endif