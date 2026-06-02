NAME = webserv

CPP = c++
FLAGS = -Wall -Wextra -Werror -std=c++98

SRC = main.cpp CGI/buildCGIResponse.cpp CGI/executeCGI.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(FLAGS) -o $(NAME) $(OBJ)

%.o: %.cpp
	$(CPP) -c $(FLAGS) -o $@ $<

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all fclean clean re