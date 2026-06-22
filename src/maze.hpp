#pragma once

#include "maze_manager.hpp"
#include "util/glfw.hpp"

#define BENCHMARK_SAMPLE_SIZE 256

#if defined(GUI)
void mazeGui(GlfwWindow &win, cl::Context &ctx, MazeManager &manager);
#endif

void dumpStatsHeader(Platform &platform, Device &device, string benchmark);

void mazeBenchmarkGenerator(MazeManager &manager);

void mazeBenchmarkSolver(MazeManager &manager);
