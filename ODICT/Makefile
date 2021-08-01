BUILD_DIR = ./build
LOG_DIR = ./log
SRC_DIR = ./src
CXX = g++
CXXFLAGS = -std=c++17 -g -O3 -fPIE -Iinclude -Wall \
		-Wmissing-declarations \
		-Wextra \
		-Wno-unused-parameter \
		-Wno-missing-field-initializers \
		-Wswitch \
		-Wsign-compare \
		-Wformat \
		-Wtype-limits \
		-Wundef \
		-Werror \
		-Wmissing-prototypes \
		-Wstrict-prototypes
LD = -lsodium

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
BUILD_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

RUN = run

all: clean make_dir $(RUN)

.PHONY: clean make_dir
clean:
	$(RM) $(BUILD_DIR)/*.o *~ $(RUN)
	$(RM) $(LOG_DIR)/*

make_dir:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(LOG_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(RUN): $(BUILD_FILES)
	$(CXX) -o $(BUILD_DIR)/$@ $^ $(LD)
	$(BUILD_DIR)/$@