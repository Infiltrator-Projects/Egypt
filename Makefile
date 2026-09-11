CXX ?= g++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic
LDLIBS ?= -lX11

TARGET := build/egypt
SOURCES := src/egypt.cpp
MENU_B64 := assets/menu.erle.b64
MENU_ASSET := assets/menu.erle

.PHONY: all clean run

all: $(TARGET)

$(MENU_ASSET): $(MENU_B64)
	@printf 'Preparing Egypt menu artwork...\n'
	@base64 -d $(MENU_B64) > $(MENU_ASSET).tmp
	@mv $(MENU_ASSET).tmp $(MENU_ASSET)

$(TARGET): $(SOURCES) $(MENU_ASSET)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build
	rm -f egypt
	rm -f $(MENU_ASSET) $(MENU_ASSET).tmp
