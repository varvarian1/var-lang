cmake_minimum_required(VERSION 3.10)
project(var VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS YES)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

include(GNUInstallDirs) # Provides paths: bin/, lib/, share/

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

# Installation (make install)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

if (EXISTS ${CMAKE_SOURCE_DIR}/examples)
    install(DIRECTORY examples/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/examples
        PATTERN "*.var" # Install only .var files
    )
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/README.md)
    install(FILES README.md
        DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/${PROJECT_NAME}
    )
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/LICENSE)
    install(FILES LICENSE
        DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/${PROJECT_NAME}
    )
endif()

# Install libraries (for VAR development)
install(TARGETS core utils
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

message(STATUS "Installation prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Binary directory: ${CMAKE_INSTALL_BINDIR}")
message(STATUS "Include directory: ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
message(STATUS "")
message(STATUS "To install: cd build && sudo make install")
message(STATUS "To uninstall: sudo rm -f ${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME}")