#pragma once

#include "benchmark.hpp"
#include "maze_manager.hpp"
#include "util/glfw.hpp"

#if defined(GUI)
void mazeGui(GlfwWindow &win, cl::Context &ctx, MazeManager &manager);
#endif

void benchmarkMazeGenerator(MazeManager &manager, benchmark::Benchmark &benchmark);

void benchmarkMazeSolver(MazeManager &manager, benchmark::Benchmark &benchmark);
