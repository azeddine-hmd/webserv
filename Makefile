NAME = server

CC = clang++

FLAGS = -Wall -Werror -Wextra
FLAGS_UNUSED = -Wall -Werror -Wextra -Wno-unused-variable -Wno-unused-parameter

FLAGS_DEBUG = -Wall -Werror -Wextra -g3 -fsanitize=address
FLAGS_DEBUG_UNUSED = -Wall -Werror -Wextra -g3 -fsanitize=address -Wno-unused-variable -Wno-unused-parameter
			#-Wno-unused-private-field

FLAGS_EMPTY =

SRC =	src/main.cpp \

OBJ = ${SRC:.cpp=.o}

CLIENT_SRC = src/client.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

all: $(NAME)

$(NAME): clean
	@$(CC) $(FLAGS_DEBUG) $(SRC) -o $(NAME)

client: clean
	@$(CC) $(FLAGS_UNUSED) $(CLIENT_SRC) -o client

clean:
	@rm -f $(OBJ) $(CLIENT_OBJ) $(NAME) client

test_config: test/config.cpp
	@$(CC) $(FLAGS_DEBUG_UNUSED) $< -o tester

re: clean all

.PHONY: re clean