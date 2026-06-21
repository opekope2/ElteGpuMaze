#pragma once

#include "maze_manager.hpp"
#include "util/glfw.hpp"

#define BENCHMARK_SAMPLE_SIZE 256

void mazeGui(GlfwWindow &win, cl::Context &ctx, MazeManager &manager);

void dumpStatsHeader(Platform &platform, Device &device, char *benchmark);

void mazeBenchmarkGenerator(MazeManager &manager);

void mazeBenchmarkSolver(MazeManager &manager);
