JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
MAKEFLAGS += -j$(JOBS)

LIBRARY = core_lib.a

CC = cc
CFLAGS = -Wall -Wextra -Werror -g -fsanitize=address -MMD -MP

INC = inc
OBJDIR = obj
SRCDIR = src

SRCS := $(shell find $(SRCDIR) -name "*.c")
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

.DEFAULT_GOAL := all

.PHONY: all clean fclean re test log

all: $(LIBRARY)

$(LIBRARY): $(OBJS)
	rm -f $(LIBRARY)
	ar rcs $(LIBRARY) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC)

-include $(OBJS:.o=.d)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(LIBRARY)

re: fclean all

test:
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) tests/test_action_travel.c src/public/action_travel.c -I $(INC) -o $(OBJDIR)/test_action_travel
	./$(OBJDIR)/test_action_travel

log:
	find inc -type f -print -exec echo "====> {} <====" \; -exec cat {} \; || true
	find src -type f -print -exec echo "====> {} <====" \; -exec cat {} \; || true
