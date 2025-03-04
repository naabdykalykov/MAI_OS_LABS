#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main() {
    int pipe1[2];
    pid_t pid;
    char filename[32];

    if (pipe(pipe1) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    printf("Введите имя файла: ");
    scanf("%s", filename);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(pipe1[0]);
        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        
        dup2(fd, STDIN_FILENO);
        close(fd);

        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);

        execl("./child", "child", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        close(pipe1[1]);
        char buffer[256];
        ssize_t count = read(pipe1[0], buffer, sizeof(buffer) - 1);
        if (count == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        buffer[count] = '\0';

        printf("Сумма чисел: %s", buffer);
        wait(NULL);
    }

    return 0;
}