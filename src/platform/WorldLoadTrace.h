#pragma once

// Stage trace for world creation.
//
// World creation is the one path on the consoles whose failure mode is a hang
// rather than an exception, and its only instrumentation used to be a pair of
// memory checkpoints wrapped around the whole World constructor -- so a run that
// stopped inside it named no stage at all. Stages nest, so the trace reads as a
// tree: the constructor's steps, the chunk provider under them, the generator's
// phases under that.
//
// Two properties matter more than the log lines themselves:
//
//   - currentStage() lives in static storage and is updated without touching the
//     filesystem. Every log line on console costs an fopen/fclose (see
//     MC_LOG_COMMIT_EVERY in platform/Log.h), so a breadcrumb that exists only in
//     debug.log is a breadcrumb that can hang while being written, and the last
//     line in the file is then the line BEFORE the one that hung. This one is
//     readable from the OOM screen and from a debugger with no I/O at all.
//
//   - Each stage reports its own duration on completion, which separates "slow"
//     from "never returned" -- the question the previous log could not answer.
//
// Logging policy, deliberate because lines are expensive here:
//   level >= 2  one line per stage, on completion, with duration and heap.
//   level >= 3  an additional line on entry, so a hang names the open stage.
namespace WorldLoadTrace
{
// Brackets one world load. Resets the stage stack and the elapsed clock, and is
// what active() reports; per-chunk trace sites use it to stay quiet during
// normal play, where they would emit thousands of lines per second.
void beginLoad(const char* what);
void endLoad();
bool active();

void begin(const char* stage);
void end();

// A linear sequence of stages at one level: closes the step opened by the
// previous call and opens the next. The World constructor is a straight run of
// steps rather than a tree, and wrapping each of them in its own block just to
// get an RAII scope would restructure code that has no other reason to change.
// Stages pushed inside a step nest under it as usual.
//
// step() closes the step on top of the stack whoever opened it, so a function
// that emits steps must own the level it emits them on: open a Scope (usually
// WORLD_LOAD_STAGE at the top of the function) first. Calling step() straight
// into a caller's level would close the caller's step and misattribute both.
void step(const char* name);

// Open-stage count, and the unwind Scope uses. popTo() closes every stage above
// the given depth, so a scope that contains step() calls still leaves the stack
// where it found it.
int depth();
void popTo(int target);

// The open stages as a dotted path ("world.chunkProvider.provideChunk"), for the
// innermost stage that has begun and not yet ended. Never null; empty before the
// first load and after a clean one.
const char* currentStage();

// Milliseconds since beginLoad(), or 0 when no load is in progress.
long elapsedMs();

class Scope
{
public:
    explicit Scope(const char* stage) : entryDepth(depth()) { begin(stage); }
    ~Scope() { popTo(entryDepth); }

    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

private:
    int entryDepth;
};

class LoadScope
{
public:
    explicit LoadScope(const char* what) { beginLoad(what); }
    ~LoadScope() { endLoad(); }

    LoadScope(const LoadScope&) = delete;
    LoadScope& operator=(const LoadScope&) = delete;
};
}

// The stage name must outlive the scope; string literals only. Nothing here
// copies or allocates, which is the point on a heap under suspicion.
#define WORLD_LOAD_STAGE(stage) WorldLoadTrace::Scope mcWorldLoadStage_(stage)
#define WORLD_LOAD_SCOPE(what)  WorldLoadTrace::LoadScope mcWorldLoadScope_(what)
