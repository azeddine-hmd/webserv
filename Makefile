NAME = server

CC = c++
FLAGS = -Wall -Werror -Wextra
FLAGS_DEBUG = -Wall -Werror -Wextra -g3 -fsanitize=address
FLAGS_EMPTY =

SRC = server.cpp request/gnl.cpp
OBJ = ${SRC:.cpp=.o}

CLIENT_SRC = client.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

all: $(NAME)

$(NAME): $(SRC)
	@$(CC) $(FLAGS_EMPTY) $(SRC) -o $(NAME)

client: $(CLIENT_SRC)
	@$(CC) $(FLAGS_EMPTY) $(CLIENT_SRC) -o client

clean:
	@rm -f $(OBJ) $(CLIENT_OBJ) $(NAME) client

re: fclean all

.PHONY: re clean