NAME = server

CC = c++

DEBUG = -g3 -Wno-unused-variable -Wno-unused-parameter #-fsanitize=address
FLAGS = -Wall -Werror -Wextra $(DEBUG) #-std=c++98


FLAGS_EMPTY =

SRC =	src/main.cpp \
		src/mimeTypes.cpp \

OBJ = ${SRC:.cpp=.o}

CLIENT_SRC = src/client.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

all: $(NAME)

$(NAME): clean
	@$(CC) $(FLAGS) $(SRC) -o $(NAME)

client: clean
	@$(CC) $(FLAGS) $(CLIENT_SRC) -o client

clean:
	@rm -f $(OBJ) $(CLIENT_OBJ) $(NAME) client

test_config: tests/config.cpp
	@$(CC) $(FLAGS) $< -o tester

re: clean all

.PHONY: re clean