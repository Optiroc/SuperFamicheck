BUILD_DIR := build

.PHONY: clean

test:
	@cmake -B$(BUILD_DIR)
	@cmake --build $(BUILD_DIR) --parallel 4
	@$(BUILD_DIR)/test

all: clean test

clean:
	@rm -rf $(BUILD_DIR)
