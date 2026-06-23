# ELTE GPU Maze

Path finding in a maze

## Dependencies

### Arch Linux

#### Make

```sh
gcc                         # C++ compiler
opencl-headers              # OpenCL headers
opencl-clhpp                # OpenCL C++ headers
tinyxxd                     # Kernel to header embedding
```

#### GUI

```sh
libglvnd                    # OpenGL headers
libepoxy                    # OpenGL extension loader
glfw                        # Windowing
```

#### Windows cross compilation

```sh
mingw-w64-gcc               # C++ cross compiler
mingw-w64-opencl-headers    # OpenCL headers
mingw-w64-opencl-clhpp      # OpenCL C++ headers
mingw-w64-opencl-icd        # OpenCL ICD
mingw-w64-libepoxy          # OpenGL extension loader
mingw-w64-glfw              # Windowing
```

`mingw-w64-opencl-clhpp` is not in the AUR at time time of this commit, but it can be built from [this gist](https://gist.github.com/opekope2/8adf5b0f4ab339deacf214c6a63705e3).

### Termux

#### Prerequisites

```sh
git                         # For compiling tinyxxd
make                        # For compiling tinyxxd
```

#### Make

```sh
clang                       # C++ compiler
opencl-headers              # OpenCL headers
opencl-clhpp                # OpenCL C++ headers
ocl-icd                     # OpenCL ICD
```

#### Runtime

```sh
clvk                        # OpenCL
vulkan-loader-android       # DO NOT USE vulkan-loader-generic, otherwise the benchmark will run on the CPU (llvmpipe)
```

## Setup

When using the clangd extension in VSCode, the `./build.sh gen_clangd` command can be used to generate a `.clangd` file, which helps with IntelliSense.

## Compiling

### Linux

Run `./build.sh` to build the executable. Run `./build.sh debug` to create a debug build. Run `OS=windows ./build.sh` to create a Windows build.

### Termux

`tinyxxd` is not packaged in Termux as of this commit, so you'll need to compile it yourself.

```sh
# 1. Clone tinyxxd
git clone https://github.com/xyproto/tinyxxd.git

# 2. Build tinyxxd
(cd tinyxxd && make)

# 3. Add tinyxxd to path
export PATH=$PATH:$PWD/tinyxxd

# 4. Clone project
git clone https://github.com/opekope2/ElteGpuMaze.git

# 5. Build project
cd ElteGpuMaze
OS=android GUI=0 ./build.sh
```

### Windows

`¯\_(ツ)_/¯`

## Running

### Linux

Run `bin/maze`

### Termux

You can only run benchmarks in Termux. Run `bin/maze`

### Windows

Run `bin\maze.exe`

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
