JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
MAKEFLAGS += -j$(JOBS)

LIBRARY = core_lib.a

CC = cc
CFLAGS = -Wall -Wextra -Werror -lm -g -fsanitize=address -MMD -MP

INC = inc
OBJDIR = obj
SRCDIR = src

SRCS := $(shell find $(SRCDIR) -name "*.c")
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

-include $(OBJS:.o=.d)

all: $(LIBRARY)

$(LIBRARY): $(OBJS)
	ar rcs $(LIBRARY) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(LIBRARY)

re:
	$(MAKE) fclean
	$(MAKE) all

log:
	find inc -type f -print -exec echo "====> {} <====" \; -exec cat {} \; || true
	find src -type f -print -exec echo "====> {} <====" \; -exec cat {} \; || true

.PHONY: all clean fclean re log
