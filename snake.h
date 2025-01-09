#ifndef SNAKE_SNAKE
#define SNAKE_SNAKE

#include "sll.h"

typedef struct coordinates
{
    int pos_x;
    int pos_y;
} coordinates;

typedef struct snake_node
{
    coordinates position;
    char symbol;
} snake_node;


void snake_init(sll* single_linked_list);
void snake_move(void* data, void* in, void* out, void* err);
void snake_draw_node_on_board(void* data, void* in, void* out, void* err);

#endif
