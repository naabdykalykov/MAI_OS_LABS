#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define SHM_NAME "/my_shared_memory"
#define SHM_SIZE 1024

int main() {
    char filename[256];
    printf("Введите имя файла: ");
    if (scanf("%255s", filename) != 1) {
        perror("Ошибка ввода имени файла");
        return 1;
    }

    // создание отображаемого файла (shared memory object)
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Ошибка при создании shared memory");
        return 1;
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("Ошибка при установке размера shared memory");
        return 1;
    }

    // отображение в память
    char* shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Ошибка при mmap");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка при fork");
        return 1;
    }

    if (pid == 0) {
        // дочерний процесс
        int input_fd = open(filename, O_RDONLY);
        if (input_fd == -1) {
            perror("Ошибка при открытии файла в дочернем процессе");
            exit(1);
        }

        dup2(input_fd, STDIN_FILENO); // stdin ← файл
        close(input_fd);

        execl("./child", "./child", SHM_NAME, NULL);
        perror("Ошибка при запуске дочернего процесса");
        exit(1);
    } else {
        // родительский процесс
        wait(NULL); // ждём завершения дочернего

        printf("Результаты из отображаемого файла:\n%s", shm_ptr);

        // очистка
        munmap(shm_ptr, SHM_SIZE);
        shm_unlink(SHM_NAME);
    }

    return 0;
}
