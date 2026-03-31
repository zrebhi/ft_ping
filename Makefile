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
			  src/icmp_utils.c \

OBJS        = $(SRCS:.c=.o)

BONUS_SRCS  = src/bonus/main_bonus.c \
              src/bonus/parser_bonus.c \
              src/bonus/socket_bonus.c \
              src/dns.c \
              src/packet.c \
              src/ping.c \
              src/stats.c \
              src/verbose.c \
			  src/icmp_utils.c \
			  
BONUS_OBJS  = $(BONUS_SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) -lm

%.o: %.c src/ft_ping.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(BONUS_OBJS) .bonus

fclean: clean
	rm -f $(NAME)

re: fclean all

bonus: .bonus

.bonus: $(BONUS_OBJS)
	$(CC) $(CFLAGS) $(BONUS_OBJS) -o $(NAME) -lm
	touch .bonus

test: $(NAME)
	@./tests/test_cli.sh
	@./tests/test_dns.sh
	@./tests/test_valgrind.sh
	@./tests/test_socket.sh
	@./tests/test_loop.sh
	@./tests/test_recv.sh
	@./tests/test_signals.sh
	@./tests/test_verbose.sh

test_bonus: bonus
	@./tests/test_cli.sh
	@./tests/test_dns.sh
	@./tests/test_valgrind.sh
	@./tests/test_socket.sh
	@./tests/test_loop.sh
	@./tests/test_recv.sh
	@./tests/test_signals.sh
	@BONUS_MODE=1 ./tests/test_verbose.sh
	@./tests/bonus/test_ttl.sh
	@./tests/bonus/test_r_flag.sh
	@./tests/bonus/test_c_flag.sh
	@./tests/bonus/test_w_flag.sh
	@./tests/bonus/test_q_flag.sh

.PHONY: all clean fclean re bonus test test_bonus
