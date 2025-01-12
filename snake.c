#include "snake.h"
#include <stdlib.h>

void snake_init(sll* single_linked_list)
{
    sll_init(single_linked_list, sizeof(snake_node));
    for (size_t i = 0; i < 3; i++)
    {
        snake_node* sn = malloc(sizeof(snake_node));
        sn->position.pos_x = 1;
        sn->position.pos_y = 1;
        sn->symbol = '*';
        sll_add(single_linked_list, sn);
    }
}

void snake_move(void* data, void* in, void* out, void* err)
{
    snake_node* sn = data;
    coordinates pos = *(coordinates*)in;
    *(coordinates*)in = sn->position;
    sn->position = pos;
}

// doesn't validate if snake is inside board
void snake_draw_node_on_board(void* data, void* in, void* out, void* err)
{
    snake_node* sn = data;
    char (*board)[32][23] = (char (*)[32][23])in; // TODO: make dynmic
    board[0][sn->position.pos_x][sn->position.pos_y] = sn->symbol;
}

void snake_death(void* data, void* in, void* out, void* err)
{
    snake_node* sn = data;
    sn->symbol = 'X';
}

void snake_start(void* data, void* in, void* out, void* err)
{
    snake_node* sn = data;
    sn->symbol = 'O';
}

void snake_undraw_node_from_board(void* data, void* in, void* out, void* err)
{
    snake_node* sn = data;
    char (*board)[32][23] = (char (*)[32][23])in; // TODO: make dynmic
    board[0][sn->position.pos_x][sn->position.pos_y] = ' ';
}
