SRCDIR = src
OBJDIR = build
EXEC = gridmaster

all: $(EXEC)

build:
	cd $(SRCDIR) && go build -o ../$(EXEC) main.go

run: all
	./$(EXEC)

fclean:
	rm -rf $(EXEC)

re: fclean all

.PHONY: all run fclean re $(EXEC)
