CC = cc
CFLAGS = -Wall -Wextra -Werror -ggdb -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
LIBRARY = con_lib.a
INC = inc/
OBJDIR = build
SRCS = $(wildcard *.c) $(wildcard **/*.c)
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

all: build $(LIBRARY)

$(LIBRARY): $(OBJS)
	ar rcs $(LIBRARY) $(OBJS)

$(OBJDIR)/$(notdir %.o): %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC)

build:
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(LIBRARY)

re: fclean all
