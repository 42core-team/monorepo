OBJDIR = build
EXEC = bot

build:
	go build -o $(EXEC) *.go

run: build
	./$(EXEC)

clean:
	rm -rf $(OBJDIR)

fclean:
	rm -rf $(EXEC)

re: fclean build

.PHONY: all run fclean re $(EXEC)
