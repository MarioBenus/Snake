#include "snake.h"

void snake_init(sll* single_linked_list)
{
    sll_init(single_linked_list, sizeof(snake_node));
    coordinates pos1 = {1, 2};
    coordinates pos2 = {2, 2};
    coordinates pos3 = {3, 2};
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
    char (*board)[22][23] = (char (*)[22][23])in;
    board[0][sn->position.pos_x + 1][sn->position.pos_y + 1] = sn->symbol;
}