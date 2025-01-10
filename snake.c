#include "snake.h"

void snake_init(sll* single_linked_list)
{
    sll_init(single_linked_list, sizeof(snake_node));
    coordinates pos1 = {1, 1};
    coordinates pos2 = {1, 1};
    coordinates pos3 = {1, 1};
    snake_node sn1 = {pos1, 'O'};
    snake_node sn2 = {pos2, 'O'};
    snake_node sn3 = {pos3, 'O'};
    sll_add(single_linked_list, &sn3);
    sll_add(single_linked_list, &sn2);
    sll_add(single_linked_list, &sn1);
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