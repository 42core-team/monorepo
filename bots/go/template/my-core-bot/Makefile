SRCDIR = src
OBJDIR = build
EXEC = bot

build:
	cd $(SRCDIR) && go build -o ../$(EXEC) main.go

run: build
	./$(EXEC)

clean:
	rm -rf $(OBJDIR)

fclean:
	rm -rf $(EXEC)

re: fclean build

.PHONY: all run fclean re $(EXEC)
