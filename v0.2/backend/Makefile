.PHONY: all build clean run

BUILD_DIR = build

all: build

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

build: $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && make -j$$(nproc)

run: build
	./$(BUILD_DIR)/oj-backend

clean:
	rm -rf $(BUILD_DIR)
