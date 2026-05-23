#ifndef SPATIAL_HASH_H
#define SPATIAL_HASH_H

#include "config.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

// ---------------------------------------------------------------------------
// Uniform grid spatial hash with a counting-sort style build.
//
// Cells store a (start, count) pair so neighbouring particles can be walked
// contiguously out of a sortedIndices buffer. We use a power-of-two cell
// size so the grid index is just a shift.
// ---------------------------------------------------------------------------

class SpatialHash {
public:
  struct Cell {
    std::uint32_t start;
    std::uint32_t count;
  };

  SpatialHash(float width, float height, float cellSize)
      : width_(width), height_(height), cellSize_(cellSize) {
    cols_ = static_cast<int>(std::ceil(width  / cellSize));
    rows_ = static_cast<int>(std::ceil(height / cellSize));
    grid_.resize(static_cast<std::size_t>(cols_) * rows_);
    cellShift_ = static_cast<int>(std::log2(cellSize));
    if ((1 << cellShift_) != static_cast<int>(cellSize_)) {
      // non-power-of-two cell size: fall back to division
      cellShift_ = -1;
    }
  }

  int cellIndexX(float x) const {
    int v = static_cast<int>(x);
    int cx = cellShift_ >= 0 ? (v >> cellShift_) : static_cast<int>(x / cellSize_);
    if (cx < 0) cx = 0;
    if (cx >= cols_) cx = cols_ - 1;
    return cx;
  }

  int cellIndexY(float y) const {
    int v = static_cast<int>(y);
    int cy = cellShift_ >= 0 ? (v >> cellShift_) : static_cast<int>(y / cellSize_);
    if (cy < 0) cy = 0;
    if (cy >= rows_) cy = rows_ - 1;
    return cy;
  }

  void build(std::vector<std::uint32_t> &indices,
             const std::vector<float> &posX,
             const std::vector<float> &posY,
             std::size_t count) {
    // 1. Reset cell counts.
    std::fill(grid_.begin(), grid_.end(), Cell{0, 0});

    // 2. Tally per-cell occupancy.
    for (std::size_t i = 0; i < count; ++i) {
      int cx = cellIndexX(posX[i]);
      int cy = cellIndexY(posY[i]);
      ++grid_[static_cast<std::size_t>(cy) * cols_ + cx].count;
    }

    // 3. Prefix-sum to convert counts into starts.
    std::uint32_t running = 0;
    for (auto &c : grid_) {
      c.start = running;
      running += c.count;
      c.count = 0;  // reused as a write cursor in step 4
    }

    if (indices.size() < count) indices.resize(count);

    // 4. Scatter indices into their cell-local slots.
    for (std::size_t i = 0; i < count; ++i) {
      int cx = cellIndexX(posX[i]);
      int cy = cellIndexY(posY[i]);
      auto &cell = grid_[static_cast<std::size_t>(cy) * cols_ + cx];
      indices[cell.start + cell.count++] = static_cast<std::uint32_t>(i);
    }
  }

  const Cell &getCell(int x, int y) const {
    if (x < 0 || x >= cols_ || y < 0 || y >= rows_) {
      static const Cell empty{0, 0};
      return empty;
    }
    return grid_[static_cast<std::size_t>(y) * cols_ + x];
  }

  int   cols()     const { return cols_; }
  int   rows()     const { return rows_; }
  float cellSize() const { return cellSize_; }

private:
  float width_, height_;
  float cellSize_;
  int   cols_, rows_;
  int   cellShift_;
  std::vector<Cell> grid_;
};

#endif
