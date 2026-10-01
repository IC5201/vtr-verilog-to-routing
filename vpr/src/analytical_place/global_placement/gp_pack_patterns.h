#pragma once
/**
 * @file
 * @brief Pack-pattern recipes for global placement.
 *
 * These patterns say which GP_DEV_BEL kinds may occupy the same site (and
 * which pins connect them). Legalization groups are derived from them; they
 * are also the intended input for a future prepacker.
 *
 * Named `t_gp_pack_pattern*` so they do not collide with the packer's
 * `t_pack_pattern_block` / `t_pack_patterns` in `pack/pack_patterns.h`.
 */

#include <cstdint>
#include <string>
#include <vector>
#include "type_operate.h"

/**
 * @brief One slot in a GP pack pattern (one primitive that will sit in a site).
 */
struct t_gp_pack_pattern_block {
    int block_id = -1;                          ///< Pattern-local index of this slot.
    e_gp_dev_bel dim = e_gp_dev_bel::UNKNOWN;   ///< GP dimension this slot occupies.
    std::vector<std::string> cells;             ///< Xilinx cell names that can fill this slot.
    FBS::SiteType site = FBS::SiteType::NONE;   ///< Site family this slot must occupy.
    const char* bel_name = nullptr;             ///< Fixed BEL, or nullptr for any BEL of `dim`.
};

/**
 * @brief A pin-to-pin connection between two pattern slots.
 */
struct t_gp_pack_pattern_conn {
    int from_block = -1;            ///< Source slot `block_id`.
    int to_block = -1;              ///< Sink slot `block_id`.
    const char* from_pin = nullptr; ///< Source cell pin name.
    const char* to_pin = nullptr;   ///< Sink cell pin name.
};

/**
 * @brief One architecture pack pattern (a site-level pairing recipe).
 */
struct t_gp_pack_pattern {
    std::string name;                             ///< Pattern name, for logging.
    bool is_chain = false;                        ///< True if the pattern can extend (carry / DSP cascade).
    std::vector<t_gp_pack_pattern_block> blocks;  ///< Slots in this pattern.
    std::vector<t_gp_pack_pattern_conn> conns;    ///< Required connections between slots.
    std::vector<uint8_t> optional;                ///< Parallel to `blocks`; 1 means the slot is optional.
};

/**
 * @brief Pack-pattern table. This is the source of truth for who may
 *        share a site.
 */
std::vector<t_gp_pack_pattern> make_gp_pack_patterns();

/**
 * @brief Legalization groups derived from `make_gp_pack_patterns()`.
 */
const std::vector<std::vector<e_gp_dev_bel>>& gp_bel_groups();
