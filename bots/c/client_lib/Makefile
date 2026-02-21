JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

LIBRARY = core_lib.a
BUILD_DIR = cmake-build
BUILD_TYPE ?= dev

all: $(LIBRARY)

$(LIBRARY): $(BUILD_DIR)/Makefile
	cmake --build $(BUILD_DIR) -j$(JOBS)

$(BUILD_DIR)/Makefile:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

prod:
	$(MAKE) BUILD_TYPE=prod all

dev:
	$(MAKE) BUILD_TYPE=dev all

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -rf $(LIBRARY)

re:
	$(MAKE) fclean
	$(MAKE) all

log:
	find inc -type f -print -exec echo "====> {} <====" \; -exec cat {} \; || true
	find src -type f -print -exec echo "====> {} <====" \; -exec cat {} \; || true

.PHONY: all clean fclean re log prod dev
