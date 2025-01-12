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
        scanf("%s", &a); 
        if (a == '1') // CREATE GAME
        {
            while (1)
            {
                printf("\nWrite a name for the server\n");
                char b[20];
                scanf("%s", b);

                const pid_t pid = fork();
                if (pid == 0)
                    server(30, 20, b);
                else
                {
                    sleep(1);
                    client_join(b);
                }
                return 0;
            }
        }
        else if (a == '2') // JOIN GAME
        {
            printf("\nWrite a name for the server\n");
            char b[20];
            scanf("%s", b);
            client_join(b);
            break;
        }
        else if (a == '3') // EXIT
            break;
    }
    

    
    return 0;
}


