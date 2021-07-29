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

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
BUILD_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

RUN = run

all: clean make_log $(RUN)

.PHONY: clean make_log
clean:
	$(RM) $(BUILD_DIR)/*.o *~ $(RUN)

make_log:
	mkdir -p $(LOG_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(RUN): $(BUILD_FILES)
	$(CXX) -o $(BUILD_DIR)/$@ $^
	$(BUILD_DIR)/$@