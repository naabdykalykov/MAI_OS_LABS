# Компилятор
CC = gcc
# Флаги для динамических библиотек
CFLAGS = -shared -fPIC
# Папки
SRC_DIR = src
LIB_DIR = lib
BIN_DIR = bin

# Цели по умолчанию
all: libs programs

# Сборка библиотек
libs:
	mkdir -p $(LIB_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/mathlib1.c -o $(LIB_DIR)/libmath1.so -lm
	$(CC) $(CFLAGS) $(SRC_DIR)/mathlib2.c -o $(LIB_DIR)/libmath2.so -lm

# Сборка программ
programs:
	mkdir -p $(BIN_DIR)
	$(CC) $(SRC_DIR)/main_static.c -L$(LIB_DIR) -lmath1 -o $(BIN_DIR)/static_program -Wl,-rpath=$(LIB_DIR)
	$(CC) $(SRC_DIR)/main_dynamic.c -ldl -o $(BIN_DIR)/dynamic_program

# Очистка
clean:
	rm -rf $(LIB_DIR) $(BIN_DIR)