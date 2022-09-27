cmake_minimum_required(VERSION 3.16.3)

project(test-coroutine C)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_EXPORT_COMPILE_COMMANDS true)

file(GLOB SRCS *)

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME}
    coroutine
    )

add_subdirectory(coroutine)
