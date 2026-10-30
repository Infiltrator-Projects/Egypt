SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c

CMAKE ?= cmake
BUILD_DIR ?= build
BUILD_TYPE ?= Release
PREFIX ?= /usr/local
BUILD_PROFILE ?= generic

TARGET := $(BUILD_DIR)/egypt

.PHONY: all configure run clean menu-asset hud-assets native install-native deb

all: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel

# Both authored artwork stages are built natively by CMake, never Python.
menu-asset hud-assets: configure
	$(CMAKE) --build $(BUILD_DIR) --target egypt-runtime-assets --parallel

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DEGYPT_BUILD_PROFILE=$(BUILD_PROFILE) -DCMAKE_INSTALL_PREFIX=$(PREFIX)

run: all
	./$(TARGET)

native:
	$(MAKE) BUILD_DIR=build-native BUILD_PROFILE=native PREFIX=$(PREFIX) all
	$(CMAKE) --build build-native --parallel
	ctest --test-dir build-native --output-on-failure

install-native: native
	$(CMAKE) --install build-native --component Runtime

deb:
	bash packaging/build-deb.sh

clean:
	rm -rf $(BUILD_DIR) build-native build-package dist
	rm -f egypt
