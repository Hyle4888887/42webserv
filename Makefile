NAME = webserv

CPP = c++
FLAGS = -Wall -Wextra -Werror -std=c++98

OBJ_DIR = objs

SRC = main.cpp CGI/buildCGIResponse.cpp CGI/executeCGI.cpp

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