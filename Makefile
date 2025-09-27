NAME		= webserv

CPP			= c++
CPPFLAG		= -Wall -Wextra -Werror -std=c++98

INCLUDES	= -I src/cgi -I src/config -I src/connection -I src/server
CPPFLAG		+= $(INCLUDES)

SRCS		= src/main.cpp \
			  src/cgi/CGIHandler.cpp \
			  src/config/Config.cpp src/config/Server.cpp src/config/Tokenizer.cpp \
			  src/connection/epoll.cpp src/connection/Socket.cpp \
			  src/server/ErrorPage.cpp src/server/Request.cpp src/server/Response.cpp
OBJDIR		= obj
OBJS		= $(SRCS:%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $^ -o $@

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAG) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re
