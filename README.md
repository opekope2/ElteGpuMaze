# ELTE GPU Maze

Path finding in a maze

## Packages to install (Arch Linux)

```sh
gcc           # C++ compiler
opencl-clhpp  # OpenCL C++ headers
libglvnd      # OpenGL headers
libepoxy      # OpenGL extension loader
tinyxxd       # Kernel to header embedding
glfw          # Windowing
```

## Packages to install (Arch Linux, Windows cross compile)

```sh
mingw-w64-gcc               # C++ cross compiler
mingw-w64-opencl-headers    # OpenCL headers
mingw-w64-opencl-clhpp      # OpenCL C++ headers
mingw-w64-opencl-icd        # OpenCL ICD
mingw-w64-libepoxy          # OpenGL extension loader
mingw-w64-glfw              # Windowing
```

`mingw-w64-opencl-clhpp` is not in the AUR at time time of this commit, but it can be built from [this gist](https://gist.github.com/opekope2/8adf5b0f4ab339deacf214c6a63705e3).

## Setup

When using the clangd extension in VSCode, the `./build.sh gen_clangd` command can be used to generate a `.clangd` file, which helps with IntelliSense.

## Building (Linux)

Run `./build.sh` to build the executable. Run `./build.sh debug` to create a debug build. Run `OS=windows ./build.sh` to create a Windows build.

## Building (Windows)

`¯\_(ツ)_/¯`

## Running (Linux)

Run `bin/main`

## Running (Windows)

Run `bin\main.exe`

You'll need `libepoxy-0.dll`, `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, and `libwinpthread-1.dll` from the cross compiler toolchain.

## Usage

* ⬅️: Decrease maze width by 1
* ➡️: Increase maze width by 1
* ⬆️: Increase maze height by 1
* ⬇️: Decrease maze height by 1
* `-`: Decrease maze seed by 1
* `=`: Increase maze seed by 1
* `P`: Generate maze using Sequential Prim algorithm
* `K`: Generate maze using Parallel-Sorted Kruskal algorithm
* `B`: Generate maze using Sequential Boruvka algorithm
* `W`: Solve maze using Parallel Breadth-First Search algorithm
* `Q`: Quit

Hold `CTRL` to increase or decrease values by 10.
Hold `SHIFT` to increase or decrease values by 100.
Hold `CTRL` and `SHIFT` to increase or decrease values by 1000.

When generating or solving a maze, the application will print the time it took.

Specify the `PLATFORM` and `DEVICE` environment variables to choose OpenCL platform and device instead of the default one.
Specify the `PLATFORM=list` and `DEVICE=list` environment variables to print the available OpenCL platforms and devices.

## Benchmark

Specify the `BENCHMARK` environment variable to run a specific benchmark.
Specify `BENCHMARK=list` to show a list of all benchmarks.

### Supported environment variables

* `BENCHMARK_SIZE`: The maze (square) size to benchmark
* `BENCHMARK_WARMUP`: The number of times to run the maze generator/solver as warmup
* `BENCHMARK_SAMPLES`: The number of times to run the maze generator/solver as part of the benchmark
* `BENCHMARK_SEED_START`: The seed to start the warmup at. Benchmarking will start at seed `BENCHMARK_SEED_START+BENCHMARK_WARMUP`
