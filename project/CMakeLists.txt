# Set the minimum version of CMake required.
cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(sse_seal VERSION 1.0 LANGUAGES CXX)

include(./common.cmake)

# C++ standard settings.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE RELEASE)
# set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Check the compiler
set (SUPPRESS_WARN_FLAGS "${SUPPRESS_WARN_FLAGS} -Wno-return-type")
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(SUPPRESS_WARN_FLAGS "${SUPPRESS_WARN_FLAGS} -Wno-sign-compare \
                                                     -Wno-unused-parameter \
                                                     -Wno-nullability-completeness \
                                                     -Wno-register")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(SUPPRESS_WARN_FLAGS "${SUPPRESS_WARN_FLAGS} -Wno-sign-compare -Wno-unused-parameter")

endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} ${SUPPRESS_WARN_FLAGS}")

if ("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
    message("In-source builds are not allowed.")
endif ()

# Generated sources
set(proto_srcs "${PROJECT_SOURCE_DIR}/src/protos/seal.pb.cc")
set(proto_hdrs "${PROJECT_SOURCE_DIR}/include/proto/seal.pb.h")
set(grpc_srcs "${PROJECT_SOURCE_DIR}/src/protos/seal.grpc.pb.cc")
set(grpc_hdrs "${PROJECT_SOURCE_DIR}/include/proto/seal.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${PROJECT_SOURCE_DIR}/protos"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/protos/seal.proto"
      DEPENDS "${hw_proto}/seal.proto"
)

# Add all files.
file(GLOB_RECURSE COMPILER_SRC RELATIVE ${PROJECT_SOURCE_DIR} 
    "src/client/*.cpp"
    "src/server/*.cpp"
    "src/protos/*.cpp"
    "src/oram/*.cpp"
    "src/crypto/*.cpp"
)

add_executable(${PROJECT_NAME} ${COMPILER_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE include include/proto)
target_link_libraries(${PROJECT_NAME} PRIVATE grpc)
add_dependencies(${PROJECT_NAME} ${_PROTOBUF_PROTOC})