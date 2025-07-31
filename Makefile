NAME = webserv

CPP = c++
CPPFLAG = -Wall -Wextra -Werror -std=c++98 -Isource/server

SRCDIR = source/server
SRC = main.cpp
SRC += $(SRCDIR)/server.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(CPPFLAG) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CPP) $(CPPFLAG) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
