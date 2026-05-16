#pragma once

#include <CL/opencl.hpp>
#include <format>
#include <iostream>

using namespace std;
using namespace cl;

class App {
protected:
  Device dev;
  Context ctx;
  CommandQueue q;

public:
  App(Device &dev) : dev(dev), ctx(dev), q(ctx, dev) {}

public:
  virtual void run() = 0;

protected:
  inline Program buildProgramFromSource(string source) {
    Program p(ctx, source);

    try {
      p.build();
    } catch (Error &e) {
      for (auto &&[dev, log] : p.getBuildInfo<CL_PROGRAM_BUILD_LOG>()) {
        cerr << format("Build log for {}:", dev.getInfo<CL_DEVICE_NAME>())
             << endl
             << log << endl;
      }

      throw;
    }

    return p;
  }
};
