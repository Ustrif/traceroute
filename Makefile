NAME = traceroute

CC = cc

CFLAGS = -Wall -Wextra -Werror -I includes -g

SRCS = parsing/init.c \
	parsing/parse_arguments.c \
	parsing/check_args.c \
	protocols/icmp.c \
	protocols/udp.c \
	protocols/tcp.c \
	resolver/resolver.c \
	resolver/reverse_resolver.c \
	utils/get_rtt.c \
	utils/checksum.c \
	utils/intro.c \
	main.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@echo "Linking $(NAME)..."
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
%.o: %.c
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning project objects..."
	@rm -f $(OBJS)

fclean: clean
	@echo "Removing executable..."
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re