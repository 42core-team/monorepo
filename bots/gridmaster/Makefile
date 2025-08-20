SRCDIR = src
OBJDIR = build
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:.c=.o)))

CC = cc
CFLAGS = -Wall -Wextra -Werror -lm -g -fsanitize=address

CONNECTIONDIR := $(if $(CONNECTION_DIR),$(CONNECTION_DIR),../connection)

LIBS = $(CONNECTIONDIR)/con_lib.a
INC = $(CONNECTIONDIR)/inc

EXEC = gridmaster

.PHONY: all run clean fclean re $(EXEC)

all: build $(EXEC)

run: all
	./$(EXEC)

$(EXEC): $(OBJS)
	make -C $(CONNECTIONDIR)
	$(CC) $(CFLAGS) $^ -o $@ -L $(CONNECTIONDIR) -l:con_lib.a -I $(INC)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC)

build:
	mkdir -p $(OBJDIR)
	make -C $(CONNECTIONDIR)

clean:
	make -C $(CONNECTIONDIR) clean
	rm -rf $(OBJDIR)

fclean: clean
	make -C $(CONNECTIONDIR) fclean
	rm -rf $(EXEC)

re: fclean all
