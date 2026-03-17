OBJDIR = build
EXEC = gridmaster

build:
	go build -o $(EXEC) main.go

run: build
	./$(EXEC)

clean:
	rm -rf $(OBJDIR)

fclean:
	rm -rf $(EXEC)

re: fclean build

.PHONY: all run fclean re $(EXEC)
