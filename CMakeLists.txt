cmake_minimum_required(VERSION 3.10)
project(var VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS YES)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

add_library(core STATIC
    src/core/lexer/lexer.cpp
    src/core/parser/parser.cpp
    src/core/interpreter/interpreter.cpp
    src/core/interpreter/visitor.cpp
)
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core
)

add_library(utils STATIC
    src/utils/cli/arg_parser.cpp
    src/utils/logger/logger.cpp
)
target_include_directories(utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
)

target_link_libraries(utils PUBLIC core)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC utils)

if(WIN32)
    message(STATUS "Running on Windows")
elseif(UNIX)
    message(STATUS "Running on Linux")
endif()
