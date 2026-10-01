#pragma once
/**
 * @file
 * @author  Alex Singer
 * @date    March 2025
 * @brief   Declaration of a model grouper class which groups together GP_DEV_BEL
 *          kinds that must be legalized together in a flat placement.
 *
 * `e_gp_dev_bel` is identical to `PrimitiveVectorDim`, so this class is already
 * the dim grouping the partial legalizer needs. There is no second
 * PrimitiveDimGrouper projection.
 */

#include <array>
#include <vector>
#include "primitive_vector.h"
#include "type_operate.h"
#include "vtr_assert.h"
#include "vtr_range.h"
#include "vtr_strong_id.h"
#include "vtr_vector.h"
#include "vtr_vector_map.h"

/// @brief A unique ID of a group of models created by the ModelGrouper class.
typedef vtr::StrongId<struct model_group_id_tag, size_t> ModelGroupId;

/**
 * @brief Groups GP_DEV_BEL kinds that must be spread together.
 *
 * Groups are the connected components of `make_gp_pack_patterns()`. Kinds that
 * do not appear in the used-dims mask are dropped so empty groups are not
 * created.
 */
class ModelGrouper {
  public:
    // Iterator for the model group IDs
    typedef typename vtr::vector_map<ModelGroupId, ModelGroupId>::const_iterator group_iterator;

    // Range for the model group IDs
    typedef typename vtr::Range<group_iterator> group_range;

  public:
    ModelGrouper() = delete;

    /**
     * @brief Constructor. Groups are formed here from `gp_bel_groups()`.
     *
     *  @param used_dims_mask
     *      Primitive vector with 1 in each dim that should be legalized.
     *      Groups whose every kind is unused are omitted.
     *  @param log_verbosity
     *      The verbosity of log messages in the grouper class.
     */
    ModelGrouper(const PrimitiveVector& used_dims_mask, int log_verbosity);

    /**
     * @brief Returns a list of all valid group IDs.
     */
    inline group_range groups() const {
        return vtr::make_range(group_ids_.begin(), group_ids_.end());
    }

    /**
     * @brief Gets the group ID of the given primitive vector dim.
     *
     * Valid because `e_gp_dev_bel` and `PrimitiveVectorDim` share the same
     * index (`dim_from_gp_bel()`).
     */
    inline ModelGroupId get_dim_group_id(PrimitiveVectorDim dim) const {
        VTR_ASSERT_SAFE_MSG(dim.is_valid(),
                            "Cannot get the group of an invalid dim");
        return get_model_group_id(gp_bel_from_dim(dim));
    }

    /**
     * @brief Gets the primitive vector dims in the given group.
     */
    inline const std::vector<PrimitiveVectorDim>& get_dims_in_group(ModelGroupId group_id) const {
        VTR_ASSERT_SAFE_MSG(group_id.is_valid(),
                            "Invalid group id");
        VTR_ASSERT_SAFE_MSG(group_dims_[group_id].size() != 0,
                            "Group is empty");
        return group_dims_[group_id];
    }

  private:
    /**
     * @brief Gets the group ID of the given GP_DEV_BEL.
     */
    inline ModelGroupId get_model_group_id(e_gp_dev_bel bel) const {
        VTR_ASSERT_SAFE_MSG(bel != e_gp_dev_bel::UNKNOWN && bel != e_gp_dev_bel::NUM,
                            "GP_DEV_BEL index outside of range for model_group_id_");
        ModelGroupId group_id = model_group_id_[static_cast<size_t>(bel)];
        VTR_ASSERT_SAFE_MSG(group_id.is_valid(),
                            "GP_DEV_BEL is not in a group");
        return group_id;
    }

    /// @brief List of all group IDs.
    vtr::vector_map<ModelGroupId, ModelGroupId> group_ids_;

    /// @brief A lookup between GP_DEV_BEL and the group ID that contains them.
    std::array<ModelGroupId, k_gp_dev_bel_count> model_group_id_{};

    /// @brief A lookup between each group ID and the GP_DEV_BEL kinds in that group.
    vtr::vector<ModelGroupId, std::vector<e_gp_dev_bel>> groups_;

    /// @brief Same groups as `groups_`, stored as PrimitiveVectorDim.
    vtr::vector<ModelGroupId, std::vector<PrimitiveVectorDim>> group_dims_;
};
