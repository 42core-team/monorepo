CXX := g++

CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pthread

LDFLAGS := -lboost_system -pthread

INCLUDEDIRS := -I./src -Isubmodules/json/single_include/nlohmann/ -Isubmodules/boost/libs/asio/include/

SOURCES := $(shell find ./src -name "*.cpp")

OBJECTS := $(SOURCES:.cpp=.o)

TARGET := core

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "🔗 Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	@echo "✂️  Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDEDIRS) -c $< -o $@

re: fclean all

start: run

run: all
	./$(TARGET)

ren: re run

clean:
	@echo "🧹 Cleaning up..."
	rm -f $(OBJECTS)

fclean: clean
	@echo "🗑️ Removing $(TARGET)..."
	rm -f $(TARGET)

.PHONY: all clean fclean re run ren start
