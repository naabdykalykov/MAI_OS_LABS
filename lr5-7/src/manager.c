#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include <unistd.h>
#include <sys/wait.h>
#include "common.h"

typedef struct Node {
    int id;
    struct Node *left;
    struct Node *right;
} Node;

Node *root = NULL;
Node* nodes[MAX_WORKERS] = {0};
void *context;

int get_size(Node* node) {
    if (!node) return 0;
    return 1 + get_size(node->left) + get_size(node->right);
}

void add_node(int id) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->id = id;
    new_node->left = NULL;
    new_node->right = NULL;
    nodes[id] = new_node;

    if (!root) {
        root = new_node;
        return;
    }

    Node* current = root;
    while (1) {
        int left_size = get_size(current->left);
        int right_size = get_size(current->right);

        if (left_size <= right_size) {
            if (!current->left) {
                current->left = new_node;
                break;
            }
            current = current->left;
        } else {
            if (!current->right) {
                current->right = new_node;
                break;
            }
            current = current->right;
        }
    }
}

void start_worker(int id) {
    pid_t pid = fork();
    if (pid == 0) {
        char id_str[10];
        sprintf(id_str, "%d", id);
        execl("./worker", "./worker", id_str, NULL);
        perror("execl");
        exit(1);
    }
}

void print_tree(Node* node, int level) {
    if (!node) return;
    print_tree(node->right, level + 1);
    for (int i = 0; i < level; i++) printf("    ");
    printf("%d\n", node->id);
    print_tree(node->left, level + 1);
}

void send_command(int id, CommandType cmd, const char *payload) {
    void *socket = zmq_socket(context, ZMQ_REQ);
    //создание сокета (для запроса), rep - для ответа
    char endpoint[256];
    sprintf(endpoint, "tcp://localhost:%d", PORT_BASE + id);
    zmq_connect(socket, endpoint);
    //ддля подключения сокета по адресу

    int timeout = 1000; // 1 секунда
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    //для времени ожидания

    char message[512];
    if (payload)
        sprintf(message, "%d %s", cmd, payload);
    else
        sprintf(message, "%d", cmd);

    zmq_send(socket, message, strlen(message), 0);
    //отправка сообщения, 0 -обычная отправка

    char buffer[1024];
    int rc = zmq_recv(socket, buffer, 1024, 0);
    //получение сообщения через сокет
    if (rc == -1) {
        printf("Node %d is unavailable.\n", id);
    } else {
        buffer[rc] = '\0';
        printf("Ok:%d: %s\n", id, buffer); 
    }

    zmq_close(socket);
    //закрываем сокет
}

void pingall() {
    int unavailable[MAX_WORKERS] = {0};
    int failed_count = 0;

    for (int id = 1; id < MAX_WORKERS; id++) {
        if (nodes[id]) {
            void *socket = zmq_socket(context, ZMQ_REQ);
            char endpoint[256];
            sprintf(endpoint, "tcp://localhost:%d", PORT_BASE + id);
            zmq_connect(socket, endpoint);

            int timeout = 500;
            zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

            zmq_send(socket, "1", 1, 0); // CMD_PING

            char buffer[256];
            int rc = zmq_recv(socket, buffer, 256, 0);
            if (rc == -1) {
                unavailable[failed_count++] = id;
            }

            zmq_close(socket);
        }
    }

    if (failed_count == 0) {
        printf("Ok: -1\n");
    } else {
        printf("Ok: ");
        for (int i = 0; i < failed_count; i++) {
            printf("%d", unavailable[i]);
            if (i < failed_count - 1)
                printf(";");
        }
        printf("\n");
    }
}

int main() {
    context = zmq_ctx_new();

    printf("Manager started.\n");

    char command[1024];
    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(command, sizeof(command), stdin))
            break;
        command[strcspn(command, "\n")] = 0;

        if (strncmp(command, "create", 6) == 0) {
            int id;
            if (sscanf(command + 7, "%d", &id) == 1) {
                add_node(id);
                printf("Tree structure:\n");
                print_tree(root, 0);
                start_worker(id);
                printf("Node %d created.\n", id);
            } else {
                printf("Invalid create command.\n");
            }
        } else if (strncmp(command, "exec", 4) == 0) {
            int id;
            char text[128], pattern[128];
            if (sscanf(command + 5, "%d %s %s", &id, text, pattern) == 3) {
                char payload[300];
                snprintf(payload, sizeof(payload), "%s %s", text, pattern);
                send_command(id, CMD_EXEC, payload);
            } else {
                printf("Usage: exec <id> <text> <pattern>\n");
            }
        } else if (strncmp(command, "pingall", 7) == 0) {
            pingall();
        } else if (strncmp(command, "exit", 4) == 0) {
            break;
        } else {
            printf("Unknown command.\n");
        }
    }

    zmq_ctx_destroy(context);
    return 0;
}
