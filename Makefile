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
CFLAGS = -D_GNU_SOURCE -Wall -Wextra -Werror -O2 -I$(INC_DIR)
LIBS = -lX11 -lXtst -lm

ifdef UBUNTU
CFLAGS += -DUBUNTU
endif

PRELOAD_DIR = preload
PRELOAD_SRC = $(PRELOAD_DIR)/preload.c
PRELOAD_SO = $(PRELOAD_DIR)/preload.so

all: $(BIN_DIR)/$(NAME)

debug: CFLAGS += -DDEBUG
debug: fclean all

$(BIN_DIR)/$(NAME): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(PRELOAD_SO): $(PRELOAD_SRC)
	@mkdir -p $(PRELOAD_DIR)
	$(CC) -shared -fPIC -ldl -o $@ $<

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(BIN_DIR)/$(NAME) $(PRELOAD_SO)

re: fclean all

run: all
ifdef UBUNTU
	$(MAKE) $(PRELOAD_SO)
	LD_PRELOAD=$(PRELOAD_SO) ./$(BIN_DIR)/$(NAME)
else
	./$(BIN_DIR)/$(NAME)
endif
