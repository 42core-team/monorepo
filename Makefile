LIBRARY = con_lib.a

CC = cc
CFLAGS = -Wall -Wextra -Werror -lm -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment

INC = inc/
OBJDIR = build
VPATH = json utils getter
SRCS = \
encode_json_utils.c \
encode_json.c \
get_next_line.c \
jsmn.c \
parse_json_objects.c \
parse_json_utils.c \
parse_json.c \
print_utils.c \
error.c \
init_con_utils.c \
socket.c \
utils.c \
actions.c \
con_lib.c \
get.c \
get_core.c \
get_team.c \
get_unit.c \
get_unit_config.c \
get_utils.c \
get_resource.c

OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

all: build $(LIBRARY)

release:
	make CFLAGS="-Wall -Wextra -Werror -lm -O3" re

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
