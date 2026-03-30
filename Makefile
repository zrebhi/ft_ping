NAME        = ft_ping

CC          = cc
CFLAGS      = -Wall -Wextra -Werror

SRCS        = src/main.c \
			  src/parser.c \
              src/dns.c \
              src/socket.c \
              src/packet.c \
			  src/ping.c \
			  src/stats.c \
			  src/verbose.c \

OBJS        = $(SRCS:.c=.o)

BONUS_SRCS  = src/bonus/main_bonus.c
BONUS_OBJS  = $(BONUS_SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) -lm

%.o: %.c src/ft_ping.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(BONUS_OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

test: $(NAME)
	@./tests/test_cli.sh
	@./tests/test_dns.sh
	@./tests/test_valgrind.sh
	@./tests/test_socket.sh
	@./tests/test_loop.sh
	@./tests/test_recv.sh
	@./tests/test_signals.sh
	@./tests/test_verbose.sh

bonus: $(BONUS_OBJS)
	$(CC) $(CFLAGS) $(BONUS_OBJS) -o $(NAME)

.PHONY: all clean fclean re bonus test
