#include "kernels.hpp"
#include "maze.hpp"
#include <CL/opencl.hpp>
#include <exception>
#include <format>
#include <iostream>

using namespace std;
using namespace cl;

int main(int argc, char **argv) {
  try {
    // TODO handle multiple platforms, multiple devices
    Device dev = Device::getDefault();
    MazeApp maze(dev);
    maze.run();

    return 0;
  } catch (const Error &e) {
    cerr << format("OpenCL error: {} ({})", e.what(), e.err()) << endl;
  } catch (const exception &e) {
    cerr << format("Fatal error: {}", e.what()) << endl;
  }

  return 1;
}
