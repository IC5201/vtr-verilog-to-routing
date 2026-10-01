#pragma once
/**
 * @file
 * @brief Placement-oriented view of a 7-series device.
 *
 * Replaces `DeviceGrid` + `t_grid_tile` for global placement. GP only needs
 * tile type, root-tile offsets, and the placeable bounding box. Wire / PIP /
 * package data is not used here.
 */

#include "dev_chip.h"
#include "vtr_ndmatrix.h"

class DeviceGrid;

namespace PLACE {

/**
 * @brief One grid cell. Multi-row tiles (DSP / BRAM) store offsets from the
 *        root (bottom-left) cell, same idea as VTR `t_grid_tile`.
 */
struct GridSite {
    FBS::TileType type = FBS::TileType::EMPTY; ///< Tile kind at this cell.
    int width_offset = 0;                      ///< Offset from the root tile in x.
    int height_offset = 0;                     ///< Offset from the root tile in y.
    int tile_width = 1;                        ///< Width of the owning tile.
    int tile_height = 1;                       ///< Height of the owning tile.
};

/**
 * @brief Device grid used by the original GP legalizer stack.
 */
class DeviceDB {
  public:
    DeviceDB() = default;

    /**
     * @brief Build a DeviceDB from the VTR `DeviceGrid`, then overlay real
     *        7-series tile kinds from `layout_gp.csv` / `device_grid.csv`
     *        when present (`GP_LAYOUT_CSV` or a sibling of the arch file).
     */
    static DeviceDB from_vtr_device(const DeviceGrid& device_grid);

    /// @brief Grid width in tiles.
    size_t width() const { return width_; }

    /// @brief Grid height in tiles.
    size_t height() const { return height_; }

    /// @brief Number of layers. Always 1 for 7-series.
    size_t num_layers() const { return 1; }

    /// @brief Grid cell at (x, y).
    const GridSite& grid_site(size_t x, size_t y) const;

    /// @brief True if (x, y) is the root (bottom-left) cell of its tile.
    bool is_root_tile(size_t x, size_t y) const;

  private:
    size_t width_ = 0;
    size_t height_ = 0;
    vtr::NdMatrix<GridSite, 2> grid_;
};

} // namespace PLACE
