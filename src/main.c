#include "definitions.h"
#include "data_handle.h"
#include "network.h"
#include "graphics_handle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

static int min(int a, int b)
{
    if(a < b) return a;
    else return b;
} 

int main(int argc, char * argv[])
{
    srand(MAIN_SEED);

    char filepath[FILE_BUFF] = DATA_DIR;
    strcat(filepath, DATA_FILE);          // finalize filepath
    FILE * fp = fopen(filepath, "r");     // open our file

    if(fp == NULL) // double check that our file opened
    {
        printf("Unable to locate file %s\n", filepath);
    }

    CharNode * features = (CharNode *) malloc(sizeof(CharNode));
    double  ** train_set     = NULL;
    Dimension train_dim       = get_frame(fp, &train_set, features);

    char test_filepath[FILE_BUFF] = DATA_DIR;
    strcat(test_filepath, TEST_FILE);
    fp = fopen(test_filepath, "r");

    if(fp == NULL) // double check that our file opened
    {
        printf("Unable to locate file %s\n", test_filepath);
    }

    CharNode * test_features = (CharNode *) malloc(sizeof(CharNode));
    double  ** test_set      = NULL;
    Dimension test_dim       = get_frame(fp, &test_set, test_features);

    fclose(fp);

    int layer_sizes[] = {(DRAWING_SIZE * DRAWING_SIZE), 16, 16, 10};
    Network * network = create_network(layer_sizes, sizeof(layer_sizes) / sizeof(layer_sizes[0]));

    initialize_SDL();
    initialize_TTF();

    // initialize drawing structures
    SDL_Window   * window     = create_window();
    SDL_Renderer * renderer   = create_renderer(window);
    TTF_Font     * bebas_neue = TTF_OpenFont(DEFAULT_FONT, 1000);
    SDL_Texture  * neuron     = SDL_CreateTextureFromSurface(renderer, load_icon(NEURON_ICON, window));

    // initialize input digit drawing
    int ** drawing_surface_values;
    Dimension draw_dim;
    draw_dim.row = DRAWING_SIZE;
    draw_dim.col = DRAWING_SIZE;
    construct_draw_array(draw_dim, &drawing_surface_values);
    for (int i = 0; i < DRAWING_SIZE; i++) for (int j = 0; j < DRAWING_SIZE; j++) drawing_surface_values[i][j] = 0; // start out as black

    int mouse_x;
    int mouse_y;
    int prev_mouse_x;
    int prev_mouse_y;

    int window_height;
    int window_width;
    SDL_GetWindowSize(window, &window_width, &window_height);

    int current_epoch = 0;

    Button draw_input;
    double x_draw;
    double y_draw;

    Button clear_button;
    Button train_button;
    Button test_button;
    Button print_button;
    Button reset_button; 

    Button nine_button;
    Button eight_button;
    Button seven_button;
    Button six_button;    
    Button five_button;    
    Button four_button;
    Button three_button;
    Button two_button;
    Button one_button;
    Button zero_button;

    bool quit    = false;
    bool drawing = false;

    bool update_draw_surface = false;
    bool update_network      = false;
    bool update_screen_size  = true;

    while(!quit) 
    {
        SDL_Event event;
        while(SDL_PollEvent(&event)) 
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    // draw input
                    if(in_button_bounds(draw_input, mouse_x, mouse_y))
                    {
                        x_draw = ((double) mouse_x - window_width + draw_input.w) / draw_input.w * DRAWING_SIZE;
                        y_draw = ((double) mouse_y) / draw_input.h * DRAWING_SIZE;
                        
                        drawing_surface_values[(int) x_draw][(int) y_draw] = 255;
                        if((int) x_draw > 0  && drawing_surface_values[(int) x_draw - 1][(int) y_draw] < 1) drawing_surface_values[(int) x_draw - 1][(int) y_draw] = 127;
                        if((int) y_draw > 0  && drawing_surface_values[(int) x_draw][(int) y_draw - 1] < 1) drawing_surface_values[(int) x_draw][(int) y_draw - 1] = 127;
                        if((int) x_draw < 27 && drawing_surface_values[(int) x_draw + 1][(int) y_draw] < 1) drawing_surface_values[(int) x_draw + 1][(int) y_draw] = 127;
                        if((int) y_draw < 27 && drawing_surface_values[(int) x_draw][(int) y_draw + 1] < 1) drawing_surface_values[(int) x_draw][(int) y_draw + 1] = 127;

                        update_draw_surface = true;
                        update_network      = true;
                        drawing             = true;
                    }
                    // clear button
                    if(in_button_bounds(clear_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, clear_button, "clear", bebas_neue);
                        for (int i = 0; i < DRAWING_SIZE; i++)
                        {
                            for (int j = 0; j < DRAWING_SIZE; j++) drawing_surface_values[i][j] = 0; // start out as black
                        }
                        update_draw_surface = true;
                    }
                    // train button
                    if(in_button_bounds(train_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, train_button, "train", bebas_neue);
                        for(int i = 1; i < 101; i++) 
                        {
                            current_epoch++;
                            printf("(%2d) ", current_epoch);
                            epoch(network, train_set, train_dim.col, train_dim.row);
                            if(i % 10 == 0) printf("Test loss: %lf\n\n", test_network(network, test_set, test_dim.col, test_dim.row));
                        }
                        update_network = true;
                    }
                    // test button
                    if(in_button_bounds(test_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, test_button, "test", bebas_neue);
                        printf("Test loss: %lf\n", test_network(network, test_set, test_dim.col, test_dim.row));
                    }
                    // print button
                    if(in_button_bounds(print_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, print_button, "print", bebas_neue);
                        print_network(network);
                    }
                    // reset button
                    if(in_button_bounds(reset_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, reset_button, "reset", bebas_neue);
                        reset_network(network);
                        update_network = true;
                    }
                    // nine button
                    if(in_button_bounds(nine_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, nine_button, "9", bebas_neue);
                        // get row with number
                        int number_row = (rand() / (double) RAND_MAX) * (test_dim.row / 2.0);
                        while(test_set[number_row][0] != 9.0) number_row++;
                        // fill screen
                        for (int i = 0; i < DRAWING_SIZE; i++) for (int j = 0; j < DRAWING_SIZE; j++) drawing_surface_values[j][i] = test_set[number_row][(i * 28) + j + 1];
                        update_draw_surface = true;
                        update_network      = true;
                    }
                    // eight button
                    if(in_button_bounds(eight_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, eight_button, "8", bebas_neue);
                        int number_row = (rand() / (double) RAND_MAX) * (test_dim.row / 2.0);
                        while(test_set[number_row][0] != 8.0) number_row++;
                        for (int i = 0; i < DRAWING_SIZE; i++) for (int j = 0; j < DRAWING_SIZE; j++) drawing_surface_values[j][i] = test_set[number_row][(i * 28) + j + 1];
                        update_draw_surface = true;
                        update_network      = true;
                    }
                    // seven button
                    if(in_button_bounds(seven_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, seven_button, "7", bebas_neue);
                        int number_row = (rand() / (double) RAND_MAX) * (test_dim.row / 2.0);
                        while(test_set[number_row][0] != 7.0) number_row++;
                        for (int i = 0; i < DRAWING_SIZE; i++) for (int j = 0; j < DRAWING_SIZE; j++) drawing_surface_values[j][i] = test_set[number_row][(i * 28) + j + 1];
                        update_draw_surface = true;
                        update_network      = true;
                    }
                    // six button
                    if(in_button_bounds(six_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, six_button, "6", bebas_neue);
                        int number_row = (rand() / (double) RAND_MAX) * (test_dim.row / 2.0);
                        while(test_set[number_row][0] != 6.0) number_row++;
                        for (int i = 0; i < DRAWING_SIZE; i++) for (int j = 0; j < DRAWING_SIZE; j++) drawing_surface_values[j][i] = test_set[number_row][(i * 28) + j + 1];
                        update_draw_surface = true;
                        update_network      = true;
                    }
                    // five button
                    if(in_button_bounds(five_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, five_button, "5", bebas_neue);
                        int number_row = (rand() / (double) RAND_MAX) * (test_dim.row / 2.0);
                        while(test_set[number_row][0] != 5.0) number_row++;
                        for (int i = 0; i < DRAWING_SIZE; i++) for (int j = 0; j < DRAWING_SIZE; j++) drawing_surface_values[j][i] = test_set[number_row][(i * 28) + j + 1];
                        update_draw_surface = true;
                        update_network      = true;
                    }
                    // four button
                    if(in_button_bounds(four_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, four_button, "4", bebas_neue);
                        int number_row = (rand() / (double) RAND_MAX) * (test_dim.row / 2.0);
                        while(test_set[number_row][0] != 4.0) number_row++;
                        for (int i = 0; i < DRAWING_SIZE; i++) for (int j = 0; j < DRAWING_SIZE; j++) drawing_surface_values[j][i] = test_set[number_row][(i * 28) + j + 1];
                        update_draw_surface = true;
                        update_network      = true;
                    }
                    // three button
                    if(in_button_bounds(three_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, three_button, "3", bebas_neue);
                        int number_row = (rand() / (double) RAND_MAX) * (test_dim.row / 2.0);
                        while(test_set[number_row][0] != 3.0) number_row++;
                        for (int i = 0; i < DRAWING_SIZE; i++) for (int j = 0; j < DRAWING_SIZE; j++) drawing_surface_values[j][i] = test_set[number_row][(i * 28) + j + 1];
                        update_draw_surface = true;
                        update_network      = true;
                    }
                    // two button
                    if(in_button_bounds(two_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, two_button, "2", bebas_neue);
                        int number_row = (rand() / (double) RAND_MAX) * (test_dim.row / 2.0);
                        while(test_set[number_row][0] != 2.0) number_row++;
                        for (int i = 0; i < DRAWING_SIZE; i++) for (int j = 0; j < DRAWING_SIZE; j++) drawing_surface_values[j][i] = test_set[number_row][(i * 28) + j + 1];
                        update_draw_surface = true;
                        update_network      = true;
                    }
                    // one button
                    if(in_button_bounds(one_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, one_button, "1", bebas_neue);
                        int number_row = (rand() / (double) RAND_MAX) * (test_dim.row / 2.0);
                        while(test_set[number_row][0] != 1.0) number_row++;
                        for (int i = 0; i < DRAWING_SIZE; i++) for (int j = 0; j < DRAWING_SIZE; j++) drawing_surface_values[j][i] = test_set[number_row][(i * 28) + j + 1];
                        update_draw_surface = true;
                        update_network      = true;
                    }
                    // zero button
                    if(in_button_bounds(zero_button, mouse_x, mouse_y))
                    {
                        render_button_click(renderer, zero_button,  "0", bebas_neue);
                        int number_row = (rand() / (double) RAND_MAX) * (test_dim.row / 2.0);
                        while(test_set[number_row][0] != 0.0) number_row++;
                        for (int i = 0; i < DRAWING_SIZE; i++) for (int j = 0; j < DRAWING_SIZE; j++) drawing_surface_values[j][i] = test_set[number_row][(i * 28) + j + 1];
                        update_draw_surface = true;
                        update_network      = true;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    drawing = false;
                    break;
                case SDL_MOUSEMOTION:
                    prev_mouse_x = mouse_x;
                    prev_mouse_y = mouse_y;

                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    if(drawing && in_button_bounds(draw_input, mouse_x, mouse_y))
                    {
                        SDL_GetMouseState(&mouse_x, &mouse_y);
                        x_draw = ((double) mouse_x - window_width + draw_input.w) / draw_input.w * DRAWING_SIZE;
                        y_draw = ((double) mouse_y) / draw_input.h * DRAWING_SIZE;

                        drawing_surface_values[(int) x_draw][(int) y_draw] = 255;
                        if(abs(mouse_x - prev_mouse_x) > abs(mouse_y - prev_mouse_y))
                        {
                            if((int) y_draw > 0  && drawing_surface_values[(int) x_draw][(int) y_draw - 1] < 1) drawing_surface_values[(int) x_draw][(int) y_draw - 1] = 127;
                            if((int) y_draw < 27 && drawing_surface_values[(int) x_draw][(int) y_draw + 1] < 1) drawing_surface_values[(int) x_draw][(int) y_draw + 1] = 127;
                        } else {
                            if((int) x_draw > 0  && drawing_surface_values[(int) x_draw - 1][(int) y_draw] < 1) drawing_surface_values[(int) x_draw - 1][(int) y_draw] = 127;
                            if((int) x_draw < 27 && drawing_surface_values[(int) x_draw + 1][(int) y_draw] < 1) drawing_surface_values[(int) x_draw + 1][(int) y_draw] = 127;
                        }

                        update_draw_surface = true;
                        update_network      = true;
                    }
                    break;
                case SDL_WINDOWEVENT:
                    if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                    {
                        SDL_GetWindowSize(window, &window_width, &window_height);
                        update_screen_size = true;
                    }
                    break;
            }
        }

        if(update_network)
        {
            activate_draw_array(network, drawing_surface_values, DRAWING_SIZE, DRAWING_SIZE);
            render_network(renderer, network, neuron, window_width - draw_input.w, window_height);
            SDL_RenderPresent(renderer);
            update_network = false;
        }

        if(update_draw_surface)
        {
            render_drawing_surface(renderer, draw_input, drawing_surface_values);
            SDL_RenderPresent(renderer);
            update_draw_surface = false;
        }

        if(update_screen_size)
        {
            // update draw input size
            draw_input.x = window_width - min((window_width / 3), (window_height / 3));
            draw_input.y = 0;
            draw_input.w = min(window_height / 3, window_width / 3);
            draw_input.h = min(window_height / 3, window_width / 3);

            // update clear button size
            clear_button.x = draw_input.x;
            clear_button.y = draw_input.y + draw_input.h;
            clear_button.w = draw_input.w;
            clear_button.h = window_height / 10;

            // update train button size
            train_button.x = draw_input.x;
            train_button.y = draw_input.y + draw_input.h + clear_button.h;
            train_button.w = draw_input.w;
            train_button.h = window_height / 10;

            // update test button size
            test_button.x = draw_input.x;
            test_button.y = draw_input.y + draw_input.h + clear_button.h + train_button.h;
            test_button.w = draw_input.w;
            test_button.h = window_height / 10;

            // update print button size
            print_button.x = draw_input.x;
            print_button.y = draw_input.y + draw_input.h + clear_button.h + train_button.h + test_button.h;
            print_button.w = draw_input.w;
            print_button.h = window_height / 10;

            // update reset button size
            reset_button.x = draw_input.x;
            reset_button.y = draw_input.y + draw_input.h + clear_button.h + train_button.h + test_button.h + print_button.h;
            reset_button.w = draw_input.w;
            reset_button.h = window_height / 10;

            // update nine button size
            nine_button.x = window_width  - min((window_width / 12), (window_height / 12));
            nine_button.y = window_height - min((window_width / 12), (window_height / 12));
            nine_button.w = min((window_width / 12), (window_height / 12));
            nine_button.h = min((window_width / 12), (window_height / 12));

            // update eight button size
            eight_button.x = nine_button.x - nine_button.w;
            eight_button.y = nine_button.y;
            eight_button.w = nine_button.w;
            eight_button.h = nine_button.h;

            // update seven button size
            seven_button.x = nine_button.x - (2 * nine_button.w);
            seven_button.y = nine_button.y;
            seven_button.w = nine_button.w;
            seven_button.h = nine_button.h;

            // update six button size
            six_button.x = nine_button.x - (3 * nine_button.w);
            six_button.y = nine_button.y;
            six_button.w = nine_button.w;
            six_button.h = nine_button.h;
            
            // update five button size
            five_button.x = nine_button.x - (4 * nine_button.w);
            five_button.y = nine_button.y;
            five_button.w = nine_button.w;
            five_button.h = nine_button.h;
            
            // update four button size
            four_button.x = nine_button.x - (5 * nine_button.w);
            four_button.y = nine_button.y;
            four_button.w = nine_button.w;
            four_button.h = nine_button.h;

            // update three button size
            three_button.x = nine_button.x - (6 * nine_button.w);
            three_button.y = nine_button.y;
            three_button.w = nine_button.w;
            three_button.h = nine_button.h;

            // update two button size
            two_button.x = nine_button.x - (7 * nine_button.w);
            two_button.y = nine_button.y;
            two_button.w = nine_button.w;
            two_button.h = nine_button.h;

            // update one button size
            one_button.x = nine_button.x - (8 * nine_button.w);
            one_button.y = nine_button.y;
            one_button.w = nine_button.w;
            one_button.h = nine_button.h;

            // update zero button size
            zero_button.x = nine_button.x - (9 * nine_button.w);
            zero_button.y = nine_button.y;
            zero_button.w = nine_button.w;
            zero_button.h = nine_button.h;

            // clear the screen
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            // render the network
            activate_draw_array(network, drawing_surface_values, DRAWING_SIZE, DRAWING_SIZE);
            render_network(renderer, network, neuron, window_width - draw_input.w, window_height);

            // render drawing surface
            render_drawing_surface(renderer, draw_input, drawing_surface_values);

            // render action button
            render_button(renderer, clear_button, "clear", bebas_neue);
            render_button(renderer, train_button, "train", bebas_neue);
            render_button(renderer, test_button,  "test",  bebas_neue);
            render_button(renderer, print_button, "print", bebas_neue);
            render_button(renderer, reset_button, "reset", bebas_neue);

            // render number buttons
            render_button(renderer, nine_button,  "9", bebas_neue);
            render_button(renderer, eight_button, "8", bebas_neue);
            render_button(renderer, seven_button, "7", bebas_neue);
            render_button(renderer, six_button,   "6", bebas_neue);
            render_button(renderer, five_button,  "5", bebas_neue);
            render_button(renderer, four_button,  "4", bebas_neue);
            render_button(renderer, three_button, "3", bebas_neue);
            render_button(renderer, two_button,   "2", bebas_neue);
            render_button(renderer, one_button,   "1", bebas_neue);
            render_button(renderer, zero_button,  "0", bebas_neue);

            // present rendering
            SDL_RenderPresent(renderer);

            update_screen_size = false;
        }
    }

    // save network to file
    save_network_to_file(network);

    // free ttf
    TTF_Quit();
    TTF_CloseFont(bebas_neue);

    // free sdl
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(neuron);
    SDL_Quit();

    // free drawing data
    free_draw_array(drawing_surface_values, draw_dim);

    // free data
    free_frame(train_set, train_dim);
    free_frame(test_set, test_dim);
    free_charnode(features);
    free_network(network);

    return EXIT_SUCCESS;
}