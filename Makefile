# Players
PLAYER1_ID := 42
PLAYER2_ID := 43
PLAYER_1_FOLDER := my-core-bot/my-core-bot
PLAYER_2_FOLDER := my-core-bot/gridmaster
PLAYER_1_EXECUTABLE := bot
PLAYER_2_EXECUTABLE := gridmaster

# Server
SERVER_FOLDER := server
SERVER_EXECUTABLE := server

# Configs
CONFIG_FOLDER := my-core-bot/configs
CONFIG_SERVER_FILE := $(CONFIG_FOLDER)/server-config.json
CONFIG_GAME_FILE := $(CONFIG_FOLDER)/soft-config.json

# Git info
CURRENT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)

# -------------------- Run targets --------------------
all: build
re: fclean all

run: stop build
	./$(SERVER_FOLDER)/$(SERVER_EXECUTABLE) $(CONFIG_SERVER_FILE) $(CONFIG_GAME_FILE) $(PLAYER1_ID) $(PLAYER2_ID) > /dev/null &
	$(PLAYER_2_FOLDER)/$(PLAYER_2_EXECUTABLE) $(PLAYER1_ID) > /dev/null &
	$(PLAYER_1_FOLDER)/$(PLAYER_1_EXECUTABLE) $(PLAYER2_ID)
ren: fclean run

debug: stop build
	$(PLAYER_2_FOLDER)/$(PLAYER_2_EXECUTABLE) $(PLAYER1_ID) &
	$(PLAYER_1_FOLDER)/$(PLAYER_1_EXECUTABLE) $(PLAYER2_ID) &
	./$(SERVER_FOLDER)/$(SERVER_EXECUTABLE) $(CONFIG_SERVER_FILE) $(CONFIG_GAME_FILE) $(PLAYER1_ID) $(PLAYER2_ID)
rebug: fclean debug

stop:
	@pkill $(SERVER_EXECUTABLE) > /dev/null || true &
	@pkill $(PLAYER_1_EXECUTABLE) > /dev/null || true &
	@pkill $(PLAYER_2_EXECUTABLE) > /dev/null || true

# -------------------- Build targets --------------------
build: server_build player_1_build player_2_build visualizer_build

server_build:
	make -C $(SERVER_FOLDER)

player_1_build:
	make -C $(PLAYER_1_FOLDER) CONNECTIONDIR=/workspaces/monorepo/client_lib

player_2_build:
	make -C $(PLAYER_2_FOLDER) CONNECTIONDIR=/workspaces/monorepo/client_lib

visualizer_build:
	cd visualizer && npm i && npm run build


# -------------------- Clean targets --------------------
clean: stop
	make -C $(PLAYER_1_FOLDER) clean
	make -C $(PLAYER_2_FOLDER) clean
	make -C $(SERVER_FOLDER) clean
	make -C client_lib clean

fclean: clean
	make -C $(PLAYER_1_FOLDER) fclean
	make -C $(PLAYER_2_FOLDER) fclean
	make -C $(SERVER_FOLDER) fclean
	make -C client_lib fclean

# -------------------- Clean targets --------------------
update:
	git submodule update --init --recursive
	git -C my-core-bot fetch origin --prune
	git -C my-core-bot checkout -B $(CURRENT_BRANCH) --track origin/$(CURRENT_BRANCH) || git -C my-core-bot checkout -B $(CURRENT_BRANCH) origin/dev
	git -C my-core-bot pull --ff-only

vis:
	make visualizer
visualizer:
	cd visualizer && npm i && npm run dev

.PHONY: all re run ren debug rebug stop build server_build player_1_build player_2_build clean fclean update vis visualizer visualizer_build
