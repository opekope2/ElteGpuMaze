# ELTE GPU Prog Assignment

Path finding in a maze

## Packages to install (Arch Linux)

```sh
gcc           # C++ compiler
opencl-clhpp  # OpenCL C++ headers
libglvnd      # OpenGL headers
libepoxy      # OpenGL extension loader
xxd           # Kernel to header embedding
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
