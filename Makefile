NAME = snakebot

SRC_DIR = srcs
INC_DIR = includes
OBJ_DIR = objs
BIN_DIR = bin

SRCS = main.c recorder.c bot.c
SRC = $(addprefix $(SRC_DIR)/, $(SRCS))

OBJ = $(SRCS:.c=.o)
OBJS = $(addprefix $(OBJ_DIR)/, $(OBJ))

CC = cc
CFLAGS = -Wall -Wextra -Werror -O2 -I$(INC_DIR)

all: $(BIN_DIR)/$(NAME)

$(BIN_DIR)/$(NAME): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(BIN_DIR)/$(NAME)

re: fclean all

run: all
	stdbuf -o0 -e0 ./$(BIN_DIR)/minisnake | ./$(BIN_DIR)/$(NAME)
