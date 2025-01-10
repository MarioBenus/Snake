#ifndef SNAKE_SERVER
#define SNAKE_SERVER

#include "snake.h"

coordinates server_place_apple(size_t game_width, size_t game_height, char (*board)[game_height + 3]);
void server(size_t game_width, size_t game_height, char* server_name);

#endif