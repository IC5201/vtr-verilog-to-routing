/**
 * @file
 * @author  Alex Singer
 * @date    March 2025
 * @brief   Implementation of a model grouper class which groups GP_DEV_BEL
 *          kinds which must be legalized together in a flat placement.
 */

#include "model_grouper.h"
#include "gp_pack_patterns.h"
#include "vtr_log.h"

ModelGrouper::ModelGrouper(const PrimitiveVector& used_dims_mask, int log_verbosity) {
    // Groups are the connected components of make_gp_pack_patterns(), already
    // in PrimitiveVectorDim space. Drop kinds that the mask does not use.
    std::vector<std::vector<e_gp_dev_bel>> filtered_bels;
    std::vector<std::vector<PrimitiveVectorDim>> filtered_dims;

    for (const std::vector<e_gp_dev_bel>& bel_group : gp_bel_groups()) {
        std::vector<e_gp_dev_bel> used_bels;
        std::vector<PrimitiveVectorDim> used_dims;
        for (e_gp_dev_bel bel : bel_group) {
            PrimitiveVectorDim dim = dim_from_gp_bel(bel);
            if (used_dims_mask.get_dim_val(dim) == 0.0f) {
                continue;
            }
            used_bels.push_back(bel);
            used_dims.push_back(dim);
        }
        if (used_bels.empty()) {
            continue;
        }
        filtered_bels.push_back(std::move(used_bels));
        filtered_dims.push_back(std::move(used_dims));
    }

    groups_.resize(filtered_bels.size());
    group_dims_.resize(filtered_dims.size());
    for (size_t i = 0; i < filtered_bels.size(); i++) {
        ModelGroupId group_id = ModelGroupId(i);
        group_ids_.push_back(group_id);
        groups_[group_id] = std::move(filtered_bels[i]);
        group_dims_[group_id] = std::move(filtered_dims[i]);
        for (e_gp_dev_bel bel : groups_[group_id]) {
            model_group_id_[static_cast<size_t>(bel)] = group_id;
        }
    }

    if (log_verbosity >= 20) {
        for (ModelGroupId group_id : groups()) {
            const std::vector<e_gp_dev_bel>& group = groups_[group_id];
            VTR_LOG("Group %zu:\n", size_t(group_id));
            VTR_LOG("\tSize = %zu\n", group.size());
            VTR_LOG("\tContained GP_DEV_BEL:\n");
            for (e_gp_dev_bel bel : group) {
                VTR_LOG("\t\t%s\n", gp_dev_bel_name(bel));
            }
        }
    }
}
