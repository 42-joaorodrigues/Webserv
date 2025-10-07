
# ░█░█░█▀▀░█▀▄░█▀▀░█▀▀░█▀▄░█░█
# ░█▄█░█▀▀░█▀▄░▀▀█░█▀▀░█▀▄░▀▄▀
# ░▀░▀░▀▀▀░▀▀░░▀▀▀░▀▀▀░▀░▀░░▀░
# by fsilva-p, joao-alm & nacao

# Compiler Settings
NAME			= webserv
CC				= c++
CFLAGS			= -Wall -Werror -Wextra -std=c++98 -g3
RM				= rm -rf
O_DIR			= obj

# Print Settings
header			= obj/header
col1pad			= 9
col2pad			= 22

# Mandatory Files
INC				= -I src/cgi -I src/config -I src/http -I src/network -I src/services -I src/cookie
SRC_MAIN		= src/main.cpp
SRC_CGI			= src/cgi/CGIHandler.cpp \
				  src/cgi/CgiUtil.cpp
SRC_CONFIG		= src/config/Config.cpp \
				  src/config/Server.cpp \
				  src/config/Tokenizer.cpp
SRC_HTTP		= src/http/HttpHandler.cpp \
				  src/http/Request.cpp \
				  src/http/Response.cpp \
				  src/http/LocationMatcher.cpp
SRC_NETWORK		= src/network/epoll.cpp \
				  src/network/Socket.cpp
SRC_SERVICES	= src/services/DirectoryListing.cpp \
				  src/services/UploadService.cpp \
				  src/services/HttpUtils.cpp \
				  src/services/Logger.cpp
SRC				= $(SRC_MAIN) $(SRC_CGI) $(SRC_CONFIG) $(SRC_HTTP) $(SRC_NETWORK) $(SRC_SERVICES)
OBJ				= $(SRC:src/%.cpp=$(O_DIR)/%.o)

# Mandatory Rules
all: $(header) $(NAME)

$(O_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@make .print_start print_color="$(y)" print_action="Compiling" print_name=$(NAME) print_file=$(notdir $@) --no-print-directory
	@$(CC) $(CFLAGS) -c $< -o $@ $(INC)

$(NAME): $(OBJ)
	@make .print_start print_color="$(y)" print_action="Linking" print_name=$(NAME) print_file=$(NAME) --no-print-directory
	@$(CC) $(CFLAGS) $(OBJ) -o $@
	@make .print_end print_color="$(y)" print_action="Linking" print_name=$(NAME) --no-print-directory

clean:
	@if [ ! -d $(O_DIR) ]; then printf "Nothing to remove\n"; fi
	@if [ -d $(O_DIR) ]; then make .remove tname="webserv_objects" t1=$(O_DIR) --no-print-directory; fi

fclean:
	@if [ ! -d $(O_DIR) ] && [ ! -f $(NAME) ]; then printf "Nothing to remove\n"; fi
	@if [ -d $(O_DIR) ]; then make .remove tname=$(O_DIR) t1=$(O_DIR) t2=$(O_DIR) --no-print-directory; fi
	@if [ -f $(NAME) ]; then make .remove tname=$(NAME) t1=$(NAME) t2=$(NAME) --no-print-directory; fi

re: fclean all

# Custom Rules
.print_start:
	@printf "$(print_color)%-$(col1pad).$(col1pad)s$(r) %-$(col2pad).$(col2pad)s %s$(c)\r" "$(print_action)" "$(print_name)" "$(print_file)"

.print_end:
	@printf "$(print_color)%-$(col1pad).$(col1pad)s$(r) %-$(col2pad).$(col2pad)s $(g)Success$(r)$(c)\n" "$(print_action)" "$(print_name)"

.remove:
	@make .print_start print_color="$(p)" print_action="Removing" print_name=$(tname) print_file=$(t1) --no-print-directory
	@$(RM) $(t1)
	@make .print_start print_color="$(p)" print_action="Removing" print_name=$(tname) print_file=$(t2) --no-print-directory
	@$(RM) $(t2)
	@make .print_end print_color="$(p)" print_action="Removing" print_name=$(tname) --no-print-directory

# Headers
$(header):
	@mkdir -p $(O_DIR)
	@printf "$(y)"
	@printf " __    __     _                         \n";
	@printf "/ / /\ \ \___| |__  ___  ___ _ ____   __\n";
	@printf "\ \/  \/ / _ \ '_ \/ __|/ _ \ '__\ \ / /\n";
	@printf " \  /\  /  __/ |_) \__ \  __/ |   \ V / \n";
	@printf "  \/  \/ \___|_.__/|___/\___|_|    \_/  \n";
	@printf "           by fsilva-p, joao-alm & nacao\n";
	@printf "$(r)\n"
	@touch $(header)

# Colours
c	= \033[K
p	= \033[38;2;211;125;174m
y	= \033[38;2;255;231;151m
g	= \033[38;2;117;197;141m
b	= \033[1m
r	= \033[0m

# Phony
.PHONY: all clean fclean re
