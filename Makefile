PLAYER1_ID := 41
PLAYER2_ID := 68

# Server
SERVER_FOLDER := server
SERVER_EXECUTABLE := server
DATA_FOLDER_PATH := server/data

# Client lib
CONNECTIONDIR := bots/c/client_lib

# Settings (defaults if unspecified, my be overridden)
VARIANT ?= hardcore
MODE    ?= dev

BOT_ROOT           := bots/c/$(VARIANT)
PLAYER_1_FOLDER    := $(BOT_ROOT)/my-core-bot
PLAYER_2_FOLDER    := $(BOT_ROOT)/gridmaster
CONFIG_FOLDER      := $(BOT_ROOT)/configs
CONFIG_SERVER_FILE := $(CONFIG_FOLDER)/server.config.json
CONFIG_GAME_FILE   := $(CONFIG_FOLDER)/game.config.json

# -------------------- Internal switches --------------------
ifeq ($(MODE),dev)
  SERVER_BUILD_TARGET := server_build_dev
else ifeq ($(MODE),prod)
  SERVER_BUILD_TARGET := server_build_prod
else
  $(error MODE must be 'dev' or 'prod' (got '$(MODE)'))
endif

# -------------------- Run targets --------------------
all: build_clients
re: fclean all

dev: MODE:=dev
dev: run_current

prod: MODE:=prod
prod: run_current

# sd = soft dev, sp = soft prod, hd = hard dev, hp = hard prod
sd: ; $(MAKE) VARIANT=softcore MODE=dev run_current
sp: ; $(MAKE) VARIANT=softcore MODE=prod run_current
hd: ; $(MAKE) VARIANT=hardcore MODE=dev run_current
hp: ; $(MAKE) VARIANT=hardcore MODE=prod run_current

soft: ; $(MAKE) VARIANT=softcore MODE=dev run_current
hard: ; $(MAKE) VARIANT=hardcore MODE=dev run_current

run: prod
start: prod

run_current: stop build_clients $(SERVER_BUILD_TARGET)
	$(PLAYER_2_FOLDER)/gridmaster $(PLAYER1_ID) &
	$(PLAYER_1_FOLDER)/bot        $(PLAYER2_ID) &
	./$(SERVER_FOLDER)/$(SERVER_EXECUTABLE) \
		$(CONFIG_SERVER_FILE) $(CONFIG_GAME_FILE) \
		$(DATA_FOLDER_PATH) $(PLAYER1_ID) $(PLAYER2_ID)

redev:  fclean dev
reprod: fclean prod

stop:
	@pkill $(SERVER_EXECUTABLE)   > /dev/null || true &
	@pkill bot                    > /dev/null || true &
	@pkill gridmaster            > /dev/null || true

# -------------------- Build targets --------------------
server_build_dev:
	$(MAKE) -C $(SERVER_FOLDER) dev

server_build_prod:
	$(MAKE) -C $(SERVER_FOLDER) prod

build_clients: player_1_build player_2_build setup-hooks

player_1_build:
	$(MAKE) -C $(CONNECTIONDIR)
	$(MAKE) -C $(PLAYER_1_FOLDER) CONNECTIONDIR=$(CONNECTIONDIR)

player_2_build:
	$(MAKE) -C $(CONNECTIONDIR)
	$(MAKE) -C $(PLAYER_2_FOLDER) CONNECTIONDIR=$(CONNECTIONDIR)

visualizer_build:
	cd visualizer && npm i && npm run build

# -------------------- Clean targets --------------------
clean: stop
	-$(MAKE) -C bots/c/hardcore/my-core-bot clean
	-$(MAKE) -C bots/c/hardcore/gridmaster clean
	-$(MAKE) -C bots/c/softcore/my-core-bot clean
	-$(MAKE) -C bots/c/softcore/gridmaster clean
	-$(MAKE) -C $(SERVER_FOLDER) clean
	-$(MAKE) -C $(CONNECTIONDIR) clean

fclean: clean
	-$(MAKE) -C bots/c/hardcore/my-core-bot fclean
	-$(MAKE) -C bots/c/hardcore/gridmaster fclean
	-$(MAKE) -C bots/c/softcore/my-core-bot fclean
	-$(MAKE) -C bots/c/softcore/gridmaster fclean
	-$(MAKE) -C $(SERVER_FOLDER) fclean
	-$(MAKE) -C $(CONNECTIONDIR) fclean

# -------------------- Meta / Monorepo Targets --------------------
setup-hooks:
	chmod +x .githooks/* || true
	git config core.hooksPath .githooks

vis:
	$(MAKE) visualizer
visualizer:
	cd visualizer && npm i && npm run dev

.PHONY: all re run start dev redeve prod reprod stop \
        server_build_dev server_build_prod build_clients \
        player_1_build player_2_build visualizer_build clean fclean \
        vis visualizer sd sp hd hp soft hard run_current
