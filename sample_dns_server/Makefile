CC=/usr/bin/gcc
CWD=.
INC_DIR=$(CWD)/src/include
SRC_DIR=$(CWD)/src
SUB_DIR_NAME=dns_server
UTILS_DIR=dns_utils
BIN_DIR=$(CWD)/bin
OBJ_DIR=$(CWD)/obj

FLAGS=-I$(INC_DIR) -DDEBUG

.PHONY: clean
	@mkdir $(BIN_DIR)
	@mkdir $(OBJ_DIR)

all: .PHONY dns_utils
	$(CC) $(FLAGS) $(SRC_DIR)/$(SUB_DIR_NAME)/server.c $(OBJ_DIR)/$(UTILS_DIR).o -o $(BIN_DIR)/server -levent

dns_utils:
	$(CC) $(FLAGS) -c $(SRC_DIR)/$(UTILS_DIR)/utils.c -o $(OBJ_DIR)/$@.o

clean:
	@rm -rf $(BIN_DIR)
	@rm -rf $(OBJ_DIR)

