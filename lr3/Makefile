CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lrt

SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build

PARENT_SRC = $(SRC_DIR)/parent.c
CHILD_SRC = $(SRC_DIR)/child.c

PARENT_OBJ = $(BUILD_DIR)/parent.o
CHILD_OBJ = $(BUILD_DIR)/child.o

PARENT_BIN = $(BIN_DIR)/parent
CHILD_BIN = $(BIN_DIR)/child

all: dirs $(PARENT_BIN) $(CHILD_BIN)

dirs:
	mkdir -p $(BIN_DIR) $(BUILD_DIR)

$(PARENT_BIN): $(PARENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CHILD_BIN): $(CHILD_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)

.PHONY: all clean dirs
