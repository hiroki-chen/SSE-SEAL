BUILD_DIR = ./build
LOG_DIR = ./log
SRC_DIR = ./src
PROTO_DIR = ./protos
GRPC_PLUGIN_DIR = /usr/local/bin
PROTOC = protoc
INCLUDE_DIR = ./include
INCLUDE_PROTO = $(INCLUDE_DIR)/proto

ifeq ($(OS),Windows_NT)

else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		CXX = g++-10
	endif
	ifeq ($(UNAME_S), Darwin)
		CXX = clang++
	endif
endif

CXXFLAGS = -std=c++17 -g -O3 -fPIE -I$(INCLUDE_DIR) -I$(INCLUDE_PROTO) -Wall \
		-Wmissing-declarations \
		-Wextra \
		-Wno-unused-parameter \
		-Wno-missing-field-initializers \
		-Wswitch \
		-Wsign-compare \
		-Wformat \
		-Wtype-limits \
		-Wno-undef

LD = -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -pthread\
           -lgrpc++_reflection\
           -ldl\
				-lpqxx\
				-lpq\
				-lsodium

BASE_SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/oram/*.cpp $(SRC_DIR)/protos/*.cpp $(SRC_DIR)/crypto/*.cpp)
CLIENT_SRC_FILES := $(BASE_SRC_FILES) $(wildcard $(SRC_DIR)/client/*.cpp) $(SRC_DIR)/test/main.cpp
SERVER_SRC_FILES := $(BASE_SRC_FILES) $(wildcard $(SRC_DIR)/server/*.cpp) $(SRC_DIR)/test/test_server.cpp $(SRC_DIR)/client/Objects.cpp
BASE_BUILD_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(BASE_SRC_FILES))
CLIENT_BUILD_FILES := $(BASE_BUILD_FILES) $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CLIENT_SRC_FILES))
SERVER_BUILD_FILES := $(BASE_BUILD_FILES) $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SERVER_SRC_FILES))
PROTO_FILES = $(wildcard $(PROTO_DIR)/*.proto)

PROTO_FLAGS = --proto_path=$(PROTO_DIR) --cpp_out=include/proto
PROTO_GRPC_FLAGS = --proto_path=$(PROTO_DIR) --plugin=protoc-gen-grpc=$(GRPC_PLUGIN_DIR)/grpc_cpp_plugin --grpc_out=include/proto

PROTOS = protos
CLIENT = client
SERVER = server

all: clean make_dir make_protos $(CLIENT) $(SERVER)

.PHONY: clean make_dir make_protos test
clean:
	$(RM) -rf $(BUILD)

make_protos:
	$(PROTOC) $(PROTO_FLAGS) $(PROTO_FILES)
	$(PROTOC) $(PROTO_GRPC_FLAGS) $(PROTO_FILES)
	mv $(INCLUDE_PROTO)/*.cc $(SRC_DIR)/protos

	for f in $(SRC_DIR)/protos/*.cc; do mv -- "$$f" "$${f%.cc}.cpp";done
make_dir:
	mkdir -p $(BUILD_DIR)/server
	mkdir -p $(BUILD_DIR)/client
	mkdir -p $(BUILD_DIR)/test
	mkdir -p $(BUILD_DIR)/oram
	mkdir -p $(BUILD_DIR)/protos
	mkdir -p $(BUILD_DIR)/crypto
	mkdir -p $(BUILD_DIR)/executable
	mkdir -p $(LOG_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(CLIENT): $(CLIENT_BUILD_FILES)
	$(CXX) -o $(BUILD_DIR)/executable/$@ $^ $(LD)

$(SERVER): $(SERVER_BUILD_FILES)
	$(CXX) -o $(BUILD_DIR)/executable/$@ $^ $(LD)