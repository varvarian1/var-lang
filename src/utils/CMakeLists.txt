add_library(utils
    cli/arg_parser.cpp
	logger/logger.cpp
)

target_include_directories(utils PUBLIC
    ${PROJECT_SOURCE_DIR}/include/utils
)
