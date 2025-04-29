#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Не передано имя отображаемого файла\n");
        return 1;
    }

    const char* shm_name = argv[1];

    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Ошибка при открытии shared memory");
        return 1;
    }

    char* shm_ptr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Ошибка при mmap в дочернем процессе");
        return 1;
    }

    char line[256];
    char result[SHM_SIZE] = "";

    while (fgets(line, sizeof(line), stdin)) {
        int sum = 0;
        char* token = strtok(line, " \t\n");
        while (token != NULL) {
            sum += atoi(token);
            token = strtok(NULL, " \t\n");
        }
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%d\n", sum);
        strncat(result, buffer, sizeof(result) - strlen(result) - 1);
    }

    strncpy(shm_ptr, result, SHM_SIZE - 1);
    munmap(shm_ptr, SHM_SIZE);

    return 0;
}
