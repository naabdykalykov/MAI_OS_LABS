#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef float (*func_float3)(float, float, float);
typedef float (*func_float2)(float, float);

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

bool validate_integral_params(float A, float B, float e) {
    if (e <= 0.0f) {
        printf("Ошибка: шаг 'e' должен быть > 0\n");
        return false;
    }
    if (A >= B) {
        printf("Ошибка: A должно быть меньше B\n");
        return false;
    }
    return true;
}

bool validate_derivative_params(float deltaX) {
    if (deltaX == 0.0f) {
        printf("Ошибка: deltaX не может быть нулём\n");
        return false;
    }
    return true;
}

int main() {
    int current_lib = 1;
    char lib_path[256];
    void *handle = NULL;
    func_float3 SinIntegral = NULL;
    func_float2 Derivative = NULL;
    
    while(1) {
        snprintf(lib_path, sizeof(lib_path), "./lib/libmath%d.so", current_lib);
        handle = dlopen(lib_path, RTLD_LAZY);
        
        if(!handle) {
            fprintf(stderr, "Ошибка загрузки %s: %s\n", lib_path, dlerror());
            return 1;
        }
        
        SinIntegral = (func_float3)dlsym(handle, "SinIntegral");
        Derivative = (func_float2)dlsym(handle, "Derivative");
        
        if(!SinIntegral || !Derivative) {
            fprintf(stderr, "Ошибка загрузки функций: %s\n", dlerror());
            dlclose(handle);
            return 1;
        }
        
        printf("Используется libmath%d.so\n", current_lib);
        printf("Команды:\n1 A B e - интеграл sin(x)\n2 A dx - производная cos(x)\n3 - переключить метод\n0 - выход\n> ");
        
        char cmd[256];
        while(fgets(cmd, sizeof(cmd), stdin)) {
            cmd[strcspn(cmd, "\n")] = '\0';
            
            if(cmd[0] == '0') {
                dlclose(handle);
                printf("Выход...\n");
                return 0;
            }
            else if(cmd[0] == '3') {
                dlclose(handle);
                current_lib = (current_lib == 1) ? 2 : 1;
                printf("Переключено на libmath%d.so\n", current_lib);
                break;
            }
            else if(cmd[0] == '1') {
                float params[3];
                char* token = strtok(cmd + 1, " ");
                int i = 0;
                bool valid = true;
                
                while (token && i < 3) {
                    if (!is_number(token)) {
                        printf("Ошибка: '%s' не является числом\n", token);
                        valid = false;
                        break;
                    }
                    params[i++] = strtof(token, NULL);
                    token = strtok(NULL, " ");
                }
                
                if (valid && i == 3) {
                    if (validate_integral_params(params[0], params[1], params[2])) {
                        printf("Результат интеграла: %f\n", SinIntegral(params[0], params[1], params[2]));
                    }
                } else if (i != 3) {
                    printf("Ошибка! Требуется 3 числа: 1 A B e\n");
                }
            }
            else if(cmd[0] == '2') {
                float params[2];
                char* token = strtok(cmd + 1, " ");
                int i = 0;
                bool valid = true;
                
                while (token && i < 2) {
                    if (!is_number(token)) {
                        printf("Ошибка: '%s' не является числом\n", token);
                        valid = false;
                        break;
                    }
                    params[i++] = strtof(token, NULL);
                    token = strtok(NULL, " ");
                }
                
                if (valid && i == 2) {
                    if (validate_derivative_params(params[1])) {
                        printf("Результат производной: %f\n", Derivative(params[0], params[1]));
                    }
                } else if (i != 2) {
                    printf("Ошибка! Требуется 2 числа: 2 A dx\n");
                }
            }
            else {
                printf("Неизвестная команда! Доступные: -1/0/1/2\n");
            }
            printf("> ");
        }
    }
    
    return 0;
}