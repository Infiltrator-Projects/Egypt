CXX ?= g++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic
LDLIBS ?= -lX11

TARGET := build/egypt
SOURCES := src/egypt.cpp
MENU_PARTS := assets/m160x_00.b64 assets/m160x_01.b64 assets/m160x_02.b64 assets/m160x_03.b64 assets/m160x_04.b64 assets/m160x_05.b64 assets/m160x_06.b64 assets/m160x_07.b64 assets/m160x_08.b64
MENU_ASSET := assets/menu.erle

.PHONY: all clean run

all: $(TARGET)

$(MENU_ASSET): $(MENU_PARTS)
	@printf 'Preparing Egypt menu artwork...\n'
	@cat $(MENU_PARTS) | base64 -d > $(MENU_ASSET).tmp
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
