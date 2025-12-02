#ifndef SPATIAL_HASH_H
#define SPATIAL_HASH_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

class SpatialHash {
public:
  struct Cell {
    uint32_t start;
    uint32_t count;
  };

  SpatialHash(float width, float height, float cellSize)
      : width(width), height(height), cellSize(cellSize) {
    cols = static_cast<int>(std::ceil(width / cellSize));
    rows = static_cast<int>(std::ceil(height / cellSize));
    grid.resize(cols * rows);
    cellShift = static_cast<int>(std::log2(cellSize));
  }

  void resize(float newWidth, float newHeight) {
    width = newWidth;
    height = newHeight;
    cols = static_cast<int>(std::ceil(width / cellSize));
    rows = static_cast<int>(std::ceil(height / cellSize));
    grid.resize(cols * rows);
  }

  // Rebuild the grid with current particle positions
  // indices: vector of particle indices that will be sorted by cell
  // positions: x and y coordinates of particles (SoA)
  void build(std::vector<uint32_t> &indices, const std::vector<float> &posX,
             const std::vector<float> &posY, size_t count) {
    std::fill(grid.begin(), grid.end(), Cell{0, 0});

    // 1. Count particles per cell
    for (size_t i = 0; i < count; ++i) {
      int cx = static_cast<int>(posX[i]) >> cellShift;
      int cy = static_cast<int>(posY[i]) >> cellShift;

      // Clamp to grid bounds
      cx = std::max(0, std::min(cols - 1, cx));
      cy = std::max(0, std::min(rows - 1, cy));

      int cellIdx = cy * cols + cx;
      grid[cellIdx].count++;
    }

    // 2. Compute prefix sums (start indices)
    uint32_t currentStart = 0;
    for (auto &cell : grid) {
      cell.start = currentStart;
      currentStart += cell.count;
      cell.count = 0; // Reset count to use as current index in step 3
    }

    // 3. Fill sorted indices
    // We need a temporary buffer if we want to sort 'indices' in place,
    // but here we assume 'indices' is the output buffer we write to.
    // The user should pass a vector of size 'count'.
    if (indices.size() < count)
      indices.resize(count);

    for (size_t i = 0; i < count; ++i) {
      int cx = static_cast<int>(posX[i]) >> cellShift;
      int cy = static_cast<int>(posY[i]) >> cellShift;

      cx = std::max(0, std::min(cols - 1, cx));
      cy = std::max(0, std::min(rows - 1, cy));

      int cellIdx = cy * cols + cx;
      uint32_t destIdx = grid[cellIdx].start + grid[cellIdx].count++;
      indices[destIdx] = static_cast<uint32_t>(i);
    }
  }

  const Cell &getCell(int x, int y) const {
    if (x < 0 || x >= cols || y < 0 || y >= rows) {
      static Cell empty{0, 0};
      return empty;
    }
    return grid[y * cols + x];
  }

  int getCols() const { return cols; }
  int getRows() const { return rows; }
  float getCellSize() const { return cellSize; }

private:
  float width, height;
  float cellSize;
  int cols, rows;
  int cellShift;
  std::vector<Cell> grid;
};

#endif
