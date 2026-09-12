SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c

CMAKE ?= cmake
PYTHON ?= python3
BUILD_DIR ?= build
BUILD_TYPE ?= Release

TARGET := $(BUILD_DIR)/egypt
MENU_ASSET := $(BUILD_DIR)/assets/menu.e16
MENU_PARTS := $(sort $(wildcard assets/menu_q20_xz_*.b64))
MENU_SHA256 := e22ea3d3689495f83b6bd31437806ff897fff530f3d8420579f142bd3a1337c9
HUD_STAMP := $(BUILD_DIR)/assets/.hud-art.stamp
HUD_BUILDER := tools/build_hud_assets.py
HUD_SOURCES := assets/hud_chrome_authored.e8p.b85 assets/hud_glyphs_authored.e8p.b85 assets/tool_icons_authored.e8p.b85

.PHONY: all configure run clean menu-asset hud-assets

all: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel

menu-asset: $(MENU_ASSET)
hud-assets: $(HUD_STAMP)

$(MENU_ASSET): $(MENU_PARTS)
	@printf 'Preparing verified 640x360 Egypt menu artwork...\n'
	@test "$$(printf '%s\n' $(MENU_PARTS) | wc -l)" = "29" || { echo 'Egypt menu payload is incomplete'; exit 1; }
	@mkdir -p $(BUILD_DIR)/assets
	@cat $(MENU_PARTS) | base64 -d | xz -dc > $(MENU_ASSET).tmp
	@test "$$(stat -c%s $(MENU_ASSET).tmp)" = "35948" || { echo 'Egypt menu asset has the wrong size'; rm -f $(MENU_ASSET).tmp; exit 1; }
	@test "$$(head -c 4 $(MENU_ASSET).tmp)" = "EJ8A" || { echo 'Egypt menu asset has the wrong signature'; rm -f $(MENU_ASSET).tmp; exit 1; }
	@echo "$(MENU_SHA256)  $(MENU_ASSET).tmp" | sha256sum -c -
	@mv $(MENU_ASSET).tmp $(MENU_ASSET)

$(HUD_STAMP): $(HUD_BUILDER) $(HUD_SOURCES)
	@printf 'Installing authored Egypt HUD artwork...\n'
	@mkdir -p $(BUILD_DIR)/assets
	@$(PYTHON) $(HUD_BUILDER) $(BUILD_DIR)/assets
	@test "$$(head -c 4 $(BUILD_DIR)/assets/hud_chrome.e8p)" = "E8PA" || { echo 'Egypt HUD chrome failed to install'; exit 1; }
	@test "$$(head -c 4 $(BUILD_DIR)/assets/hud_glyphs.e8p)" = "E8PA" || { echo 'Egypt HUD glyph artwork failed to install'; exit 1; }
	@test "$$(head -c 4 $(BUILD_DIR)/assets/tool_icons.e8p)" = "E8PA" || { echo 'Egypt tool artwork failed to install'; exit 1; }
	@touch $(HUD_STAMP)

configure: $(MENU_ASSET) $(HUD_STAMP)
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -f egypt
