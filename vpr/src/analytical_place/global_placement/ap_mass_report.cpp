/**
 * @file
 * @author  Alex Singer
 * @date    May 2025
 * @brief   Implementation of the AP mass report generator.
 */

#include "ap_mass_report.h"
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include "ap_netlist.h"
#include "dev_chip.h"
#include "primitive_dim_manager.h"
#include "primitive_vector.h"
#include "vpr_error.h"
#include "vtr_time.h"
#include "vtr_vector.h"

namespace {

/**
 * @brief Prints all of the non-zero dimensions of the given primitive vector.
 */
void print_primitive_vector(std::ofstream& os,
                            const PrimitiveVector& primitive_vec,
                            const PrimitiveDimManager& dim_manager,
                            const std::string& prefix) {
    std::vector<PrimitiveVectorDim> contained_dims = primitive_vec.get_non_zero_dims();

    size_t max_model_name_len = 0;
    for (PrimitiveVectorDim dim : contained_dims) {
        std::string dim_name = dim_manager.get_dim_name(dim);
        max_model_name_len = std::max(max_model_name_len, dim_name.size());
    }

    for (PrimitiveVectorDim dim : contained_dims) {
        std::string dim_name = dim_manager.get_dim_name(dim);
        os << prefix << std::setw(max_model_name_len) << dim_name;
        os << ": " << primitive_vec.get_dim_val(dim);
        os << "\n";
    }
}

/**
 * @brief Sum the mass of every AP block in the netlist.
 */
PrimitiveVector calc_total_netlist_mass(const APNetlist& ap_netlist,
                                        const vtr::vector<APBlockId, PrimitiveVector>& block_mass) {
    PrimitiveVector total_netlist_mass;
    for (APBlockId ap_blk_id : ap_netlist.blocks()) {
        total_netlist_mass += block_mass[ap_blk_id];
    }
    return total_netlist_mass;
}

} // namespace

void generate_ap_mass_report(const std::vector<PrimitiveVector>& logical_block_type_capacities,
                             const std::vector<PrimitiveVector>& physical_tile_type_capacities,
                             const vtr::vector<APBlockId, PrimitiveVector>& block_mass,
                             const PrimitiveDimManager& dim_manager,
                             const APNetlist& ap_netlist) {

    vtr::ScopedStartFinishTimer timer("Generating AP Mass Report");

    std::string mass_report_file_name = "ap_mass.rpt";
    std::ofstream os(mass_report_file_name);
    if (!os.is_open()) {
        VPR_FATAL_ERROR(VPR_ERROR_AP,
                        "Unable to open AP mass report file");
        return;
    }

    os << "=================================================================\n";
    os << "GP resource dimensions (e_gp_dev_bel / TypeOperate GP_DEV_BEL):\n";
    os << "=================================================================\n";
    for (PrimitiveVectorDim dim : dim_manager.dims()) {
        os << "\t" << dim_manager.get_dim_name(dim) << "\n";
    }
    os << "\n";

    os << "=================================================================\n";
    os << "Site type capacities:\n";
    os << "=================================================================\n";
    for (size_t i = 0; i < logical_block_type_capacities.size(); i++) {
        os << FBS::site_type_name(static_cast<FBS::SiteType>(i)) << ":\n";
        print_primitive_vector(os, logical_block_type_capacities[i], dim_manager, "\t");
        os << "\n";
    }

    os << "=================================================================\n";
    os << "Tile type capacities:\n";
    os << "=================================================================\n";
    for (size_t i = 0; i < physical_tile_type_capacities.size(); i++) {
        os << FBS::tile_type_name(static_cast<FBS::TileType>(i)) << ":\n";
        print_primitive_vector(os, physical_tile_type_capacities[i], dim_manager, "\t");
        os << "\n";
    }

    PrimitiveVector total_netlist_mass = calc_total_netlist_mass(ap_netlist, block_mass);
    os << "=================================================================\n";
    os << "Netlist mass by dimension:\n";
    os << "=================================================================\n";
    print_primitive_vector(os, total_netlist_mass, dim_manager, "\t");
}
