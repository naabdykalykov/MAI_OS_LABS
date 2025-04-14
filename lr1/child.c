#include <stdio.h>
#include <stdlib.h>

int main() {
    int sum = 0;
    int number;

    while (scanf("%d", &number) == 1) {
        sum += number;
    }

    printf("%d\n", sum);

    return 0;
}