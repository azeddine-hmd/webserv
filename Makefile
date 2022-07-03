NAME = webserv

CC = c++

FLAGS = -Wall -Werror -Wextra -std=c++98

SRC =	src/main.cpp \
		src/mimeTypes.cpp \

OBJ = ${SRC:.cpp=.o}

all: $(NAME)

$(NAME):
	@$(CC) $(FLAGS) $(SRC) -o $(NAME)

clean:
	@rm -f $(OBJ) $(NAME)

re: clean all

.PHONY: re clean