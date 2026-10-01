#pragma once
/**
 * @file
 * @author  Alex Singer
 * @date    June 2025
 * @brief   Declaration of the primitive dim manager class. This class stores
 *          the named PrimitiveVector dimensions used by mass reports and the
 *          partial legalizer.
 *
 * The Mass Calculator creates one named dimension per `e_gp_dev_bel`
 * (TypeOperate `GP_DEV_BEL`). The dim index is the enumerator itself
 * (`dim_from_gp_bel()`).
 */

#include <string>
#include "primitive_vector_fwd.h"
#include "vtr_assert.h"
#include "vtr_range.h"
#include "vtr_vector_map.h"

/**
 * @brief Named PrimitiveVector dimensions for GP mass / legalization.
 *
 * Dim index equals `e_gp_dev_bel` (`dim_from_gp_bel()`). This class only
 * stores creation order and printable names; it does not map BELs onto dims.
 */
class PrimitiveDimManager {
  public:
    // Iterator for the primitive vector dims.
    typedef typename vtr::vector_map<PrimitiveVectorDim, PrimitiveVectorDim>::const_iterator dim_iterator;

    // Range for the primitive vector dims.
    typedef typename vtr::Range<dim_iterator> dim_range;

  public:
    /**
     * @brief Returns a list of all valid primitive vector dimensions.
     */
    inline dim_range dims() const {
        return vtr::make_range(dims_.begin(), dims_.end());
    }

    /**
     * @brief Create an empty primitive vector dimension with the given name.
     */
    inline PrimitiveVectorDim create_empty_dim(const std::string& name) {
        PrimitiveVectorDim new_dim = static_cast<PrimitiveVectorDim>(dims_.size());
        dims_.push_back(new_dim);
        dim_name_.push_back(name);
        return new_dim;
    }

    /**
     * @brief Get the name of the given primitive vector dim.
     */
    inline const std::string& get_dim_name(PrimitiveVectorDim dim) const {
        VTR_ASSERT_SAFE_MSG(dim.is_valid(),
                            "Cannot get the name of an invalid dim");
        return dim_name_[dim];
    }

  private:
    /// @brief All of the valid primitive vector dims.
    vtr::vector_map<PrimitiveVectorDim, PrimitiveVectorDim> dims_;

    /// @brief A lookup between primitive dims and their names.
    vtr::vector_map<PrimitiveVectorDim, std::string> dim_name_;
};
