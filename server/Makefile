JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
MAKEFLAGS += -j$(JOBS)

CXX := clang++

CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pthread -MMD -MP -fsanitize=address -g

LDFLAGS := -pthread

INCLUDEDIRS := $(addprefix -I, $(shell find inc -type d))

SRCDIR := src
OBJDIR := obj

SOURCES := $(shell find $(SRCDIR) -name "*.cpp")
OBJECTS   := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

TARGET := server

build: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "🔗 Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	@echo "✂️  Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDEDIRS) -c $< -o $@

re:
	$(MAKE) fclean
	$(MAKE) all

clean:
	@echo "🧹 Cleaning up..."
	rm -f $(OBJECTS)

fclean: clean
	@echo "🗑️ Removing $(TARGET)..."
	rm -f $(TARGET)

-include $(OBJECTS:.o=.d)

.PHONY: logfiles
logfiles:
	@echo "📜 Logging all source and include files (excluding submodules)..."
	@echo "🔍 Scanning source directory..."
	@find src -type f -not -path "./submodules/*" -not -name "*.txt" -not -name "*.html" | sort | while read -r file; do \
		echo "Source file: $$file"; \
		cat $$file; \
	done
	@echo "🔍 Scanning include directory..."
	@find inc -type f -not -path "./submodules/*" | sort | while read -r file; do \
		echo "Include file: $$file"; \
		cat $$file; \
	done

.PHONY: all clean fclean re run ren start

# Test Dockerfile
build-docker:
	docker build -t game-server-test:latest -f ../.github/workflows/server/Dockerfile ../

run-docker:
	docker run --rm -p 4242:4242 game-server-test:latest 1 2
