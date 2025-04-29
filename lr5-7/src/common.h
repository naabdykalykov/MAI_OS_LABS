#ifndef COMMON_H
#define COMMON_H

#define PORT_BASE 5550 //базовый порт
#define MAX_WORKERS 128 //максимум рабочих узлов
#define PING_TIMEOUT 1000 // ms

//команды которые можем отправить узлам
typedef enum {
    CMD_EXEC,
    CMD_PING,
    CMD_PING_RESPONSE
} CommandType;

#endif
