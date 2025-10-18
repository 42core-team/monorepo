SRCDIR = src
OBJDIR = build
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:.c=.o)))

CC = cc
CFLAGS = -Wall -Wextra -Werror -lm -g -fsanitize=address
CONNECTIONDIR ?= /core
INC = -I inc -I $(CONNECTIONDIR)/inc
EXEC = bot

all: ensure_obj_dir $(EXEC)

run: all
	./$(EXEC)

$(EXEC): $(OBJS) $(CONNECTIONDIR)/core_lib.a
	$(CC) $(CFLAGS) $(OBJS) $(CONNECTIONDIR)/core_lib.a -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INC)

ensure_obj_dir:
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(EXEC)

re: fclean all

.PHONY: all run clean fclean re $(EXEC) ensure_obj_dir
