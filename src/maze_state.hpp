#pragma once

#include "util/gl.hpp"
#include "util/maze.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <cassert>
#include <utility>
#include <vector>

using namespace cl;

class MazeState {
protected:
    cl_uint _width;
    cl_uint _height;
    cl_uint _seed;

    Context &_ctx;

    Buffer _parent;
    Buffer _mazeData;
    Buffer _meet;

    cl_uint _cachedWavefrontSize;

    Buffer _size1;
    Buffer _size2;
    Buffer _v1;
    Buffer _v2;
    Buffer _ui1;
    Buffer _ui2;

public:
    MazeState(Context &ctx)
        : _ctx(ctx),
          _meet(ctx, CL_MEM_READ_WRITE, sizeof(vertex_t)),
          _size1(ctx, CL_MEM_READ_WRITE, sizeof(cl_uint)),
          _size2(ctx, CL_MEM_READ_WRITE, sizeof(cl_uint)) {}

    cl_uint width() { return _width; }
    cl_uint height() { return _height; }
    cl_uint seed() { return _seed; }

    virtual cl_uint minWidth() { return 2; }
    virtual cl_uint minHeight() { return 2; }
    virtual cl_uint maxWidth() { return CL_UINT_MAX; }
    virtual cl_uint maxHeight() { return CL_UINT_MAX; }

    Buffer &parent() { return _parent; }
    Buffer &mazeData() { return _mazeData; }
    Buffer &meet() { return _meet; }

    Buffer &size1() { return _size1; }
    Buffer &size2() { return _size2; }
    Buffer &vertex1() { return _v1; }
    Buffer &vertex2() { return _v2; }
    Buffer &uint1() { return _ui1; }
    Buffer &uint2() { return _ui2; }

    void swapWavefronts() { swap(_size1, _size2), swap(_v1, _v2); }

    cl_uint cachedWavefrontSize() { return _cachedWavefrontSize; }
    void updateWavefrontSize(CommandQueue &q) { q.enqueueReadBuffer(_size2, CL_TRUE, 0, sizeof(cl_uint), &_cachedWavefrontSize); }

    virtual void size(cl_uint width, cl_uint height) {
        if (width < minWidth() || height < minHeight() || width > maxWidth() || height > maxHeight())
            return;

        _width = width;
        _height = height;

        // TODO don't recreate each time
        _parent = Buffer(_ctx, CL_MEM_READ_WRITE, sizeof(vertex_t) * width * height);
        _mazeData = Buffer(_ctx, CL_MEM_READ_WRITE, sizeof(maze_data_t) * width * height);

        _v1 = Buffer(_ctx, CL_MEM_READ_WRITE, sizeof(vertex_t) * width * height);
        _v2 = Buffer(_ctx, CL_MEM_READ_WRITE, sizeof(vertex_t) * width * height);
        _ui1 = Buffer(_ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * width * height);
        _ui2 = Buffer(_ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * width * height);
    }

    void resize(cl_uint deltaWidth, cl_uint deltaHeight) { size(_width + deltaWidth, _height + deltaHeight); }

    void seed(cl_uint seed) { _seed = seed; }
};

#if defined(GUI)
#include <epoxy/gl.h>

class GlMazeState : public MazeState {
private:
    GLint _maxSize;

    GlTexture _tex;
    ImageGL _glImg;
    std::vector<Memory> _glObjs;

public:
    GlMazeState(Context &ctx) : MazeState(ctx), _tex(0), _glObjs(1) {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxSize);
        assert(_maxSize >= 0);
    }

    GlTexture &texture() { return _tex; }
    ImageGL &glImage() { return _glImg; }
    std::vector<Memory> &glObjs() { return _glObjs; }

    cl_uint maxWidth() override { return _maxSize; };
    cl_uint maxHeight() override { return _maxSize; };

    void size(cl_uint width, cl_uint height) override {
        MazeState::size(width, height);

        // TODO don't recreate each time
        _tex = createTexture<GL_TEXTURE_2D>();
        glTextureStorage2D(_tex, 1, GL_R8UI, _width, _height);
        glTextureParameteri(_tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(_tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        _glImg = ImageGL(_ctx, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, _tex);
        _glObjs[0] = _glImg;
    }
};
#endif
