cmake_minimum_required(VERSION 3.12)
project(BlockchainVotingSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(blockchain_voting main.cpp ${SOURCES})

# Threading support for POSIX HttpServer
find_package(Threads REQUIRED)
target_link_libraries(blockchain_voting PRIVATE Threads::Threads)
