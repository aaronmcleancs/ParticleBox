#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------
// A tiny persistent worker pool tuned for the "split-a-range-into-chunks"
// access pattern that dominates this simulation.
//
// Instead of using std::async which spawns and joins OS threads on every
// frame (costing 50-300us per frame just in scheduling overhead), we keep
// the workers alive for the lifetime of the program. The main thread hands
// them a callable + a range; workers grab chunks from a shared counter.
// ---------------------------------------------------------------------------

class ThreadPool {
public:
  using RangeFn = std::function<void(std::size_t /*begin*/, std::size_t /*end*/)>;

  explicit ThreadPool(unsigned int numThreads = 0);
  ~ThreadPool();

  // Run `fn` over [0, total) split into `chunkSize` chunks. Blocks until
  // every chunk has finished. Workers steal the next chunk from an atomic
  // counter so unbalanced workloads (denser regions of the simulation)
  // don't starve idle cores.
  void parallelFor(std::size_t total, std::size_t chunkSize, const RangeFn &fn);

  unsigned int size() const { return static_cast<unsigned int>(workers_.size()); }

private:
  void workerLoop();

  std::vector<std::thread> workers_;
  std::mutex               mtx_;
  std::condition_variable  cv_start_;
  std::condition_variable  cv_done_;

  // Shared job state
  const RangeFn *jobFn_ = nullptr;
  std::size_t   jobTotal_   = 0;
  std::size_t   jobChunk_   = 0;
  std::atomic<std::size_t> jobCursor_{0};
  std::atomic<int>         activeWorkers_{0};

  bool stop_      = false;
  std::uint64_t generation_ = 0; // increments every job; workers compare against last seen
};

#endif
