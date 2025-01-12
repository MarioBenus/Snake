#include "server.h"

#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>

#include "pipe.h"
#include "sll.h"
#include "common.h"
#include "syn_sll.h"

#define SLEEP_LENGTH 150000

typedef struct user_data
{
    sll snake;
    int pid;
    size_t high_score;
    int fd_pipe_input; 
    int fd_pipe_render; 
    char last_input;
} user_data;

void user_exists(void* data, void* in, void* out, void* err)
{
    user_data* ud = data;
    int* pid = in;
    bool* result = out;
    if (ud->pid == *pid)
        *result = true;
}

typedef struct server_assign_id_thread_data
{
    char* server_name;
    syn_sll* sll_users;
    int* fd_pipe_assign_id;
} server_assign_id_thread_data;

void* server_assign_id(void* args)
{
    server_assign_id_thread_data* saitd = args;
    char* pipe_assign_id = add_suffix(saitd->server_name, "JOIN");
    pipe_init(pipe_assign_id);
    *saitd->fd_pipe_assign_id = pipe_open_read(pipe_assign_id);
    char buf[20];
    while (1)
    {   
        read(*saitd->fd_pipe_assign_id, buf, 20);

        int pid = atoi(buf);
        bool exists = false;
        syn_sll_for_each(saitd->sll_users, user_exists, &pid, &exists, NULL);
        if (exists)
        {
            usleep(500000);
            continue;
        }

        user_data* ud = malloc(sizeof(user_data));

        snake_init(&ud->snake);
        ud->pid = atoi(buf);
        ud->high_score = 0;
        ud->last_input = '\0';

        char* pipe_input_buf = add_suffix(saitd->server_name, "INPUT"); // TODO: mem leaks
        char* pipe_input = add_suffix(pipe_input_buf, buf);
        free(pipe_input_buf);
        pipe_init(pipe_input);
        ud->fd_pipe_input = pipe_open_read(pipe_input);

        char* pipe_render_buf = add_suffix(saitd->server_name, "RENDER");
        char* pipe_render = add_suffix(pipe_render_buf, buf);
        free(pipe_render_buf);
        pipe_init(pipe_render);
        ud->fd_pipe_render = pipe_open_write(pipe_render);

        syn_sll_add(saitd->sll_users, ud);
    }
}


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

typedef struct server_game_loop_data
{
    size_t game_width; 
    size_t game_height; 
    char* board;
} server_game_loop_data;


void server_game_loop(void* data, void* in, void* out, void* err)
{
    server_game_loop_data* sgld = in;
    char (*board)[sgld->game_width + 2][sgld->game_height + 3] = (char (*)[sgld->game_width + 2][sgld->game_height + 3])sgld->board;

    user_data* ud = data;
    char new_input;
    read(ud->fd_pipe_input, &new_input, 1);
    if ((new_input == 'q' || new_input == 'w' || new_input == 'a' || new_input == 's' || new_input == 'd') &&
        !(ud->last_input == 'w' && new_input == 's') && !(ud->last_input == 's' && new_input == 'w') &&
        !(ud->last_input == 'a' && new_input == 'd') && !(ud->last_input == 'd' && new_input == 'a'))
        ud->last_input = new_input;


    snake_node sn;
    sll_get(&ud->snake, 0, &sn);
    
    coordinates snake_pos = sn.position;
    
    // if (ud->last_input == 'q')
        // TODO: exit
    if (ud->last_input == 'a')
        snake_pos.pos_x -= 1;
    if (ud->last_input == 's')
        snake_pos.pos_y += 1;
    if (ud->last_input == 'w')
        snake_pos.pos_y -= 1;
    if (ud->last_input == 'd')
        snake_pos.pos_x += 1;

    if ((ud->last_input == 'w' || ud->last_input == 'a' || ud->last_input == 's' || ud->last_input == 'd') && sn.symbol == '*')
    {
        sll_for_each(&ud->snake, snake_start, NULL, NULL, NULL);
    }

    if (sn.symbol == 'X') // respawn
    {
        sll_for_each(&ud->snake, snake_undraw_node_from_board, board, NULL, NULL);
        sll_clear(&ud->snake);
        snake_init(&ud->snake);
        ud->last_input = '\0';
        snake_pos.pos_x = 1;
        snake_pos.pos_y = 1;
    }

    if (board[0][snake_pos.pos_x][snake_pos.pos_y] == '@') // ate an apple
    {
        snake_node sn2;
        sn2.symbol = 'O';
        sll_add(&ud->snake, &sn2);
        // apple_pos = server_place_apple(game_width, game_height, board);

    }
    else if (board[0][snake_pos.pos_x][snake_pos.pos_y] == '-' ||
             board[0][snake_pos.pos_x][snake_pos.pos_y] == '|' ||
             board[0][snake_pos.pos_x][snake_pos.pos_y] == 'O')
    {
        sll_for_each(&ud->snake, snake_death, NULL, NULL, NULL);
    }
    
    // board[apple_pos.pos_x][apple_pos.pos_y] = '@';

    sll_for_each(&ud->snake, snake_undraw_node_from_board, board, NULL, NULL);

    sll_get(&ud->snake, 0, &sn);
    if (sn.symbol != 'X')
        sll_for_each(&ud->snake, snake_move, &snake_pos, NULL, NULL);

    sll_for_each(&ud->snake, snake_draw_node_on_board, board, NULL, NULL);
}


void server_write(void* data, void* in, void* out, void* err)
{
    server_game_loop_data* sgld = in;
    char (*board)[sgld->game_width + 2][sgld->game_height + 3] = (char (*)[sgld->game_width + 2][sgld->game_height + 3])sgld->board;

    user_data* ud = data;

    for (size_t i = 0; i < sgld->game_width + 2; i++)
    {
        write(ud->fd_pipe_render, board[0][i], sizeof(char) * (sgld->game_height + 3));
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
    for (size_t i = 1; i < game_height + 1; i++)
        for (size_t j = 1; j < game_width + 1; j++)
            board[j][i] = ' ';
        
    



    server_assign_id_thread_data saitd;
    saitd.server_name = server_name;
    syn_sll ssll;
    syn_sll_init(&ssll, sizeof(user_data));
    saitd.sll_users = &ssll;
    int fd_pipe_assign_id;
    saitd.fd_pipe_assign_id = &fd_pipe_assign_id;

    pthread_t server_assign_id_thread;
    pthread_create(&server_assign_id_thread, NULL, server_assign_id, &saitd);

    // coordinates apple_pos = server_place_apple(game_width, game_height, board);

    server_game_loop_data sgld;
    sgld.board = board;
    sgld.game_height = game_height;
    sgld.game_width = game_width;

    while (1) // GAME LOOP
    {
        syn_sll_for_each(&ssll, server_game_loop, &sgld, NULL, NULL);
        syn_sll_for_each(&ssll, server_write, &sgld, NULL, NULL);


        usleep(SLEEP_LENGTH);
    }
    


    pipe_close(fd_pipe_assign_id);
    return;
}