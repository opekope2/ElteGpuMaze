#pragma once

#include "boruvka.hpp"
#include "maze_generator.hpp"
#include "maze_state.hpp"
#include "prim.hpp"
#include <CL/opencl.hpp>

using namespace std;

class MazeManager {
private:
    CommandQueue &_q;

    MazeState &_state;

    prim::SequentialPrim _seqPrim;
    boruvka::SequentialBoruvka _seqBoruvka;
    MazeGenerator *_generator;

public:
    MazeManager(Context &ctx, CommandQueue &q, MazeState &state)
        : _q(q),
          _state(state),
          _seqPrim(ctx),
          _seqBoruvka(ctx),
          _generator(&_seqPrim) {}

    CommandQueue &queue() { return _q; }

    MazeState &state() { return _state; }

    MazeGenerator *sequentialPrim() { return &_seqPrim; }
    MazeGenerator *sequentialBoruvka() { return &_seqBoruvka; }
    MazeGenerator *generator() { return _generator; }

    void generator(MazeGenerator *gen) { _generator = gen; }
};
