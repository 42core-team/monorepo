CC = cc
CFLAGS = -Wall -Wextra -Werror -ggdb -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
LIBRARY = con_lib.a
INC = inc/
OBJDIR = build
SRCS = json/encode_json_utils.c json/encode_json.c json/get_next_line.c json/jsmn.c json/parse_json_objects.c json/parse_json_utils.c json/parse_json.c utils/print_utils.c utils/socket.c utils/utils.c actions.c con_lib.c
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

all: build $(LIBRARY)

release:
	make CFLAGS="-Wall -Wextra -Werror -O3" re

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
