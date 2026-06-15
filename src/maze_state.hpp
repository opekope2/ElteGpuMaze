#pragma once

#include "util/gl.hpp"
#include "util/maze.hpp"
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <vector>

using namespace cl;

class MazeState {
private:
    cl_uint _width;
    cl_uint _height;
    cl_uint _seed;

    Context &_ctx;
    GlTexture _tex;
    ImageGL _glImg;
    std::vector<Memory> _glObjs;
    Buffer _mazeData;

public:
    MazeState(Context &ctx, cl_uint width, cl_uint height, cl_uint seed) : _ctx(ctx), _tex(0), _glObjs(1) {
        this->size(width, height);
        this->seed(seed);
    }

    cl_uint width() { return _width; }
    cl_uint height() { return _height; }
    cl_uint seed() { return _seed; }

    GlTexture &texture() { return _tex; }
    ImageGL &glImage() { return _glImg; }
    std::vector<Memory> &glObjs() { return _glObjs; }
    Buffer &mazeData() { return _mazeData; }

    void size(cl_uint width, cl_uint height) {
        if (width < 2 || height < 2 || width > 1024 || height > 1024)
            return;

        _width = width;
        _height = height;

        // TODO don't recreate each time
        _tex = createTexture<GL_TEXTURE_2D>();
        glTextureStorage2D(_tex, 1, GL_R8UI, width, height);
        glTextureParameteri(_tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(_tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        _glImg = ImageGL(_ctx, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, _tex);
        _glObjs[0] = _glImg;

        _mazeData = Buffer(_ctx, CL_MEM_READ_WRITE, sizeof(maze_data_t) * width * height);
    }

    void resize(cl_uint deltaWidth, cl_uint deltaHeight) { size(_width + deltaWidth, _height + deltaHeight); }

    void seed(cl_uint seed) { _seed = seed; }
};
