SRCDIR = src
OBJDIR = build
EXEC = gridmaster

build:
	cd $(SRCDIR) && go build -o ../$(EXEC) main.go

run: build
	./$(EXEC)

fclean:
	rm -rf $(EXEC)

re: fclean build

.PHONY: all run fclean re $(EXEC)
