/**
 * @file
 * @author  Alex Singer
 * @date    February 2024
 * @brief   Implementation of the mass calculator used in the AP flow.
 *
 * Capacities come from TypeOperate `initV7ResourceInfo`. Atom masses come from
 * native `e_gp_dev_bel` on each AP-block primitive, mapped onto dims.
 */

#include "flat_placement_mass_calculator.h"
#include "ap_mass_report.h"
#include "ap_netlist.h"
#include "type_operate.h"
#include "vtr_log.h"

/**
 * @brief One PrimitiveVector dim per `e_gp_dev_bel`, in enum order.
 *
 * Dim index equals the enumerator, which is what `dim_from_gp_bel()` assumes.
 * Names are kept on the dim manager for mass-report printing.
 */
static void initialize_gp_dim_manager(PrimitiveDimManager& dim_manager) {
    for (size_t i = 0; i < k_gp_dev_bel_count; i++) {
        auto bel = static_cast<e_gp_dev_bel>(i);
        dim_manager.create_empty_dim(gp_dev_bel_name(bel));
    }
}

/**
 * @brief Build a PrimitiveVector from a dense `e_gp_dev_bel` capacity table.
 */
static PrimitiveVector primitive_vector_from_res_table(const std::array<float, k_gp_dev_bel_count>& table) {
    PrimitiveVector vec;
    for (size_t i = 0; i < k_gp_dev_bel_count; i++) {
        if (table[i] == 0.0f) {
            continue;
        }
        auto bel = static_cast<e_gp_dev_bel>(i);
        if (bel == e_gp_dev_bel::UNKNOWN) {
            continue;
        }
        vec.add_val_to_dim(table[i], dim_from_gp_bel(bel));
    }
    return vec;
}

/**
 * @brief Get the primitive mass of the given block.
 *
 * Each native primitive already carries `e_gp_dev_bel`; `dim_from_gp_bel()`
 * is the PrimitiveVector dim of that kind.
 */
static PrimitiveVector calc_block_mass(APBlockId blk_id,
                                       const APNetlist& netlist) {
    PrimitiveVector mass;
    for (const t_gp_primitive& prim : netlist.block_primitives(blk_id)) {
        if (prim.bel == e_gp_dev_bel::UNKNOWN || prim.bel == e_gp_dev_bel::NUM)
            continue;

        PrimitiveVectorDim dim = dim_from_gp_bel(prim.bel);
        mass.add_val_to_dim(mass_of_gp_bel(prim.bel, prim.bram_mass), dim);
    }
    return mass;
}

FlatPlacementMassCalculator::FlatPlacementMassCalculator(const APNetlist& ap_netlist,
                                                         int log_verbosity)
    : physical_tile_type_capacity_(FBS::k_tile_type_count)
    , logical_block_type_capacity_(FBS::k_site_type_count)
    , block_mass_(ap_netlist.blocks().size())
    , log_verbosity_(log_verbosity) {

    initialize_gp_dim_manager(primitive_dim_manager_);

    // Site capacity: TypeOperate initV7ResourceInfo.
    for (size_t i = 0; i < FBS::k_site_type_count; i++) {
        auto site_type = static_cast<FBS::SiteType>(i);
        logical_block_type_capacity_[i] = primitive_vector_from_res_table(capacity_of_site(site_type));
    }

    // Tile capacity: sites that occupy that tile, summed.
    for (size_t i = 0; i < FBS::k_tile_type_count; i++) {
        auto tile_type = static_cast<FBS::TileType>(i);
        physical_tile_type_capacity_[i] = primitive_vector_from_res_table(capacity_of_tile(tile_type));
    }

    // Precompute the mass of each block in the APNetlist
    VTR_LOGV(log_verbosity_ >= 10, "Pre-computing the block masses...\n");
    for (APBlockId ap_block_id : ap_netlist.blocks()) {
        block_mass_[ap_block_id] = calc_block_mass(ap_block_id, ap_netlist);
    }
    VTR_LOGV(log_verbosity_ >= 10, "Finished pre-computing the block masses.\n");
}

void FlatPlacementMassCalculator::generate_mass_report(const APNetlist& ap_netlist) const {
    generate_ap_mass_report(logical_block_type_capacity_,
                            physical_tile_type_capacity_,
                            block_mass_,
                            primitive_dim_manager_,
                            ap_netlist);
}
