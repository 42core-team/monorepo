# Players
PLAYER1_ID := 42
PLAYER2_ID := 43
PLAYER_1_FOLDER := bot/martin
PLAYER_2_FOLDER := bot/gridmaster
PLAYER_1_EXECUTABLE := martin
PLAYER_2_EXECUTABLE := gridmaster

#Server
SERVER_FOLDER := server
SERVER_EXECUTABLE := server

# -------------------- Run targets --------------------
all: build
re: fclean all

run: stop build
	./$(SERVER_FOLDER)/$(SERVER_EXECUTABLE) ./config.server.json $(PLAYER1_ID) $(PLAYER2_ID) > /dev/null &
	$(PLAYER_2_FOLDER)/$(PLAYER_2_EXECUTABLE) $(PLAYER1_ID) > /dev/null &
	$(PLAYER_1_FOLDER)/$(PLAYER_1_EXECUTABLE) $(PLAYER2_ID)
ren: fclean run

debug: stop build
	$(PLAYER_2_FOLDER)/$(PLAYER_2_EXECUTABLE) $(PLAYER1_ID) &
	$(PLAYER_1_FOLDER)/$(PLAYER_1_EXECUTABLE) $(PLAYER2_ID) &
	./$(SERVER_FOLDER)/$(SERVER_EXECUTABLE) ./config.server.json $(PLAYER1_ID) $(PLAYER2_ID)
rebug: fclean debug

stop:
	@pkill $(SERVER_EXECUTABLE) > /dev/null || true &
	@pkill $(PLAYER_1_EXECUTABLE) > /dev/null || true &
	@pkill $(PLAYER_2_EXECUTABLE) > /dev/null || true

# -------------------- Build targets --------------------
build: server_build player_1_build player_2_build

server_build:
	make -C $(SERVER_FOLDER)

player_1_build:
	make -C $(PLAYER_1_FOLDER)

player_2_build:
	make -C $(PLAYER_2_FOLDER)


# -------------------- Clean targets --------------------
clean: stop
	make -C $(PLAYER_1_FOLDER) clean
	make -C $(PLAYER_2_FOLDER) clean
	make -C $(SERVER_FOLDER) clean

fclean: clean
	make -C $(PLAYER_1_FOLDER) fclean
	make -C $(PLAYER_2_FOLDER) fclean
	make -C $(SERVER_FOLDER) fclean

.PHONY: all re run ren debug rebug stop build server_build player_1_build player_2_build clean fclean
