#include "thread_pool.h"

#include <algorithm>

ThreadPool::ThreadPool(unsigned int numThreads) {
  if (numThreads == 0) {
    numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;
  }
  workers_.reserve(numThreads);
  for (unsigned int i = 0; i < numThreads; ++i) {
    workers_.emplace_back([this] { workerLoop(); });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::lock_guard<std::mutex> lk(mtx_);
    stop_ = true;
    ++generation_;
  }
  cv_start_.notify_all();
  for (auto &t : workers_) {
    if (t.joinable()) t.join();
  }
}

void ThreadPool::parallelFor(std::size_t total, std::size_t chunkSize,
                             const RangeFn &fn) {
  if (total == 0) return;

  // For tiny workloads, just run on the calling thread. Synchronisation
  // overhead would dwarf the actual work.
  if (workers_.empty() || total <= chunkSize) {
    fn(0, total);
    return;
  }

  {
    std::lock_guard<std::mutex> lk(mtx_);
    jobFn_    = &fn;
    jobTotal_ = total;
    jobChunk_ = std::max<std::size_t>(1, chunkSize);
    jobCursor_.store(0, std::memory_order_relaxed);
    activeWorkers_.store(static_cast<int>(workers_.size()), std::memory_order_relaxed);
    ++generation_;
  }
  cv_start_.notify_all();

  // Wait for all workers to drain. Use a condition variable so we don't burn
  // a CPU core in a spin loop.
  std::unique_lock<std::mutex> lk(mtx_);
  cv_done_.wait(lk, [this] {
    return activeWorkers_.load(std::memory_order_acquire) == 0;
  });
  jobFn_ = nullptr;
}

void ThreadPool::workerLoop() {
  std::uint64_t lastSeen = 0;
  while (true) {
    const RangeFn *fn = nullptr;
    std::size_t total = 0;
    std::size_t chunk = 0;

    {
      std::unique_lock<std::mutex> lk(mtx_);
      cv_start_.wait(lk, [this, &lastSeen] {
        return stop_ || generation_ != lastSeen;
      });
      if (stop_) return;
      lastSeen = generation_;
      fn    = jobFn_;
      total = jobTotal_;
      chunk = jobChunk_;
    }

    if (fn) {
      // Pull chunks until the range is exhausted. Atomic cursor lets us
      // dynamically balance load across cores.
      while (true) {
        std::size_t begin = jobCursor_.fetch_add(chunk, std::memory_order_relaxed);
        if (begin >= total) break;
        std::size_t end = std::min(begin + chunk, total);
        (*fn)(begin, end);
      }
    }

    if (activeWorkers_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      std::lock_guard<std::mutex> lk(mtx_);
      cv_done_.notify_one();
    }
  }
}
