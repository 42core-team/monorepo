CXX := g++

CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pthread -MMD -MP -g

LDFLAGS := -pthread

INCLUDEDIRS := $(addprefix -I, $(shell find inc -type d))

SRCDIR := src
OBJDIR := obj

SOURCES := $(shell find $(SRCDIR) -name "*.cpp")
OBJECTS   := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

TARGET := core

build: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "🔗 Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	@echo "✂️  Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDEDIRS) -c $< -o $@

re: fclean all

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
