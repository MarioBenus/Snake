#ifndef SNAKE_SNAKE
#define SNAKE_SNAKE

#include "sll.h"

typedef struct snake_node
{
    int pos_x;
    int pos_y;
    char symbol;
} snake_node;


void snake_init(sll* single_linked_list);
void snake_move(sll* single_linked_list, char direction);
void snake_draw_node_on_board(void* data, void* in, void* out, void* err);

#endif
