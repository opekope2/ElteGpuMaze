#!/bin/bash
set -euo pipefail

# Compiler
CXX=g++

# Dirs
SRC=src
KERNELS="$SRC"/kernels
SHADERS="$SRC"/shaders
OUT=bin
GEN=gen

# Flags
DEFINES=(-DCL_HPP_ENABLE_EXCEPTIONS -DCL_HPP_MINIMUM_OPENCL_VERSION=120 -DCL_HPP_TARGET_OPENCL_VERSION=300 -DCL_TARGET_OPENCL_VERSION=300 -DGLFW_INCLUDE_NONE)
CXXFLAGS=(-std=c++20)
LDFLAGS=(-lOpenCL -lepoxy -lglfw -lEGL)
BUILD_FLAGS=(-O2)
DEBUG_FLAGS=(-g -O0)

if [ "${OS:-}" = "windows" ]; then
    CXX=x86_64-w64-mingw32-g++
    LDFLAGS=(-lOpenCL -lepoxy -lglfw3 -lgdi32 -lopengl32)
fi

# Commands
__run() (
    echo "$@"
    "$@"
)

_gen_prepare() (
    __run mkdir -p "$GEN"
)

_gen_kernels() (
    KERNELS_HPP="$GEN"/kernels.hpp
    __run rm -f "$KERNELS_HPP"
    echo "#pragma once" >> "$KERNELS_HPP"

    for f in "$KERNELS"/*.cl; do
        FILENAME=$(basename "$f")
        FILENAME=${FILENAME//./_}

        __run xxd -i -t -n "$FILENAME" "$f" "$GEN"/"$FILENAME".cpp
        echo "extern unsigned char $FILENAME[];" >> "$KERNELS_HPP"
        echo "extern unsigned int ${FILENAME}_len;" >> "$KERNELS_HPP"
    done
)

_gen_shaders() (
    SHADERS_HPP="$GEN"/shaders.hpp
    __run rm -f "$SHADERS_HPP"
    echo "#pragma once" >> "$SHADERS_HPP"

    for f in "$SHADERS"/*.vert "$SHADERS"/*.frag; do
        FILENAME=$(basename "$f")
        FILENAME=${FILENAME//./_}

        __run xxd -i -t -n "$FILENAME" "$f" "$GEN"/"$FILENAME".cpp
        echo "extern unsigned char $FILENAME[];" >> "$SHADERS_HPP"
        echo "extern unsigned int ${FILENAME}_len;" >> "$SHADERS_HPP"
    done
)

_build() (
    __run "$CXX" -Wall -o "$OUT"/main "$@" "$SRC"/*.cpp "$GEN"/*.cpp ${CXXFLAGS[@]} ${LDFLAGS[@]} ${DEFINES[@]}
)

_gen_clangd() (
    echo "CompileFlags:"
    echo "  Add:"
    for flag in ${CXXFLAGS[@]} ${LDFLAGS[@]} ${DEFINES[@]}; do
        echo "    - $flag"
    done
)

gen_clangd() (
    _gen_clangd > .clangd
)

clean() (
    __run rm -rf "$OUT" "$GEN"
)

gen_kernels() (
    _gen_prepare
    _gen_kernels
)

gen_shaders() (
    _gen_prepare
    _gen_shaders
)

debug() (
    clean
    gen_kernels
    gen_shaders
    __run mkdir -p "$OUT"
    _build "${DEBUG_FLAGS[@]}"
)

build() (
    clean
    gen_kernels
    gen_shaders
    __run mkdir -p "$OUT"
    _build "${BUILD_FLAGS[@]}"
)

COMMANDS=(gen_clangd clean gen_kernels gen_shaders debug build)

# Argument processing
_process() (
    for cmd in "${COMMANDS[@]}"; do
        if [[ "$cmd" == "$1" ]]; then
        "$cmd"
        exit 0
        fi
    done

    echo "Invalid command: $1"
    exit 1
)

if [ $# -eq 0 ]; then
    _process build
else
    _process "$1"
fi
