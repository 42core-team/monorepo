JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

BUILD_DIR = cmake-build
BUILD_TYPE ?= dev
EXEC = gridmaster

all: $(EXEC)

$(EXEC): $(BUILD_DIR)/Makefile
	cmake --build $(BUILD_DIR) -j$(JOBS)

$(BUILD_DIR)/Makefile:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

run: all
	./$(EXEC)

prod:
	$(MAKE) BUILD_TYPE=prod all

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -rf $(EXEC)

re: fclean all

.PHONY: all run clean fclean re prod $(EXEC)
