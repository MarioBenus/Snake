#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "pipe.h"

#include "server.h"
#include "client.h"
#include "common.h"

#define SLEEP_LENGTH 150000

int main() {
    while (1)
    {
        printf("1) Create a new game\n");
        printf("2) Join game\n");
        printf("3) Exit\n\n");

        char a;
        //scanf("%s", &a); // disabled for testing
        if (a == '1' || 1 /* for testing*/) // CREATE GAME
        {
            while (1)
            {
                //printf("\nWrite a name for the server\n"); // Disabled for testing
                char b[11] = "SERVER";
                //scanf("%s", b);
                char* pipe_name_render = add_suffix(b, "RENDER");
                char* pipe_name_input = add_suffix(b, "INPUT");
                pipe_init(pipe_name_render);
                pipe_init(pipe_name_input);

                const pid_t pid = fork();
                if (pid == 0)
                    server(30, 20, b);
                else
                {
                    pthread_t render_thread;
                    pthread_t input_thread;
                    bool quit = false;
                    client_render_thread_data crtd = {30, 20, b, &quit};
                    pthread_create(&render_thread, NULL, client_render, &crtd);
                    client_input_thread_data citd = {&quit, b};
                    pthread_create(&input_thread, NULL, client_input, &citd);

                    pthread_join(render_thread, NULL);
                    pthread_join(input_thread, NULL);

                    pipe_destroy(pipe_name_render);
                    pipe_destroy(pipe_name_input);
                    free(pipe_name_input);
                    free(pipe_name_render);
                }
                return 0;
            }
        }
        else if (a == '2') // JOIN GAME
        {
            break; // TODO
        }
        else if (a == '3') // EXIT
            break;
    }
    

    
    return 0;
}


