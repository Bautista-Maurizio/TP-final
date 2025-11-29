#ifndef WORDS_H
#define WORDS_H

// Parámetros
#define GLYPH_W 3    // columnas (ancho del carácter)
#define GLYPH_H 5    // filas (alto del carácter)
#define COL_PAD 1    // espacio entre letras
#define STEP_MS 70   // tiempo de scroll

// Macro para definir columnas de 5 filas.
// IMPORTANTE: b0 es la fila SUPERIOR (MSB), b4 es la fila INFERIOR (LSB).
// COL(Superior, b3, b2, b1, Inferior)
#define COL(b4,b3,b2,b1,b0) \
  ((unsigned char)((((b0)&1)<<4)|(((b1)&1)<<3)|(((b2)&1)<<2)|(((b3)&1)<<1)|((b4)&1)))

// ... (El resto de las declaraciones de funciones se mantienen igual) ...

// Funciones de "vidas"
void scroll_text_vidas(const char* s, int step, int pad_cols);
unsigned char* glyph_for_char_vidas(char ch);
void scroll_digit(int d, int step, int pad_cols);
void show_life_lost(int lives_left);

// Funciones genéricas de scroll
unsigned char* glyph_for_char_general(char ch);
void scroll_text(const char* s, int step, int pad_cols);
unsigned char* digit_glyph(int d);
void scroll_number(int value, int step, int pad_cols);

// Funciones de menú y pantallas
void show_menu(void);
void show_game_over(void);
void show_win(void);
void show_score(int score);
void show_pause(void);  // pausa con recuadro

// Funciones auxiliares (necesarias para el linkeo)
int top_y(void);
void build_col(int X, int topY, unsigned char colbits); // <--- Aquí va el cambio
void tap_button(void);
void led_on(int x, int y); // Asumimos que esta es la función de bajo nivel

#endif