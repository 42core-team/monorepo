# -------------------- inputs --------------------

.PHONY: require-vars

require-vars:
	@test -n "$(LANGUAGE)" || (echo "LANGUAGE is required (e.g. LANGUAGE=c) (c|go)" && exit 2)
	@test -n "$(STAGE)" || (echo "STAGE is required (dev|prod)" && exit 2)
	@echo "$(LANGUAGE)" | grep -Eq '^(c|go)$$' || (echo "LANGUAGE must be c or go (got $(LANGUAGE))" && exit 2)
	@echo "$(STAGE)" | grep -Eq '^(dev|prod)$$' || (echo "STAGE must be dev or prod (got $(STAGE))" && exit 2)

# -------------------- variables --------------------

PLAYER1_ID ?= 42
PLAYER2_ID ?= 418

SERVER_FOLDER		:= server
SERVER_EXECUTABLE	:= server
DATA_FOLDER_PATH	:= server/data

CLIENT_LIB_DIR := /workspaces/monorepo/bots/$(LANGUAGE)/client_lib

BOT_ROOT			:= bots/$(LANGUAGE)/bot
PLAYER_1_FOLDER		:= $(BOT_ROOT)/my-core-bot
PLAYER_2_FOLDER		:= $(BOT_ROOT)/gridmaster
CONFIG_ROOT			:= bots/configs
CONFIG_FOLDER		= $(if $(and $(wildcard $(CONFIG_ROOT)/real/server.config.json),$(wildcard $(CONFIG_ROOT)/real/game.config.json)),$(CONFIG_ROOT)/real,$(CONFIG_ROOT)/fake_demo)
CONFIG_SERVER_FILE	= $(CONFIG_FOLDER)/server.config.json
CONFIG_GAME_FILE		= $(CONFIG_FOLDER)/game.config.json

# -------------------- targets --------------------

.PHONY: all configs stop clean fclean

.DEFAULT_GOAL := all
all: setup-hooks require-vars configs stop
	# build client lib (only for c)
	$(if $(filter c,$(LANGUAGE)),$(MAKE) -C $(CLIENT_LIB_DIR))

	# build clients
	$(MAKE) -C $(PLAYER_1_FOLDER) CLIENT_LIB_DIR=$(CLIENT_LIB_DIR)
	$(MAKE) -C $(PLAYER_2_FOLDER) CLIENT_LIB_DIR=$(CLIENT_LIB_DIR)

	# build server
	$(MAKE) -C $(SERVER_FOLDER) $(STAGE)

	# run clients and server
	$(PLAYER_2_FOLDER)/gridmaster	$(PLAYER2_ID) &
	$(PLAYER_1_FOLDER)/bot			$(PLAYER1_ID) &
	./$(SERVER_FOLDER)/$(SERVER_EXECUTABLE) \
		$(CONFIG_SERVER_FILE) $(CONFIG_GAME_FILE) \
		$(DATA_FOLDER_PATH) $(PLAYER1_ID) $(PLAYER2_ID)

configs:
	-@git submodule update --init --remote --recursive

stop:
	@pkill $(SERVER_EXECUTABLE)	> /dev/null 2>&1 || true &
	@pkill bot					> /dev/null 2>&1 || true &
	@pkill gridmaster			> /dev/null 2>&1 || true

clean: stop
	-$(MAKE) -C $(SERVER_FOLDER) clean

	# c
	-$(MAKE) -C bots/c/bot clean
	-$(MAKE) -C bots/c/client_lib clean

fclean: clean
	-$(MAKE) -C $(SERVER_FOLDER) fclean

	# c
	-$(MAKE) -C bots/c/bot fclean
	-$(MAKE) -C bots/c/client_lib fclean

# -------------------- misc --------------------

.PHONY: visualizer vis setup-hooks

vis: visualizer
visualizer:
	cd visualizer && npm i && npm run dev

setup-hooks:
	@chmod +x .githooks/* || true
	@git config core.hooksPath .githooks
