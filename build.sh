#!/bin/bash
set -euo pipefail

# Dirs
SRC=src
KRN="$SRC"/kernels
OUT=bin
GEN=gen

# Flags
CXXFLAGS=(-std=c++20 -lOpenCL -DCL_HPP_ENABLE_EXCEPTIONS -DCL_HPP_MINIMUM_OPENCL_VERSION=120 -DCL_HPP_TARGET_OPENCL_VERSION=300)
BUILD_FLAGS=(-O2)
DEBUG_FLAGS=(-g -O0)

# Commands
__run() (
  echo "$@"
  "$@"
)

_gen_kernels() (
  KERNELS_HPP="$GEN"/kernels.hpp
  __run rm -f "$KERNELS_HPP"

  for f in "$KRN"/*.cl; do
    FILENAME=$(basename "$f")
    FILENAME=${FILENAME%.*} # Without extension

    __run xxd -i -t -n "$FILENAME" "$f" "$GEN"/"$FILENAME".cpp
    echo "extern unsigned char $FILENAME[];" >> "$KERNELS_HPP"
    echo "extern unsigned int ${FILENAME}_len;" >> "$KERNELS_HPP"
  done
)

_build() (
  __run mkdir -p "$OUT"
  __run g++ -Wall -o "$OUT"/main "$@" ${CXXFLAGS[@]} "$SRC"/*.cpp "$GEN"/*.cpp
)

clean() (
  __run rm -rf "$OUT" "$GEN"
)

gen_kernels() (
  __run mkdir -p "$GEN"
  _gen_kernels
)

debug() (
  _gen_kernels
  _build "${DEBUG_FLAGS[@]}"
)

build() (
  _gen_kernels
  _build "${BUILD_FLAGS[@]}"
)

COMMANDS=(clean gen_kernels debug build)

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
