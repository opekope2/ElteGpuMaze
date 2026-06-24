#pragma once

#include "bfs.hpp"
#include "boruvka.hpp"
#include "kruskal.hpp"
#include "maze_generator.hpp"
#include "maze_solver.hpp"
#include "maze_state.hpp"
#include "prim.hpp"
#include "util/cl.hpp"
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <vector>

using namespace std;
using namespace cl;

class MazeManager {
private:
    CommandQueue &_q;

    MazeState &_state;

    prim::SequentialPrim _seqPrim;
    kruskal::ParallelSortedKruskal _parKruskal;
    boruvka::SequentialBoruvka _seqBoruvka;
    MazeGenerator *_generator = &_parKruskal;
    std::vector<MazeGenerator *> _generators{&_seqPrim, &_parKruskal, &_seqBoruvka};

    bfs::ParallelBFS _parBfs;
    MazeSolver *_solver = nullptr;
    std::vector<MazeSolver *> _solvers{&_parBfs};

    bool _solved = false;
    cl_ulong _solveNs = 0;
    uint8_t _solveSpeed = 1;

public:
    MazeManager(Context &ctx, CommandQueue &q, MazeState &state)
        : _q(q),
          _state(state),
          _seqPrim(ctx),
          _parKruskal(ctx),
          _seqBoruvka(ctx),
          _parBfs(ctx) {}

    CommandQueue &queue() { return _q; }

    MazeState &state() { return _state; }

    MazeGenerator *sequentialPrim() { return &_seqPrim; }
    MazeGenerator *parallelSortedKruskal() { return &_parKruskal; }
    MazeGenerator *sequentialBoruvka() { return &_seqBoruvka; }
    MazeGenerator *generator() { return _generator; }
    std::vector<MazeGenerator *> &generators() { return _generators; }

    void generator(MazeGenerator *gen) { _generator = gen; }

    MazeSolver *parallelBfs() { return &_parBfs; }
    MazeSolver *solver() { return _solver; }
    std::vector<MazeSolver *> &solvers() { return _solvers; }

    void startSolving(MazeSolver *solver) {
        _solver = solver;
        std::vector<Event> events;
        _solver->markInitialFrontiers(_q, _state, events);
        _solveNs += getProfilingTimeNs(events);
    }

    bool solved() { return _solved; }
    bool solving() { return _solver != nullptr; }
    cl_ulong solveNs() { return _solveNs; }

    uint8_t solvingSpeed() { return _solveSpeed; }
    void solvingSpeed(uint8_t speed) { _solveSpeed = speed; }

    bool stepSolve() {
        if (_solved || !solving())
            return false;

        std::vector<Event> events;
        for (uint8_t i = _solveSpeed; i > 0; i--)
            if (_solver->stepSolve(_q, _state, events)) {
                _solved = true;
                break;
            }

        _solveNs += getProfilingTimeNs(events);
        return _solved;
    }

    void resetSolver(bool resetSolved) { _solved &= !resetSolved, _solver = nullptr, _solveNs = 0; }
};
