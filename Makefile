NAME    := webserv

CPP     := c++
CPPFLAG := -Wall -Wextra -Werror -std=c++98 -MMD -MP

INCLUDES := $(shell find include source -type d)
CPPFLAG  += $(addprefix -I, $(INCLUDES))

SRCDIRS := source/server source/config source/connection
SRCS    := $(wildcard $(SRCDIRS:%=%/*.cpp)) source/main.cpp
OBJDIR  := build
OBJS    := $(SRCS:%.cpp=$(OBJDIR)/%.o)
DEPS    := $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CPPFLAG) $(OBJS) -o $@

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
