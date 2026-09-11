CXX ?= g++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic
LDLIBS ?= -lX11

TARGET := build/egypt
SOURCES := src/egypt.cpp
MENU_PARTS := assets/menu96_0a1.b64 assets/menu96_0a2.b64 assets/menu96_0b.b64 assets/menu96_1.b64 assets/menu96_2.b64
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
