#include "graphics_handle.h"
#include "definitions.h"
#include "network.h"

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <unistd.h>

static int min(int a, int b)
{
    if(a < b) return a;
    else return b;
}

void initialize_SDL(void)
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0) 
    {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
    }
}

void initialize_TTF(void)
{
    if(TTF_Init() < 0) 
    {
        printf("SDL_ttf initialization error: %s\n", TTF_GetError());
    }
}

SDL_Window * create_window(void)
{
    SDL_Window * window = SDL_CreateWindow(WINDOW_TITLE, 
                                           SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED,
                                           WINDOW_WIDTH, WINDOW_HEIGHT, 
                                           SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE
                                          );
    if(!window) 
    {
        fprintf(stderr, "Error SDL creating window: %s\n", SDL_GetError());
        SDL_Quit();
    }

    return window;
}

SDL_Renderer * create_renderer(SDL_Window * window)
{
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    if(!renderer) 
    {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    return renderer;
}

void construct_draw_array(Dimension dim, int *** drawing_surface_values)
{
    int rows = dim.row; // grab rows
    int cols = dim.col; // grab cols

    * drawing_surface_values = (int **) malloc(rows * sizeof(int *));     // malloc rows of dataframe
    for(int i = 0; i < rows; i++)
    {
        (* drawing_surface_values)[i] = (int *) malloc(cols * sizeof(int)); // malloc columns of dataframe
    }
}

void free_draw_array(int ** drawing_surface_values, Dimension dim)
{
    for (int i = 0; i < dim.row; i++) 
    {
        free(drawing_surface_values[i]);
    }
    free(drawing_surface_values);
}

SDL_Surface * load_icon(char * filepath, SDL_Window * window)
{
    SDL_Surface * icon = SDL_LoadBMP(filepath);
    
    // if(!icon)
    // {
    //     fprintf(stderr, "Error loading icon image: %s\n", SDL_GetError());
    //     SDL_DestroyWindow(window);
    //     SDL_Quit();
    // }

    return icon;
}

void render_network(SDL_Renderer * renderer, Network * network, SDL_Texture * neuron, int window_width, int window_height)
{
    Layer  * main_layer = network -> layers;
    Layer  * layer_pt   = main_layer;
    Neuron * neuron_pt;
    Weight * weight_pt;

    int max_size = 0;
    while(layer_pt != NULL)
    {
        if(layer_pt -> size > max_size) max_size = layer_pt -> size;
        layer_pt = layer_pt -> next;
    }
    layer_pt = main_layer;

    if(max_size > MAX_LAYER_SIZE) max_size = MAX_LAYER_SIZE;

    int size  = min(window_height / ((max_size * 1.2) + 0.2), window_width / ((network -> size - 1) * 9));
    int space = size / 5;
    
    int x               = 0;
    int y               = 0;
    int next_layer_size = 0;

    int next_x;
    int next_y;

    int neuron_brightness;

    while(layer_pt != NULL)
    {
        if(layer_pt -> next != NULL) next_layer_size = (layer_pt -> next) -> size;
        else next_layer_size = 0;

        neuron_pt = layer_pt -> neurons;
        if(layer_pt -> size > MAX_LAYER_SIZE)
        {
            if((layer_pt -> size) / 2 > MAX_LAYER_SIZE) for(int i = 0; i < ((layer_pt -> size) / 2) - 5; i++) neuron_pt = neuron_pt -> next; // get to middle of our values
            y = (window_height / 2) - ((MAX_LAYER_SIZE) * (size + space) / 2) + space;

            for(int i = 0; i < MAX_LAYER_SIZE; i++)
            {
                if(i == MAX_LAYER_SIZE / 2) // middle
                {
                    x += size / 3;
                    for(int j = 0; j < 3; j++)
                    {
                        SDL_Rect neuron_rect = {x, y, (size / 3), (size / 3)};
                        SDL_RenderCopy(renderer, neuron, NULL, &neuron_rect);

                        y += (size / 3) + (space / 3);
                    }
                    x -= size / 3;
                } else {
                    SDL_Rect neuron_rect = {x, y, size, size};
                    neuron_brightness = (neuron_pt -> activation) * 255.0;
                    SDL_SetRenderDrawColor(renderer, neuron_brightness, neuron_brightness, neuron_brightness, neuron_brightness);
                    SDL_RenderFillRect(renderer, &neuron_rect);
                    SDL_RenderCopy(renderer, neuron, NULL, &neuron_rect);

                    // draw line to next layer
                    next_x = x + (size * 8) + space;
                    next_y = (window_height / 2) - ((next_layer_size) * (size + space) / 2) + space;

                    weight_pt = neuron_pt -> weights;
                    for(int i = 0; i < next_layer_size; i++)
                    {
                        SDL_SetRenderDrawColor(renderer, weight_pt -> val * 255, 63, 31, 255);
                        SDL_RenderDrawLine(renderer, x + size, y + (size / 2), next_x, next_y + (size / 2));
                        next_y += size + space;
                        weight_pt = weight_pt -> next;
                    }
                    
                    y += size + space;
                }

                neuron_pt = neuron_pt -> next;
            }
        } else {
            y = (window_height / 2) - ((layer_pt -> size) * (size + space) / 2) + space;
            
            while(neuron_pt != NULL)
            {
                SDL_Rect neuron_rect = {x, y, size, size};
                neuron_brightness = (neuron_pt -> activation) * 255.0;
                SDL_SetRenderDrawColor(renderer, neuron_brightness, neuron_brightness, neuron_brightness, neuron_brightness);
                SDL_RenderFillRect(renderer, &neuron_rect);
                SDL_RenderCopy(renderer, neuron, NULL, &neuron_rect);

                // draw line to next layer
                next_x = x + (size * 8) + space;
                next_y = (window_height / 2) - ((next_layer_size) * (size + space) / 2) + space;

                weight_pt = neuron_pt -> weights;
                for(int i = 0; i < next_layer_size; i++)
                {
                    SDL_SetRenderDrawColor(renderer, weight_pt -> val * 255, 63, 31, 255);
                    SDL_RenderDrawLine(renderer, x + size, y + (size / 2), next_x, next_y + (size / 2));
                    next_y += size + space;
                    weight_pt = weight_pt -> next;
                }
                
                y += size + space;
                neuron_pt = neuron_pt -> next;
            }
        }

        x += (size * 8) + space;
        y = 0;

        layer_pt = layer_pt -> next;
    }
}

void activate_draw_array(Network * network, int ** drawing_surface_values, int width, int height)
{
    double input[width * height];
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++) input[(i * 28) + j] = drawing_surface_values[j][i];
    }

    get_input_activation(network, input, (width * height));
    activate_network(network);

    Layer * main_layer = network -> layers;
    Layer * layer_pt   = main_layer;

    while(layer_pt -> next != NULL) layer_pt = layer_pt -> next;

    Neuron * neuron_pt = layer_pt -> neurons;

    double max  = 0;
    int max_pos = 0;
    int ct      = 0;
    while(neuron_pt != NULL)
    {
        if((neuron_pt -> activation) > max)
        {
            max = neuron_pt -> activation;
            max_pos = ct;
        }

        ct++;
        neuron_pt = neuron_pt -> next;
    }
    printf("%d\n", max_pos);
}

void render_button(SDL_Renderer * renderer, Button dimensions, char * text, TTF_Font * font)
{
    SDL_Color white = {255, 255, 255};
    
    SDL_Rect outer_box = {dimensions.x, dimensions.y, dimensions.w, dimensions.h};
    SDL_Rect inner_box = {dimensions.x + 2, dimensions.y + 2, dimensions.w - 4, dimensions.h - 4};

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &outer_box);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &inner_box);

    SDL_Surface * text_surf = TTF_RenderText_Solid(font, text, white);
    SDL_Texture * text_text = SDL_CreateTextureFromSurface(renderer, text_surf);
    SDL_Rect text_rect = {dimensions.x + 2, dimensions.y + 2, dimensions.w - 4, dimensions.h - 4};

    SDL_RenderCopy(renderer, text_text, NULL, &text_rect);
    
    SDL_FreeSurface(text_surf);
    SDL_DestroyTexture(text_text);
}

void render_button_click(SDL_Renderer * renderer, Button dimensions, char * text, TTF_Font * font)
{
    SDL_Rect white_box = {dimensions.x, dimensions.y, dimensions.w, dimensions.h};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &white_box);

    SDL_Color black = {0, 0, 0};
    SDL_Surface * text_surf = TTF_RenderText_Solid(font, text, black);
    SDL_Texture * text_text = SDL_CreateTextureFromSurface(renderer, text_surf);
    SDL_Rect text_rect = {dimensions.x + 2, dimensions.y + 2, dimensions.w - 4, dimensions.h - 4};
    SDL_RenderCopy(renderer, text_text, NULL, &text_rect);
    SDL_RenderPresent(renderer);

    usleep(100000);
    render_button(renderer, dimensions, text, font);
    SDL_RenderPresent(renderer);

    SDL_FreeSurface(text_surf);
    SDL_DestroyTexture(text_text);
}

void render_drawing_surface(SDL_Renderer * renderer, Button dimensions, int ** drawing_surface_values)
{
    SDL_Rect drawing_box       = {dimensions.x, dimensions.y, dimensions.w, dimensions.h};
    SDL_Rect drawing_box_inner = {dimensions.x + 2, dimensions.y + 2, dimensions.w - 4, dimensions.h - 4};
    SDL_Rect drawing_pixels;

    // draw outline surrounding draw surface
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &drawing_box);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &drawing_box_inner);

    double x_pixel;
    double y_pixel;
    double w_pixel;
    double h_pixel;

    int shade = 0;

    for (int x = 0; x < DRAWING_SIZE; x++) {
        for (int y = 0; y < DRAWING_SIZE; y++) {
            if (drawing_surface_values[x][y] > 0) {
                x_pixel = (double) x * dimensions.w / DRAWING_SIZE;
                y_pixel = (double) y * dimensions.h / DRAWING_SIZE;
                w_pixel = dimensions.w / DRAWING_SIZE;
                h_pixel = dimensions.h / DRAWING_SIZE;

                shade = drawing_surface_values[x][y];

                drawing_pixels.x = (int) dimensions.x + x_pixel;
                drawing_pixels.y = (int) dimensions.y + y_pixel;
                drawing_pixels.w = (int) w_pixel - 1;
                drawing_pixels.h = (int) h_pixel - 1;

                SDL_SetRenderDrawColor(renderer, shade, shade, shade, shade);
                SDL_RenderFillRect(renderer, &drawing_pixels);
            }
        }
    }
}

bool in_button_bounds(Button dimensions, int mouse_x, int mouse_y)
{
    if(mouse_x >= dimensions.x                &&
       mouse_x <= dimensions.x + dimensions.w &&
       mouse_y >= dimensions.y                &&
       mouse_y <= dimensions.y + dimensions.h )
    {
        return true;
    }
    else return false;
}