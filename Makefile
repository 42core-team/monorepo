CXX := g++

CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pthread -MMD -MP -fsanitize=address -g

LDFLAGS := -pthread

INCLUDEDIRS := $(addprefix -I, $(shell find inc -type d))
DEPENDDIRS := -Isubmodules/json/single_include/nlohmann/

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
	$(CXX) $(CXXFLAGS) $(INCLUDEDIRS) $(DEPENDDIRS) -c $< -o $@

re: fclean all

clean:
	@echo "🧹 Cleaning up..."
	rm -f $(OBJECTS)

fclean: clean
	@echo "🗑️ Removing $(TARGET)..."
	rm -f $(TARGET)

-include $(OBJECTS:.o=.d)

.PHONY: all clean fclean re run ren start
