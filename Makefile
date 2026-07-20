.SILENT:

NAME = webserv

CPP = c++
FLAGS = -Wall -Wextra -Werror -std=c++98

OBJ_DIR = objs

SRC = main.cpp \
	HTTP/HTTP.cpp HTTP/build.cpp HTTP/handle.cpp HTTP/response.cpp HTTP/htmlBody.cpp\
	CGI/buildResponse.cpp CGI/start.cpp CGI/CGI.cpp\
	server/server.cpp server/buildHTTP.cpp server/checkTimeout.cpp server/handleCGI.cpp server/handleOperation.cpp server/poll.cpp server/socket.cpp\
	utils/utils.cpp

OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(FLAGS) -o $(NAME) $(OBJ)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CPP) -c $(FLAGS) -o $@ $<

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all fclean clean re