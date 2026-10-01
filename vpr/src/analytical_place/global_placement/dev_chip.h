#pragma once
/**
 * @file
 * @brief 7-series tile / site catalog used by global placement.
 *
 * Enumerators follow the FBS names in the existing TypeOperate / device
 * dictionary (Argo). GP does not keep a separate `BelType` here: netlist
 * primitives map onto `e_gp_dev_bel` (TypeOperate `GP_DEV_BEL`).
 *
 * Routing tiles have no placeable capacity.
 */

#include <cstddef>
#include <cstdint>

namespace FBS {

/**
 * @brief Physical tile kinds on a 7-series grid.
 *
 * Replaces `t_physical_tile_type` for global placement. IO names match
 * TypeOperate `isIoTileType()`.
 */
enum class TileType : uint8_t {
    EMPTY = 0,
    INT,
    CLBLL_L,
    CLBLL_R,
    CLBLM_L,
    CLBLM_R,
    DSP48_L,
    DSP48_R,
    BRAM_L,
    BRAM_R,
    LIOB18,
    LIOB18_SING,
    LIOB33,
    LIOB33_SING,
    RIOB18,
    RIOB18_SING,
    RIOB33,
    RIOB33_SING,
    NUM_TYPES
};

/**
 * @brief Site kinds that sit inside a tile.
 *
 * Replaces the root `t_logical_block_type` / `t_pb_type` for GP capacity.
 * Names match TypeOperate `initV7ResourceInfo` / `isSLICESiteType` / `isIoSiteType`.
 */
enum class SiteType : uint8_t {
    NONE = 0,
    SLICEL,
    SLICEM,
    DSP48E1,
    RAMB18E1,
    RAMB36E1,
    RAMBFIFO36E1,
    IDELAYE2,
    ODELAYE2,
    ILOGICE2,
    ILOGICE3,
    OLOGICE2,
    OLOGICE3,
    ISERDESE2,
    IOB,
    IOB18,
    IOB18M,
    IOB18S,
    IOB33,
    IOB33M,
    IOB33S,
    IOBM,
    IOBS,
    BUFG,
    BUFGCTRL,
    BUFGMUX,
    TIEOFF,
    NUM_TYPES
};

/// @brief Number of tile kinds, including `EMPTY`.
inline constexpr size_t k_tile_type_count = static_cast<size_t>(TileType::NUM_TYPES);

/// @brief Number of site kinds, including `NONE`.
inline constexpr size_t k_site_type_count = static_cast<size_t>(SiteType::NUM_TYPES);

/**
 * @brief Printable name of a tile type.
 */
inline const char* tile_type_name(TileType type) {
    switch (type) {
        case TileType::EMPTY:
            return "EMPTY";
        case TileType::INT:
            return "INT";
        case TileType::CLBLL_L:
            return "CLBLL_L";
        case TileType::CLBLL_R:
            return "CLBLL_R";
        case TileType::CLBLM_L:
            return "CLBLM_L";
        case TileType::CLBLM_R:
            return "CLBLM_R";
        case TileType::DSP48_L:
            return "DSP48_L";
        case TileType::DSP48_R:
            return "DSP48_R";
        case TileType::BRAM_L:
            return "BRAM_L";
        case TileType::BRAM_R:
            return "BRAM_R";
        case TileType::LIOB18:
            return "LIOB18";
        case TileType::LIOB18_SING:
            return "LIOB18_SING";
        case TileType::LIOB33:
            return "LIOB33";
        case TileType::LIOB33_SING:
            return "LIOB33_SING";
        case TileType::RIOB18:
            return "RIOB18";
        case TileType::RIOB18_SING:
            return "RIOB18_SING";
        case TileType::RIOB33:
            return "RIOB33";
        case TileType::RIOB33_SING:
            return "RIOB33_SING";
        case TileType::NUM_TYPES:
            return "NUM_TYPES";
        default:
            return "UNKNOWN";
    }
}

/**
 * @brief Printable name of a site type.
 */
inline const char* site_type_name(SiteType type) {
    switch (type) {
        case SiteType::NONE:
            return "NONE";
        case SiteType::SLICEL:
            return "SLICEL";
        case SiteType::SLICEM:
            return "SLICEM";
        case SiteType::DSP48E1:
            return "DSP48E1";
        case SiteType::RAMB18E1:
            return "RAMB18E1";
        case SiteType::RAMB36E1:
            return "RAMB36E1";
        case SiteType::RAMBFIFO36E1:
            return "RAMBFIFO36E1";
        case SiteType::IDELAYE2:
            return "IDELAYE2";
        case SiteType::ODELAYE2:
            return "ODELAYE2";
        case SiteType::ILOGICE2:
            return "ILOGICE2";
        case SiteType::ILOGICE3:
            return "ILOGICE3";
        case SiteType::OLOGICE2:
            return "OLOGICE2";
        case SiteType::OLOGICE3:
            return "OLOGICE3";
        case SiteType::ISERDESE2:
            return "ISERDESE2";
        case SiteType::IOB:
            return "IOB";
        case SiteType::IOB18:
            return "IOB18";
        case SiteType::IOB18M:
            return "IOB18M";
        case SiteType::IOB18S:
            return "IOB18S";
        case SiteType::IOB33:
            return "IOB33";
        case SiteType::IOB33M:
            return "IOB33M";
        case SiteType::IOB33S:
            return "IOB33S";
        case SiteType::IOBM:
            return "IOBM";
        case SiteType::IOBS:
            return "IOBS";
        case SiteType::BUFG:
            return "BUFG";
        case SiteType::BUFGCTRL:
            return "BUFGCTRL";
        case SiteType::BUFGMUX:
            return "BUFGMUX";
        case SiteType::TIEOFF:
            return "TIEOFF";
        case SiteType::NUM_TYPES:
            return "NUM_TYPES";
        default:
            return "UNKNOWN";
    }
}

} // namespace FBS
