SRCDIR = src
OBJDIR = build
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:.c=.o)))

CC = cc
CFLAGS = -Wall -Wextra -Werror -lm -g -fsanitize=address
CLIENT_LIB_DIR ?= /core
INC = -I inc -I $(CLIENT_LIB_DIR)/inc
EXEC = bot

all: ensure_obj_dir $(EXEC)

run: all
	./$(EXEC)

$(EXEC): $(OBJS) $(CLIENT_LIB_DIR)/core_lib.a
	$(CC) $(CFLAGS) $(OBJS) $(CLIENT_LIB_DIR)/core_lib.a -o $@

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
