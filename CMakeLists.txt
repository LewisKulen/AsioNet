set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-D_WIN32_WINNT=0x0A00)  # win10

cmake_minimum_required(VERSION 3.10)

project(AsioNet)

find_package(asio CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(kcp CONFIG REQUIRED)

# 这只是在学习cmake
file(GLOB_RECURSE ALL_CPP_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"

    # protoc
    "${CMAKE_CURRENT_SOURCE_DIR}/protoc/*.cc"

    # for test
    "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp"
)

add_executable(AsioNet ${ALL_CPP_SRCS})

target_compile_features(AsioNet PRIVATE cxx_std_17)

target_link_libraries(AsioNet PRIVATE asio::asio)
target_link_libraries(AsioNet PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
target_link_libraries(AsioNet PRIVATE kcp::kcp)

target_link_libraries(AsioNet PRIVATE ws2_32)
target_link_libraries(AsioNet PRIVATE mswsock)