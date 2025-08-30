JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
CMAKE ?= cmake
BUILD_DIR ?= build
CONFIG ?= Debug

.PHONY: all build dev prod clean fclean re

all:
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CONFIG) --log-level=VERBOSE
	$(CMAKE) --build $(BUILD_DIR) --parallel $(JOBS)

build: all
dev:
	$(MAKE) CONFIG=dev all
prod:
	$(MAKE) CONFIG=prod all

clean:
	@if [ -d "$(BUILD_DIR)" ]; then $(CMAKE) --build "$(BUILD_DIR)" --target clean; else :; fi

fclean: clean
	rm -f server
	rm -rf build

re:
	$(MAKE) fclean
	$(MAKE) all
