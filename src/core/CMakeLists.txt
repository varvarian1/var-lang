add_library(core
    lexer/lexer.cpp
	parser/parser.cpp
    interpreter/interpreter.cpp
    interpreter/visitor.cpp
)

target_include_directories(core PUBLIC
    ${PROJECT_SOURCE_DIR}/include/core
)
