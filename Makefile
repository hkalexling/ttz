.PHONY: clean libs example

BUILD_DIR ?= ./build

COUNTRY_COUNT ?= 100
MIN_YEAR ?= 1980
MAX_YEAR ?= 2030

SRC_GEN = src/countries.c src/zones.c
SRC = $(SRC_GEN) src/ttz.c
OBJ = $(SRC:src/%.c=$(BUILD_DIR)/%.o)

libs: $(BUILD_DIR)/libttz.so $(BUILD_DIR)/libttz.a

resources/countries.csv:
	COUNTRY_COUNT=$(COUNTRY_COUNT) MIN_YEAR=$(MIN_YEAR) MAX_YEAR=$(MAX_YEAR) ./run-tool.sh fetch

src/countries.c: resources/countries.csv
	COUNTRY_COUNT=$(COUNTRY_COUNT) MIN_YEAR=$(MIN_YEAR) MAX_YEAR=$(MAX_YEAR) ./run-tool.sh country

src/zones.c: resources/countries.csv
	COUNTRY_COUNT=$(COUNTRY_COUNT) MIN_YEAR=$(MIN_YEAR) MAX_YEAR=$(MAX_YEAR) ./run-tool.sh timezone

$(BUILD_DIR)/example: example.c $(SRC)
	@mkdir -p $(BUILD_DIR)
	cc -Wall -o $(BUILD_DIR)/example example.c -g $(SRC)

example: $(BUILD_DIR)/example

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(BUILD_DIR)
	cc -Os -Wall -fPIC -c $< -o $@

$(BUILD_DIR)/libttz.so: $(OBJ)
	cc $(OBJ) -shared -fPIC -o $@

$(BUILD_DIR)/libttz.a: $(OBJ)
	ar rcs $@ $(OBJ)

clean:
	rm -rf resources/countries.csv $(SRC_GEN) $(BUILD_DIR)
