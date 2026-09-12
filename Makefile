CMAKE ?= cmake
BUILD_DIR ?= build
BUILD_TYPE ?= Release

TARGET := $(BUILD_DIR)/egypt
MENU_ASSET := $(BUILD_DIR)/assets/menu.e16
MENU_PARTS := \
	assets/menu_hd_00.b64 assets/menu_hd_00_tail.b64 \
	assets/menu_hd_01.b64 assets/menu_hd_01_tail.b64 \
	assets/menu_hd_02.b64 assets/menu_hd_03.b64 assets/menu_hd_04.b64 \
	assets/menu_hd_05a.b64 assets/menu_hd_05b0.b64 assets/menu_hd_05b1.b64 \
	assets/menu_hd_05b1_tail.b64 assets/menu_hd_05.b64

.PHONY: all configure run clean menu-asset

all: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel

menu-asset: $(MENU_ASSET)

$(MENU_ASSET): $(MENU_PARTS)
	@printf 'Preparing 640x360 Egypt menu artwork...\n'
	@mkdir -p $(BUILD_DIR)/assets
	@{ \
		cat assets/menu_hd_00.b64 assets/menu_hd_00_tail.b64; \
		cat assets/menu_hd_01.b64 assets/menu_hd_01_tail.b64; \
		cat assets/menu_hd_02.b64; \
		cat assets/menu_hd_03.b64; \
		head -c 12000 assets/menu_hd_04.b64; \
		cat assets/menu_hd_05a.b64 assets/menu_hd_05b0.b64 assets/menu_hd_05b1.b64 assets/menu_hd_05b1_tail.b64; \
		cat assets/menu_hd_05.b64; \
	} | base64 -d > $(MENU_ASSET).tmp
	@test "$$(stat -c%s $(MENU_ASSET).tmp)" = "54800" || { echo 'Egypt HD menu asset has the wrong size'; rm -f $(MENU_ASSET).tmp; exit 1; }
	@test "$$(head -c 4 $(MENU_ASSET).tmp)" = "EJ8A" || { echo 'Egypt HD menu asset has the wrong signature'; rm -f $(MENU_ASSET).tmp; exit 1; }
	@mv $(MENU_ASSET).tmp $(MENU_ASSET)

configure: $(MENU_ASSET)
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -f egypt
