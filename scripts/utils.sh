#!/bin/bash

SCRIPT_DIR=$(dirname "$0")

function build() {
    echo "Building the project..."
    if [ -d "$SCRIPT_DIR/../build" ]; then
        rm -rf $SCRIPT_DIR/../build
    fi

    cmake -B $SCRIPT_DIR/../build $SCRIPT_DIR/.. -DCMAKE_BUILD_TYPE=Debug
    make -C $SCRIPT_DIR/../build -j$(nproc)

    echo "Build completed successfully."
}

function run() {
    echo "Running the project..."
    if [ ! -f "$SCRIPT_DIR/../output/var" ]; then
        echo "Executable not found. Please build the project first."
        exit 1
    fi

    $SCRIPT_DIR/../output/var $SCRIPT_DIR/../examples/test.var
}

function main() {
    case "$1" in
        build)
            build
            ;;
        run)
            run
            ;;
        *)
            echo "Usage: $0 {build|run}"
            exit 1
            ;;
    esac
}

main "$@"