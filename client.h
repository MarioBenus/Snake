#ifndef SNAKE_CLIENT
#define SNAKE_CLIENT

#include <stdbool.h>
#include <stddef.h>

typedef struct client_input_thread_data
{
    bool* quit;
    char* server_name;
} client_input_thread_data;

typedef struct client_render_thread_data
{
    size_t game_width; 
    size_t game_height; 
    char* server_name;
    bool* quit;
} client_render_thread_data;

void* client_input(void* args);
void* client_render(void* args);

#endif