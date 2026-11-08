CC := gcc
CFLAGS := -std=c11 -Wall -Wextra
BIN_DIR := bin
SRC_DIR := src

PROGRAMS := client local root nation other edu gov ptr

.PHONY: all clean

all: $(PROGRAMS:%=$(BIN_DIR)/%)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/client: $(SRC_DIR)/Client.c $(SRC_DIR)/DNS.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/local: $(SRC_DIR)/local.c $(SRC_DIR)/DNS.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/root: $(SRC_DIR)/Root.c $(SRC_DIR)/othersvr.c $(SRC_DIR)/DNS.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/nation: $(SRC_DIR)/Nation.c $(SRC_DIR)/othersvr.c $(SRC_DIR)/DNS.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/other: $(SRC_DIR)/Other.c $(SRC_DIR)/othersvr.c $(SRC_DIR)/DNS.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/edu: $(SRC_DIR)/Edu.c $(SRC_DIR)/othersvr.c $(SRC_DIR)/DNS.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/gov: $(SRC_DIR)/Gov.c $(SRC_DIR)/othersvr.c $(SRC_DIR)/DNS.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/ptr: $(SRC_DIR)/PTR.c $(SRC_DIR)/othersvr.c $(SRC_DIR)/DNS.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) -r $(BIN_DIR)
