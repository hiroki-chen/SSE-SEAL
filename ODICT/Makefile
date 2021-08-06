BUILD_DIR = ./build
LOG_DIR = ./log
SRC_DIR = ./src
PROTP_DIR = ./protos
GRPC_PLUGIN_DIR = /usr/local/opt/grpc/bin/
PROTOC = protoc
CXX = g++
CXXFLAGS = -std=c++17 -g -O3 -fPIE -Iinclude -Wall \
		-Wmissing-declarations \
		-Wextra \
		-Wno-unknown-attributes \
		-Wno-unused-parameter \
		-Wno-missing-field-initializers \
		-Wswitch \
		-Wsign-compare \
		-Wformat \
		-Wtype-limits \
		-Wno-undef \
		-Werror \
		-Wno-missing-prototypes \
		-Wstrict-prototypes

LD = -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -pthread\
           -lgrpc++_reflection\
           -ldl\
		   -lpqxx\
		   -lpq\
		   -lsodium

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/server/*.cpp $(SRC_DIR)/client/*.cpp $(SRC_DIR)/oram/*.cpp $(SRC_DIR)/test/main.cpp $(SRC_DIR)/crypto/*.cpp)
BUILD_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
PROTO_FILES = $(wildcard $(PROTO_DIR)/*.proto)

PROTO_FLAGS = --proto_path=$(PROTO_DIR) --cpp_out=include/proto
PROTO_GRPC_FLAGS = --proto_path=$(PROTO_DIR)\ 
				   --plugin=protoc-gen-grpc=$(GRPC_PLUGIN_DIR)/grpc_cpp_plugin\
				   --grpc_out=include/proto

RUN = run

all: clean make_dir make_protos $(RUN)

.PHONY: clean make_dir make_protos test
clean:
	$(RM) $(BUILD_DIR)/*.o *~ $(RUN)
	$(RM) $(LOG_DIR)/*

make_protos:
	$(PROTOC) $(PROTO_FLAGS) $(PROTO_FILES)
	$(PROTOC) $(PROTO_GRPC_FLAGS) $(PROTO_FILES)
	mv $(PROTO_DIR)/*.cc $(SRC_DIR)/protos
make_dir:
	mkdir -p $(BUILD_DIR)/server
	mkdir -p $(BUILD_DIR)/client
	mkdir -p $(BUILD_DIR)/test
	mkdir -p $(BUILD_DIR)/oram
	mkdir -p $(BUILD_DIR)/crypto
	mkdir -p $(LOG_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(RUN): $(BUILD_FILES)
	$(CXX) -o $(BUILD_DIR)/$@ $^ $(LD)
	$(BUILD_DIR)/$@

test:
	rm -rf *.o
	clang++ -std=c++17 -c -O2 -fPIE -Iinclude src/server/*.cpp src/Connector.cpp src/oram/*.cpp src/*.cpp src/test/test_server.cpp
	clang++ -o test_server *.o $(LD)

none:
	rm -rf test_server.o
	clang++ -c test/test_client.cpp src/client/*.cpp -std=c++17 -O2 -fPIE -Iinclude
	clang++ -o test_client *.o $(LD)