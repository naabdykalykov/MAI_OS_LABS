#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

extern float SinIntegral(float A, float B, float e);
extern float Derivative(float A, float deltaX);

bool is_number(const char* str) {
    if (*str == '-' || *str == '+') str++;
    bool has_dot = false;
    while (*str) {
        if (*str == '.') {
            if (has_dot) return false;
            has_dot = true;
        } 
        else if (!isdigit(*str)) {
            return false;
        }
        str++;
    }
    return true;
}

int main() {
    char input[100];
    
    printf("Калькулятор:\n1 A B e - интеграл sin(x)\n2 A dx - производная cos(x)\n0 - выход\n");
    
    while(1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) {
            printf("Ошибка чтения ввода\n");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';
        
        if(input[0] == '1') {
            char* endptr;
            float params[3];
            char* token = strtok(input + 1, " ");
            int i = 0;
            bool valid = true;
            
            while (token && i < 3) {
                if (!is_number(token)) {
                    printf("Ошибка: '%s' не является числом\n", token);
                    valid = false;
                    break;
                }
                params[i++] = strtof(token, &endptr);
                token = strtok(NULL, " ");
            }
            
            if (valid && i == 3) {
                if (params[2] <= 0.0f) {
                    printf("Ошибка: шаг 'e' должен быть > 0\n");
                } 
                else if (params[0] >= params[1]) {
                    printf("Ошибка: A должно быть меньше B\n");
                }
                else {
                    printf("Результат: %f\n", SinIntegral(params[0], params[1], params[2]));
                }
            } 
            else if (i != 3) {
                printf("Ошибка! Нужно 3 числа: 1 A B e\n");
            }
        }
        else if(input[0] == '2') {
            char* endptr;
            float params[2];
            char* token = strtok(input + 1, " ");
            int i = 0;
            bool valid = true;
            
            while (token && i < 2) {
                if (!is_number(token)) {
                    printf("Ошибка: '%s' не является числом\n", token);
                    valid = false;
                    break;
                }
                params[i++] = strtof(token, &endptr);
                token = strtok(NULL, " ");
            }
            
            if (valid && i == 2) {
                if (params[1] == 0.0f) {
                    printf("Ошибка: deltaX не может быть нулём\n");
                }
                else {
                    printf("Результат: %f\n", Derivative(params[0], params[1]));
                }
            } 
            else if (i != 2) {
                printf("Ошибка! Нужно 2 числа: 2 A dx\n");
            }
        }
        else if(input[0] == '0') {
            printf("Выход...\n");
            break;
        }
        else {
            printf("Неизвестная команда! Доступно: 1/2/0\n");
        }
    }
    
    return 0;
}