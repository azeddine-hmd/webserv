NAME = server

CC = c++

UNUSED_FLAGS = -Wno-unused-variable -Wno-unused-parameter
FLAGS = -Wall -Werror -Wextra
FLAGS_DEBUG = -Wall -Werror -Wextra -g3 -fsanitize=address


FLAGS_EMPTY =

SRC =	src/main.cpp \

OBJ = ${SRC:.cpp=.o}

CLIENT_SRC = src/client.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

all: $(NAME)

$(NAME): clean
	@$(CC) $(FLAGS_DEBUG) $(SRC) -o $(NAME)

client: clean
	@$(CC) $(FLAGS) $(CLIENT_SRC) -o client

clean:
	@rm -f $(OBJ) $(CLIENT_OBJ) $(NAME) client

test_config: tests/config.cpp
	@$(CC) $(FLAGS_DEBUG_UNUSED) $< -o tester

re: clean all

.PHONY: re clean