#include <stdlib.h>
#include <time.h>

#include "server.h"
#include "pipe.h"
#include "sll.h"
#include "common.h"

#define SLEEP_LENGTH 150000

coordinates server_place_apple(size_t game_width, size_t game_height, char (*board)[game_height + 3])
{
    srand(time(NULL));
    coordinates apple_pos;
    while (1)
    {
        int x = rand() % game_width + 1;
        int y = rand() % game_height + 1;

        if (board[x][y] == ' ')
        {
            apple_pos.pos_x = x;
            apple_pos.pos_y = y;
            return apple_pos;
        }
    }
}

void server(size_t game_width, size_t game_height, char* server_name)
{
    char board[game_width + 2][game_height + 3];
    
    for (size_t i = 0; i < game_width + 2; i++)
    {
        board[i][game_height + 2] = '\0';
    }
    

    // borders
    for (size_t i = 1; i < game_width + 1; i++)
        board[i][0] = '-';
    for (size_t i = 1; i < game_width + 1; i++)
        board[i][game_height + 1] = '-';
    for (size_t i = 1; i < game_height + 1; i++)
        board[0][i] = '|';
    for (size_t i = 1; i < game_height + 1; i++)
        board[game_width + 1][i] = '|';
    board[0][0] = '/';
    board[game_width + 1][game_height + 1] = '/';
    board[0][game_height + 1] = '\\';
    board[game_width + 1][0] = '\\';

    char* pipe_render = add_suffix(server_name, "RENDER");
    char* pipe_input = add_suffix(server_name, "INPUT");
    const int fd_pipe_board = pipe_open_write(pipe_render);
    const int fd_pipe_input = pipe_open_read(pipe_input);
    sll snake;
    snake_init(&snake);

    coordinates apple_pos = server_place_apple(game_width, game_height, board);

    char last_input = '\0';
    while (1) // GAME LOOP
    {
        char new_input;
        read(fd_pipe_input, &new_input, 1);
        if ((new_input == 'q' || new_input == 'w' || new_input == 'a' || new_input == 's' || new_input == 'd') &&
            !(last_input == 'w' && new_input == 's') && !(last_input == 's' && new_input == 'w') &&
            !(last_input == 'a' && new_input == 'd') && !(last_input == 'd' && new_input == 'a'))
            last_input = new_input;


        snake_node sn;
        sll_get(&snake, 0, &sn);
        
        coordinates snake_pos = sn.position;
        
        if (last_input == 'q')
            break;
        if (last_input == 'w')
            snake_pos.pos_y -= 1;
        if (last_input == 'a')
            snake_pos.pos_x -= 1;
        if (last_input == 's')
            snake_pos.pos_y += 1;
        if (last_input == 'd')
            snake_pos.pos_x += 1;

        if ((last_input == 'w' || last_input == 'a' || last_input == 's' || last_input == 'd') && sn.symbol == '*')
            sll_for_each(&snake, snake_start, NULL, NULL, NULL);

        if (sn.symbol == 'X')
        {
            sll_clear(&snake);
            snake_init(&snake);
            last_input = '\0';
            snake_pos.pos_x = 1;
            snake_pos.pos_y = 1;
        }

        if (board[snake_pos.pos_x][snake_pos.pos_y] == '@') // ate an apple
        {
            snake_node* sn2 = malloc(sizeof(snake_node));
            sn2->symbol = 'O';
            sll_add(&snake, sn2);
            apple_pos = server_place_apple(game_width, game_height, board);

        }
        else if (board[snake_pos.pos_x][snake_pos.pos_y] == '-' ||
                 board[snake_pos.pos_x][snake_pos.pos_y] == '|' ||
                 board[snake_pos.pos_x][snake_pos.pos_y] == 'O')
        {
            sll_for_each(&snake, snake_death, NULL, NULL, NULL);
        }

        for (size_t i = 1; i < game_height + 1; i++)
            for (size_t j = 1; j < game_width + 1; j++)
                board[j][i] = ' ';
        
        board[apple_pos.pos_x][apple_pos.pos_y] = '@';

        sll_get(&snake, 0, &sn);
        if (sn.symbol != 'X')
            sll_for_each(&snake, snake_move, &snake_pos, NULL, NULL);

        sll_for_each(&snake, snake_draw_node_on_board, board, NULL, NULL);

        for (size_t i = 0; i < game_width + 2; i++)
        {
            write(fd_pipe_board, board[i], sizeof(char) * (game_height + 3));
        }

        usleep(SLEEP_LENGTH);
    }
    

    free (pipe_input);
    free (pipe_render);
    sll_clear(&snake);
    pipe_close(fd_pipe_input);
    pipe_close(fd_pipe_board);
    return;
}