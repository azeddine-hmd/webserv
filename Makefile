NAME = server

CC = c++

FLAGS = -Wall -Werror -Wextra -std=c++98

SRC =	src/main.cpp \
		src/mimeTypes.cpp \

OBJ = ${SRC:.cpp=.o}

CLIENT_SRC = src/client.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

all: $(NAME)

$(NAME):
	@$(CC) $(FLAGS) $(SRC) -o $(NAME)

client:
	@$(CC) $(FLAGS) $(CLIENT_SRC) -o client

clean:
	@rm -f $(OBJ) $(CLIENT_OBJ) $(NAME) client

test_config: tests/config.cpp
	@$(CC) $(FLAGS) $< -o tester

re: clean all

.PHONY: re clean