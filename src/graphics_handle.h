#ifndef __GRAPHICS_HANDLE_H__
#define __GRAPHICS_HANDLE_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include "network.h"

void           initialize_SDL        (void);
void           initialize_TTF        (void);
SDL_Window   * create_window         (void);
SDL_Renderer * create_renderer       (SDL_Window * window);
void           construct_draw_array  (Dimension dim, int *** drawing_surface_values);
void           free_draw_array       (int ** drawing_surface_values, Dimension dim);
SDL_Surface  * load_icon             (char * filepath, SDL_Window * window);
void           render_network        (SDL_Renderer * renderer, Network * network, SDL_Texture * neuron, int window_width, int window_height);
void           activate_draw_array   (Network * network, int ** drawing_surface_values, int width, int height);
void           render_button         (SDL_Renderer * renderer, Button dimensions, char * text, TTF_Font * font);
void           render_button_click   (SDL_Renderer * renderer, Button dimensions, char * text, TTF_Font * font);
void           render_drawing_surface(SDL_Renderer * renderer, Button dimensions, int ** drawing_surface_values);
bool           in_button_bounds      (Button dimensions, int mouse_x, int mouse_y);

#endif