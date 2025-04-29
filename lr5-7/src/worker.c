#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include <unistd.h>
#include "common.h"

int find_substrings(const char *text, const char *pattern, char *result) {
    int n = strlen(text);
    int m = strlen(pattern);
    int found = 0;
    char temp[10];

    result[0] = '\0';

    for (int i = 0; i <= n - m; i++) {
        if (strncmp(&text[i], pattern, m) == 0) {
            if (found) strcat(result, ";");
            sprintf(temp, "%d", i);
            strcat(result, temp);
            found++;
        }
    }
    return found;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <id>\n", argv[0]);
        exit(1);
    }

    int id = atoi(argv[1]);

    void *context = zmq_ctx_new(); //создает новый контекст для работы с сокетами.
    void *socket = zmq_socket(context, ZMQ_REP); 
    //создает новый сокет, используя ранее созданный контекст context
    //ZMQ_REP.обработка запросов и отправка ответов

    char endpoint[256];
    sprintf(endpoint, "tcp://*:%d", PORT_BASE + id);
    zmq_bind(socket, endpoint);

    printf("Worker %d started.\n", id);

    while (1) {
        char buffer[1024];
        zmq_recv(socket, buffer, sizeof(buffer), 0); //для получения сообщения
        buffer[1023] = '\0';

        int cmd;
        sscanf(buffer, "%d", &cmd);

        if (cmd == CMD_EXEC) {
            char *text = strchr(buffer, ' ') + 1;
            char *pattern = strchr(text, ' ') + 1;
            *(pattern - 1) = '\0'; // разделяем строки
            char result[256];
            find_substrings(text, pattern, result);

            zmq_send(socket, result, strlen(result), 0);
        } else if (cmd == CMD_PING) {
            zmq_send(socket, "PONG", 4, 0);
        } else {
            zmq_send(socket, "Unknown command", 15, 0);
        }
    }

    zmq_close(socket);//закрываем сокет
    zmq_ctx_destroy(context);//уничтожается контекст(удаление всех ресусров, контекст очень важен)
    return 0;
}
