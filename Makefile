NAME = webserv

SRCS =	$(wildcard *.cpp) \
		src/RequestParser.cpp

CC = c++

OBJS = $(SRCS:.cpp=.o)

FLAGS = -Wall -Wextra -Werror -std=c++98

%.o: %.c
	$(CC) $(FLAGS) $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

clean:
	rm -rf *.o

fclean:
	rm -rf *.o $(NAME)

re: fclean all

.PHONY:		all clean fclean re
