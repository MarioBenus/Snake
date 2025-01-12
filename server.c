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

void server_place_apple(size_t game_width, size_t game_height, char (*board)[game_height + 3])
{
    srand(time(NULL));
    while (1)
    {
        int x = rand() % game_width + 1;
        int y = rand() % game_height + 1;

        if (board[x][y] == ' ')
        {
            printf("trying to place apple at %d %d\n", x, y);
            board[x][y] = '@';
            return;
        }
    }
}

typedef struct user_data
{
    sll snake;
    int pid;
    size_t high_score;
    int fd_pipe_input; 
    int fd_pipe_render; 
    char last_input;
    bool quit;
    coordinates apple_coordinates;
} user_data;

void free_user_data_sll(void* data, void* in, void* out, void* err)
{
    user_data* ud = data;
    sll_clear(&ud->snake);
}

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
    bool* quit;
} server_assign_id_thread_data;

void* server_assign_id(void* args)
{
    server_assign_id_thread_data* saitd = args;
    char* pipe_assign_id = add_suffix(saitd->server_name, "JOIN");
    pipe_init(pipe_assign_id);
    *saitd->fd_pipe_assign_id = pipe_open_read(pipe_assign_id);
    free(pipe_assign_id);
    char buf[20];
    int previous_pid = -2;
    while (1)
    {   
        if (*saitd->quit)
            break;
        
        read(*saitd->fd_pipe_assign_id, buf, 20);

        int pid = atoi(buf);
        bool exists = false;
        syn_sll_for_each(saitd->sll_users, user_exists, &pid, &exists, NULL);
        if (exists || pid == previous_pid)
        {
            usleep(500000);
            continue;
        }

        previous_pid = pid;

        user_data ud;

        snake_init(&ud.snake);
        ud.pid = atoi(buf);
        ud.high_score = 0;
        ud.last_input = '\0';
        ud.quit = false;

        char* pipe_input_buf = add_suffix(saitd->server_name, "INPUT");
        char* pipe_input = add_suffix(pipe_input_buf, buf);
        free(pipe_input_buf);
        pipe_init(pipe_input);
        ud.fd_pipe_input = pipe_open_read(pipe_input);
        free(pipe_input);

        char* pipe_render_buf = add_suffix(saitd->server_name, "RENDER");
        char* pipe_render = add_suffix(pipe_render_buf, buf);
        free(pipe_render_buf);
        pipe_init(pipe_render);
        ud.fd_pipe_render = pipe_open_write(pipe_render);
        free(pipe_render);

        syn_sll_add(saitd->sll_users, &ud);
    }
}




typedef struct server_game_loop_data
{
    size_t game_width; 
    size_t game_height; 
    char* board;
    size_t* apple_count;
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
    
    if (ud->last_input == 'q')
        ud->quit = true;
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
        coordinates coor;
        coor.pos_x = 0;
        coor.pos_y = 0;
        sn2.position = coor;
        sll_add(&ud->snake, &sn2);
        *sgld->apple_count -= 1;

    }
    else if (board[0][snake_pos.pos_x][snake_pos.pos_y] == '-' ||
             board[0][snake_pos.pos_x][snake_pos.pos_y] == '|' ||
             board[0][snake_pos.pos_x][snake_pos.pos_y] == 'O')
    {
        sll_for_each(&ud->snake, snake_death, NULL, NULL, NULL);
    }
    

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

void server_test_quit_user(void* data, void* in, void* out, void* err)
{
    int* index = in;
    int* result = out;
    user_data* ud = data;
    if (ud->quit)
        *result = *index;

    (*index)++;
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
        
    

    size_t apple_count = 0;

    server_assign_id_thread_data saitd;
    saitd.server_name = server_name;
    syn_sll ssll;
    syn_sll_init(&ssll, sizeof(user_data));
    saitd.sll_users = &ssll;
    int fd_pipe_assign_id;
    saitd.fd_pipe_assign_id = &fd_pipe_assign_id;
    bool saitd_quit = false;
    saitd.quit = &saitd_quit;

    pthread_t server_assign_id_thread;
    pthread_create(&server_assign_id_thread, NULL, server_assign_id, &saitd);


    server_game_loop_data sgld;
    sgld.board = board;
    sgld.game_height = game_height;
    sgld.game_width = game_width;
    sgld.apple_count = &apple_count;

    int no_players_counter = 0;

    while (1) // GAME LOOP
    {
        size_t user_count = syn_sll_get_size(&ssll);

        while (apple_count < user_count)
        {
            server_place_apple(game_width, game_height, board);
            apple_count++;
        }

        syn_sll_for_each(&ssll, server_game_loop, &sgld, NULL, NULL);

        int zero = 0;
        int index = -1;
        syn_sll_for_each(&ssll, server_test_quit_user, &zero, &index, NULL);
        while (index != -1)
        {
            user_data ud;
            syn_sll_get(&ssll, index, &ud);

            pipe_close(ud.fd_pipe_input);
            pipe_close(ud.fd_pipe_render);

            char buf[20];
            sprintf(buf, "%d", ud.pid);

            char* pipe_input_buf = add_suffix(server_name, "INPUT");
            char* pipe_input = add_suffix(pipe_input_buf, buf);
            free(pipe_input_buf);
            pipe_destroy(pipe_input);
            free(pipe_input);

            char* pipe_render_buf = add_suffix(server_name, "RENDER");
            char* pipe_render = add_suffix(pipe_render_buf, buf);
            free(pipe_render_buf);
            pipe_destroy(pipe_render);
            free(pipe_render);

            sll_for_each(&ud.snake, snake_undraw_node_from_board, board, NULL, NULL);

            sll_clear(&ud.snake);
            syn_sll_remove(&ssll, index);

            zero = 0;
            index = -1;
            syn_sll_for_each(&ssll, server_test_quit_user, &zero, &index, NULL);
        }


        if (syn_sll_get_size(&ssll) == 0)
            no_players_counter++;
        else
            no_players_counter = 0;

        if (no_players_counter >= 50)
        {
            char* pipe_assign_id = add_suffix(server_name, "JOIN");
            pipe_close(fd_pipe_assign_id);
            pipe_destroy(pipe_assign_id);
            free(pipe_assign_id);
            syn_sll_for_each(&ssll, free_user_data_sll, NULL, NULL, NULL);
            syn_sll_clear(&ssll);
            saitd_quit = true;
            pthread_join(server_assign_id_thread, NULL);
            break;

        }
        
        syn_sll_for_each(&ssll, server_write, &sgld, NULL, NULL);

        usleep(SLEEP_LENGTH);
    }
    
    return;
}