cmake_minimum_required(VERSION 3.16.3)

project(coroutine C)

file(GLOB SRCS *.c)

add_library(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE
    .
    include
    )
target_include_directories(${PROJECT_NAME} INTERFACE include)

target_link_libraries(${PROJECT_NAME})
