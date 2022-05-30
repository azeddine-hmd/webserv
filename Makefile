NAME = server

CC = c++
FLAGS = -Wall -Werror -Wextra
FLAGS_DEBUG = -Wall -Werror -Wextra -g3 -fsanitize=address

SRC = server.cpp
OBJ = ${SRC:.cpp=.o}

CLIENT_SRC = client.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

all: $(NAME)

$(NAME): server

server: $(SRC)
	@$(CC) $(FLAGS) -o server

client: $(CLIENT_SRC)
	@$(CC) $(FLAGS) -o client

clean:
	@rm -f $(OBJ) $(CLIENT_OBJ)

fclean: clean
	@rm -f server client

re: fclean all

.PHONY: re clean fclean