#pragma once
/**
 * @file
 * @brief 7-series TypeOperate helpers used by global placement.
 *
 * Port of PLACE::TypeOperate (Argo) for the V7 / Singutheo family:
 *   - `GP_DEV_BEL` → `e_gp_dev_bel` (GP resource dimensions)
 *   - `isIoTileType`
 *   - `initV7ResourceInfo` site capacities
 *
 * 2V / clock-bus / IO-standard helpers are not ported; GP does not use them.
 */

#include <array>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <string>
#include <vector>
#include "dev_chip.h"
#include "primitive_vector_fwd.h"
#include "vtr_assert.h"

/**
 * @brief GP resource kinds. Same catalog as TypeOperate `GP_DEV_BEL`.
 *
 * Each enumerator is a `PrimitiveVectorDim` index (`dim_from_gp_bel` /
 * `gp_bel_from_dim`). These are not FBS site/tile kinds and not netlist
 * primitive names.
 */
enum class e_gp_dev_bel : uint8_t {
    UNKNOWN = 0, ///< GP_DEV_UNKNOWN
    LUT,         ///< GP_DEV_LUT
    DMEM,        ///< GP_DEV_DMEM
    FF,          ///< GP_DEV_FF
    LATCH,       ///< GP_DEV_LATCH
    CARRY_CHAIN, ///< GP_DEV_CARRY_CHAIN
    MUXF_L1,     ///< GP_DEV_MUXF_L1 (MUXF7)
    MUXF_L2,     ///< GP_DEV_MUXF_L2 (MUXF8)
    IO,          ///< GP_DEV_IO
    IOL,         ///< GP_DEV_IOL
    IODEL,       ///< GP_DEV_IODEL
    BRAM,        ///< GP_DEV_BRAM
    DSP,         ///< GP_DEV_DSP
    NUM          ///< GP_DEV_NUM
};

/// @brief Number of GP_DEV_BEL kinds, including UNKNOWN.
inline constexpr size_t k_gp_dev_bel_count = static_cast<size_t>(e_gp_dev_bel::NUM);

/**
 * @brief Printable name. Matches TypeOperate `enum2str(GP_DEV_BEL)`.
 */
inline const char* gp_dev_bel_name(e_gp_dev_bel type) {
    switch (type) {
        case e_gp_dev_bel::UNKNOWN:
            return "unknown";
        case e_gp_dev_bel::LUT:
            return "lut";
        case e_gp_dev_bel::DMEM:
            return "lut-or-mem";
        case e_gp_dev_bel::FF:
            return "ff";
        case e_gp_dev_bel::LATCH:
            return "latch-or-ff";
        case e_gp_dev_bel::CARRY_CHAIN:
            return "carry4";
        case e_gp_dev_bel::MUXF_L1:
            return "muxf7";
        case e_gp_dev_bel::MUXF_L2:
            return "muxf8";
        case e_gp_dev_bel::IO:
            return "io";
        case e_gp_dev_bel::IOL:
            return "iol";
        case e_gp_dev_bel::IODEL:
            return "iodelay";
        case e_gp_dev_bel::BRAM:
            return "bram";
        case e_gp_dev_bel::DSP:
            return "dsp";
        case e_gp_dev_bel::NUM:
            return "";
        default:
            return "invalid_type";
    }
}

/**
 * @brief Identity: PrimitiveVector dim index equals the `e_gp_dev_bel` value.
 *
 * UNKNOWN is dim 0 and unused. NUM is not a dim.
 */
inline PrimitiveVectorDim dim_from_gp_bel(e_gp_dev_bel bel) {
    return PrimitiveVectorDim(static_cast<size_t>(bel));
}

/// @brief Inverse of `dim_from_gp_bel()`.
inline e_gp_dev_bel gp_bel_from_dim(PrimitiveVectorDim dim) {
    return static_cast<e_gp_dev_bel>(size_t(dim));
}

/**
 * @brief One placeable primitive on an AP block.
 *
 * Native netlists already carry `e_gp_dev_bel` on each cell. `bram_mass` is 1
 * for RAMB18 and 2 for RAMB36 (`initV7ResourceInfo` units).
 */
struct t_gp_primitive {
    e_gp_dev_bel bel = e_gp_dev_bel::UNKNOWN; ///< GP resource kind.
    float bram_mass = 1.0f;                   ///< Extra BRAM weight; ignored for other kinds.
};

/**
 * @brief TypeOperate `isIoTileType()`.
 */
inline bool is_io_tile_type(FBS::TileType tile_type) {
    switch (tile_type) {
        case FBS::TileType::LIOB18:
        case FBS::TileType::LIOB18_SING:
        case FBS::TileType::LIOB33:
        case FBS::TileType::LIOB33_SING:
        case FBS::TileType::RIOB18:
        case FBS::TileType::RIOB18_SING:
        case FBS::TileType::RIOB33:
        case FBS::TileType::RIOB33_SING:
            return true;
        default:
            return false;
    }
}

/**
 * @brief TypeOperate `initV7ResourceInfo()`.
 *
 * Indexed by `FBS::SiteType`. SLICEL is the SLICEM table without DMEM
 * (UG474; SLICEL was not on the visible TypeOperate screenshot).
 */
inline void init_v7_site_capacity(std::vector<std::map<e_gp_dev_bel, float>>& site_capacity) {
    site_capacity.assign(FBS::k_site_type_count, {});

    auto set = [&](FBS::SiteType site, e_gp_dev_bel bel, float val) {
        site_capacity[static_cast<size_t>(site)][bel] = val;
    };

    set(FBS::SiteType::DSP48E1, e_gp_dev_bel::DSP, 1.0f);
    set(FBS::SiteType::RAMB18E1, e_gp_dev_bel::BRAM, 1.0f);
    set(FBS::SiteType::RAMB36E1, e_gp_dev_bel::BRAM, 2.0f);
    set(FBS::SiteType::RAMBFIFO36E1, e_gp_dev_bel::BRAM, 2.0f);

    set(FBS::SiteType::IDELAYE2, e_gp_dev_bel::IODEL, 1.0f);
    set(FBS::SiteType::ODELAYE2, e_gp_dev_bel::IODEL, 1.0f);
    set(FBS::SiteType::ILOGICE2, e_gp_dev_bel::IOL, 1.0f);
    set(FBS::SiteType::ILOGICE3, e_gp_dev_bel::IOL, 1.0f);
    set(FBS::SiteType::OLOGICE2, e_gp_dev_bel::IOL, 1.0f);
    set(FBS::SiteType::OLOGICE3, e_gp_dev_bel::IOL, 1.0f);
    set(FBS::SiteType::ISERDESE2, e_gp_dev_bel::IOL, 1.0f);

    for (FBS::SiteType io : {FBS::SiteType::IOB,
                             FBS::SiteType::IOB18,
                             FBS::SiteType::IOB18M,
                             FBS::SiteType::IOB18S,
                             FBS::SiteType::IOB33,
                             FBS::SiteType::IOB33M,
                             FBS::SiteType::IOB33S,
                             FBS::SiteType::IOBM,
                             FBS::SiteType::IOBS}) {
        set(io, e_gp_dev_bel::IO, 1.0f);
    }

    // SLICEM from TypeOperate initV7ResourceInfo.
    set(FBS::SiteType::SLICEM, e_gp_dev_bel::LUT, 4.0f);
    set(FBS::SiteType::SLICEM, e_gp_dev_bel::DMEM, 4.0f);
    set(FBS::SiteType::SLICEM, e_gp_dev_bel::FF, 8.0f);
    set(FBS::SiteType::SLICEM, e_gp_dev_bel::LATCH, 4.0f);
    set(FBS::SiteType::SLICEM, e_gp_dev_bel::MUXF_L1, 2.0f);
    set(FBS::SiteType::SLICEM, e_gp_dev_bel::MUXF_L2, 1.0f);
    set(FBS::SiteType::SLICEM, e_gp_dev_bel::CARRY_CHAIN, 1.0f);

    // SLICEL: same as SLICEM without distributed RAM.
    set(FBS::SiteType::SLICEL, e_gp_dev_bel::LUT, 4.0f);
    set(FBS::SiteType::SLICEL, e_gp_dev_bel::FF, 8.0f);
    set(FBS::SiteType::SLICEL, e_gp_dev_bel::LATCH, 4.0f);
    set(FBS::SiteType::SLICEL, e_gp_dev_bel::MUXF_L1, 2.0f);
    set(FBS::SiteType::SLICEL, e_gp_dev_bel::MUXF_L2, 1.0f);
    set(FBS::SiteType::SLICEL, e_gp_dev_bel::CARRY_CHAIN, 1.0f);
}

/**
 * @brief Dense site-capacity row indexed by `e_gp_dev_bel`.
 */
inline std::array<float, k_gp_dev_bel_count> capacity_of_site(FBS::SiteType type) {
    static const std::vector<std::array<float, k_gp_dev_bel_count>> tables = []() {
        std::vector<std::map<e_gp_dev_bel, float>> site_cap;
        init_v7_site_capacity(site_cap);
        std::vector<std::array<float, k_gp_dev_bel_count>> dense(FBS::k_site_type_count);
        for (size_t i = 0; i < site_cap.size(); i++) {
            for (const auto& kv : site_cap[i]) {
                dense[i][static_cast<size_t>(kv.first)] = kv.second;
            }
        }
        return dense;
    }();

    size_t index = static_cast<size_t>(type);
    VTR_ASSERT(index < tables.size());
    return tables[index];
}

/**
 * @brief Add two dense GP_DEV_BEL capacity rows.
 */
inline std::array<float, k_gp_dev_bel_count> add_capacity(const std::array<float, k_gp_dev_bel_count>& a,
                                                          const std::array<float, k_gp_dev_bel_count>& b) {
    std::array<float, k_gp_dev_bel_count> out{};
    for (size_t i = 0; i < k_gp_dev_bel_count; i++) {
        out[i] = a[i] + b[i];
    }
    return out;
}

/**
 * @brief Tile capacity from the sites that sit in that tile.
 *
 * TypeOperate stores capacity per site. GP bins are one-per-root-tile, so a
 * CLBLL is two SLICEL, a CLBLM is SLICEL+SLICEM, a BRAM tile is one RAMB36.
 */
inline std::array<float, k_gp_dev_bel_count> capacity_of_tile(FBS::TileType type) {
    if (type == FBS::TileType::CLBLL_L || type == FBS::TileType::CLBLL_R) {
        auto slice = capacity_of_site(FBS::SiteType::SLICEL);
        return add_capacity(slice, slice);
    }
    if (type == FBS::TileType::CLBLM_L || type == FBS::TileType::CLBLM_R) {
        return add_capacity(capacity_of_site(FBS::SiteType::SLICEL),
                            capacity_of_site(FBS::SiteType::SLICEM));
    }
    if (type == FBS::TileType::DSP48_L || type == FBS::TileType::DSP48_R) {
        return capacity_of_site(FBS::SiteType::DSP48E1);
    }
    if (type == FBS::TileType::BRAM_L || type == FBS::TileType::BRAM_R) {
        return capacity_of_site(FBS::SiteType::RAMB36E1);
    }
    if (is_io_tile_type(type)) {
        if (type == FBS::TileType::LIOB18 || type == FBS::TileType::LIOB18_SING
            || type == FBS::TileType::RIOB18 || type == FBS::TileType::RIOB18_SING) {
            return capacity_of_site(FBS::SiteType::IOB18);
        }
        return capacity_of_site(FBS::SiteType::IOB33);
    }
    return {};
}

/**
 * @brief Mass of one GP_DEV_BEL instance in site-capacity units.
 *
 * RAMB18 is 1 BRAM and RAMB36 is 2 BRAM in `initV7ResourceInfo`. The VTR
 * model-name bridge passes `bram_mass` so a 36K atom costs 2.
 */
inline float mass_of_gp_bel(e_gp_dev_bel bel, float bram_mass = 1.0f) {
    switch (bel) {
        case e_gp_dev_bel::LUT:
        case e_gp_dev_bel::DMEM:
        case e_gp_dev_bel::FF:
        case e_gp_dev_bel::LATCH:
        case e_gp_dev_bel::CARRY_CHAIN:
        case e_gp_dev_bel::MUXF_L1:
        case e_gp_dev_bel::MUXF_L2:
        case e_gp_dev_bel::IO:
        case e_gp_dev_bel::IOL:
        case e_gp_dev_bel::IODEL:
        case e_gp_dev_bel::DSP:
            return 1.0f;
        case e_gp_dev_bel::BRAM:
            return bram_mass;
        case e_gp_dev_bel::UNKNOWN:
        case e_gp_dev_bel::NUM:
        default:
            return 0.0f;
    }
}

