NAME = server

CC = c++
FLAGS = -Wall -Werror -Wextra
FLAGS_DEBUG = -Wall -Werror -Wextra -g3 -fsanitize=address
FLAGS_EMPTY =

SRC = src/server.cpp src/gnl.cpp src/mimeTypes.cpp
OBJ = ${SRC:.cpp=.o}

all: $(NAME)

$(NAME): $(SRC)
	@$(CC) $(FLAGS_EMPTY) $(SRC) -o $(NAME)

clean:
	@rm -f $(OBJ) $(CLIENT_OBJ) $(NAME) client

re: fclean all

.PHONY: re clean