#pragma once
/**
 * @file
 * @author  Alex Singer
 * @date    February 2024
 * @brief   Mass calculation for AP blocks and tile / site types.
 *
 * This class is used to abstract away the concept of mass in the partial legalizer.
 * Mass is a multi-dimensional quantity where every AP block has mass and every
 * tile has a mass capacity.
 *
 * Outside of this class, mass is viewed as a sparse, multi-dimensional vector
 * called the PrimitiveVector. Dimensions are TypeOperate `GP_DEV_BEL`
 * (`e_gp_dev_bel`); site capacity comes from `init_v7_site_capacity()`.
 */

#include <vector>
#include "ap_netlist_fwd.h"
#include "dev_chip.h"
#include "primitive_dim_manager.h"
#include "primitive_vector.h"
#include "vtr_assert.h"
#include "vtr_vector.h"

/**
 * @brief A calculator class which computes the M-dimensional mass of AP blocks
 *        and the capacity of tiles.
 *
 * Each native primitive already carries `e_gp_dev_bel`. An AP block may contain
 * several primitives, so its mass is the sum of those kinds mapped onto
 * PrimitiveVector dims.
 *
 * Tile / site capacity comes from TypeOperate `initV7ResourceInfo`.
 */
class FlatPlacementMassCalculator {
  public:
    /**
     * @brief Construct the mass calculator.
     *
     *  @param ap_netlist
     *      Native AP blocks; each block lists `t_gp_primitive` with `e_gp_dev_bel`.
     *  @param log_verbosity
     *      The verbosity of log messages in the mass calculator.
     */
    FlatPlacementMassCalculator(const APNetlist& ap_netlist, int log_verbosity);

    /**
     * @brief Get the M-dimensional capacity of the given physical tile type.
     */
    inline const PrimitiveVector& get_physical_tile_type_capacity(FBS::TileType tile_type) const {
        size_t index = static_cast<size_t>(tile_type);
        VTR_ASSERT(index < physical_tile_type_capacity_.size());
        return physical_tile_type_capacity_[index];
    }

    /**
     * @brief Get the M-dimensional mass of the given AP block.
     */
    inline const PrimitiveVector& get_block_mass(APBlockId blk_id) const {
        VTR_ASSERT(blk_id.is_valid());
        return block_mass_[blk_id];
    }

    /**
     * @brief Get a reference to the primitive dim manager.
     */
    inline const PrimitiveDimManager& get_dim_manager() const {
        return primitive_dim_manager_;
    }

    /**
     * @brief Generate a report on the mass and capacities calculated by this
     *        class.
     */
    void generate_mass_report(const APNetlist& ap_netlist) const;

  private:
    /// @brief The capacity of each physical tile type, indexed by `FBS::TileType`.
    std::vector<PrimitiveVector> physical_tile_type_capacity_;

    /// @brief The capacity of each site type, indexed by `FBS::SiteType`.
    std::vector<PrimitiveVector> logical_block_type_capacity_;

    /// @brief The mass of each block in the AP netlist.
    vtr::vector<APBlockId, PrimitiveVector> block_mass_;

    /// @brief Named PrimitiveVector dims for reports. Index equals `e_gp_dev_bel`.
    PrimitiveDimManager primitive_dim_manager_;

    /// @brief The verbosity of log messages in the mass calculator.
    const int log_verbosity_;
};
