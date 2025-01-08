#include "snake.h"

void snake_init(sll* single_linked_list)
{
    sll_init(single_linked_list, sizeof(snake_node));
    snake_node sn1 = {1, 3, 'O'}; // TODO: change default starting point?
    snake_node sn2 = {2, 3, 'O'};
    snake_node sn3 = {3, 3, 'O'};
    sll_add(single_linked_list, &sn3); // snake is facing right
    sll_add(single_linked_list, &sn2);
    sll_add(single_linked_list, &sn1);
}

void snake_move(sll* single_linked_list, char direction)
{

}

// doesn't validate if snake is inside board
void snake_draw_node_on_board(void* data, void* in, void* out, void* err)
{
    snake_node* sn = data;
    char (*board)[22][23] = (char (*)[22][23])in;
    board[0][sn->pos_x + 1][sn->pos_y + 1] = sn->symbol;
}