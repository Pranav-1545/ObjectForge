cmake_minimum_required(VERSION 3.25)
project(ObjectForge LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Required Packages via vcpkg
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL)
find_package(OpenCV REQUIRED)
find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)

# Protobuf & gRPC generation setup
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proto/pipeline.proto")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

set(PROTO_SRCS "${PROTO_GEN_DIR}/pipeline.pb.cc")
set(PROTO_HDRS "${PROTO_GEN_DIR}/pipeline.pb.h")
set(GRPC_SRCS "${PROTO_GEN_DIR}/pipeline.grpc.pb.cc")
set(GRPC_HDRS "${PROTO_GEN_DIR}/pipeline.grpc.pb.h")

# Custom command to invoke protoc for C++ code generation
add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${PROTO_GEN_DIR}
         --grpc_out=${PROTO_GEN_DIR}
         --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
         -I ${CMAKE_CURRENT_SOURCE_DIR}/proto
         ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating C++ code from proto/pipeline.proto"
)

# Static library target for generated Proto/gRPC files
add_library(pipeline_proto STATIC ${PROTO_SRCS} ${GRPC_SRCS})
target_include_directories(pipeline_proto PUBLIC 
    ${PROTO_GEN_DIR}
    ${Protobuf_INCLUDE_DIRS}
)
target_link_libraries(pipeline_proto PUBLIC 
    gRPC::grpc++ 
    protobuf::libprotobuf
)

add_subdirectory(app)