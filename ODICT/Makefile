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
		-Wundef \
		-Werror \
		-Wmissing-prototypes \
		-Wstrict-prototypes

# Be sure export PKG_CONFIG_PATH=/PATH/TO/OPENSSL/PKGCONFIG/PATH
ifeq ($(SYSTEM),Darwin)
LD += -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -pthread\
           -lgrpc++_reflection\
           -ldl\
		   -lpqxx\
		   -lpq
else
LD += -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl\
		   -lpqxx\
		   -lpq
endif

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/server/*.cpp $(SRC_DIR)/client/*.cpp $(SRC_DIR)/oram/*.cpp)
BUILD_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
PROTO_FILES = $(wildcard $(PROTO_DIR)/*.proto)

PROTO_FLAGS = --proto_path=$(PROTO_DIR) --cpp_out=include/proto
PROTO_GRPC_FLAGS = --proto_path=$(PROTO_DIR)\ 
				   --plugin=protoc-gen-grpc=$(GRPC_PLUGIN_DIR)/grpc_cpp_plugin\
				   --grpc_out=include/proto

RUN = run

all: clean make_dir make_protos $(RUN)

.PHONY: clean make_dir make_protos
clean:
	$(RM) $(BUILD_DIR)/*.o *~ $(RUN)
	$(RM) $(LOG_DIR)/*

make_protos:
	$(PROTOC) $(PROTO_FLAGS) $(PROTO_FILES)
	$(PROTOC) $(PROTO_GRPC_FLAGS) $(PROTO_FILES)
	mv $(PROTO_DIR)/*.cc $(SRC_DIR)/protos
make_dir:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(LOG_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(RUN): $(BUILD_FILES)
	$(CXX) -o $(BUILD_DIR)/$@ $^ $(LD)
	$(BUILD_DIR)/$@