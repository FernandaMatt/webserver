NAME = webserv

SRCS =	src/main.cpp \
		src/cgi/HandleCGI.cpp \
		src/config/Location.cpp \
		src/config/Parser.cpp \
		src/config/Server.cpp \
		src/core/WebServer.cpp \
		src/http/RequestParser.cpp \
		src/http/Response.cpp \
		src/http/ResponseBuilder.cpp \
		src/utils/Logger.cpp \
		src/utils/MimeTypes.cpp

CC = c++

OBJS = $(SRCS:.cpp=.o)

FLAGS = -Wall -Wextra -Werror -std=c++98

%.o: %.c
	$(CC) $(FLAGS) $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJS)

fclean:
	rm -rf $(OBJS) $(NAME)

re: fclean all

.PHONY:		all clean fclean re
