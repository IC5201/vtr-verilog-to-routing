/**
 * @file
 * @brief Pack patterns and the legalization groups they imply.
 */

#include "gp_pack_patterns.h"
#include <queue>
#include <unordered_set>
#include "vtr_assert.h"

std::vector<t_gp_pack_pattern> make_gp_pack_patterns() {
    std::vector<t_gp_pack_pattern> pats;

    // Same-slice LUT.O6 → FF.D (the 7-series BLE).
    pats.push_back(t_gp_pack_pattern{
        "ble_lut_ff",
        false,
        {
            {0, e_gp_dev_bel::LUT, {"LUT6", "LUT5", "LUT4"}, FBS::SiteType::SLICEL, nullptr},
            {1, e_gp_dev_bel::FF, {"FDRE", "FDSE", "FDCE", "FDPE"}, FBS::SiteType::SLICEL, nullptr},
        },
        {{0, 1, "O6", "D"}},
        {0, 0}});

    // CARRY4 chain. LUT/FF are optional; CO→CI extends the chain.
    pats.push_back(t_gp_pack_pattern{
        "carry4_chain",
        true,
        {
            {0, e_gp_dev_bel::CARRY_CHAIN, {"CARRY4"}, FBS::SiteType::SLICEL, "CARRY4"},
            {1, e_gp_dev_bel::LUT, {"LUT6"}, FBS::SiteType::SLICEL, nullptr},
            {2, e_gp_dev_bel::FF, {"FDRE"}, FBS::SiteType::SLICEL, nullptr},
        },
        {
            {0, 0, "CO", "CI"},
            {1, 0, "O6", "S"},
        },
        {0, 1, 1}});

    // Latch is the alternate FF primitive in the same slice.
    pats.push_back(t_gp_pack_pattern{
        "ble_lut_latch",
        false,
        {
            {0, e_gp_dev_bel::LUT, {"LUT6", "LUT5", "LUT4"}, FBS::SiteType::SLICEL, nullptr},
            {1, e_gp_dev_bel::LATCH, {"LDCE", "LDPE"}, FBS::SiteType::SLICEL, nullptr},
        },
        {{0, 1, "O6", "D"}},
        {0, 0}});

    // SLICEM LUT used as distributed RAM / SRL.
    pats.push_back(t_gp_pack_pattern{
        "slicem_lut_dmem",
        false,
        {
            {0, e_gp_dev_bel::LUT, {"LUT6"}, FBS::SiteType::SLICEM, nullptr},
            {1, e_gp_dev_bel::DMEM, {"RAM32M", "RAM64M", "SRL16E", "SRLC32E"}, FBS::SiteType::SLICEM, nullptr},
        },
        {},
        {0, 0}});

    // MUXF7 combines two LUTs in the same slice.
    pats.push_back(t_gp_pack_pattern{
        "muxf7",
        false,
        {
            {0, e_gp_dev_bel::LUT, {"LUT6"}, FBS::SiteType::SLICEL, nullptr},
            {1, e_gp_dev_bel::MUXF_L1, {"MUXF7"}, FBS::SiteType::SLICEL, "MUXF7"},
        },
        {{0, 1, "O6", "I0"}},
        {0, 0}});

    // MUXF8 sits on top of MUXF7.
    pats.push_back(t_gp_pack_pattern{
        "muxf8",
        false,
        {
            {0, e_gp_dev_bel::MUXF_L1, {"MUXF7"}, FBS::SiteType::SLICEL, "MUXF7"},
            {1, e_gp_dev_bel::MUXF_L2, {"MUXF8"}, FBS::SiteType::SLICEL, "MUXF8"},
        },
        {{0, 1, "O", "I0"}},
        {0, 0}});

    // DSP cascade. Same dimension twice; does not merge new kinds.
    pats.push_back(t_gp_pack_pattern{
        "dsp_cascade",
        true,
        {
            {0, e_gp_dev_bel::DSP, {"DSP48E1"}, FBS::SiteType::DSP48E1, "DSP48E1"},
            {1, e_gp_dev_bel::DSP, {"DSP48E1"}, FBS::SiteType::DSP48E1, "DSP48E1"},
        },
        {{0, 1, "PCOUT", "PCIN"}},
        {0, 1}});

    // IBUF/OBUF into ILOGIC/OLOGIC in the same IO tile.
    pats.push_back(t_gp_pack_pattern{
        "io_iddr",
        false,
        {
            {0, e_gp_dev_bel::IO, {"IBUF", "OBUF"}, FBS::SiteType::IOB33, nullptr},
            {1, e_gp_dev_bel::IOL, {"FDRE", "IDDR", "ODDR"}, FBS::SiteType::ILOGICE2, nullptr},
        },
        {{0, 1, "O", "D"}},
        {0, 0}});

    // IDELAY sits with ILOGIC in the IO tile.
    pats.push_back(t_gp_pack_pattern{
        "iol_iodelay",
        false,
        {
            {0, e_gp_dev_bel::IOL, {"IDDR"}, FBS::SiteType::ILOGICE2, nullptr},
            {1, e_gp_dev_bel::IODEL, {"IDELAYE2"}, FBS::SiteType::IDELAYE2, "IDELAYE2"},
        },
        {{1, 0, "DATAOUT", "D"}},
        {0, 0}});

    for (const t_gp_pack_pattern& pattern : pats) {
        VTR_ASSERT_MSG(pattern.optional.size() == pattern.blocks.size(),
                       "Pattern optional flags must be parallel to blocks");
        std::unordered_set<int> ids;
        for (const t_gp_pack_pattern_block& block : pattern.blocks) {
            VTR_ASSERT_MSG(block.block_id >= 0, "Pattern block_id must be non-negative");
            VTR_ASSERT_MSG(ids.insert(block.block_id).second, "Pattern block_id must be unique");
            VTR_ASSERT_MSG(block.dim != e_gp_dev_bel::UNKNOWN && block.dim != e_gp_dev_bel::NUM,
                           "Pattern slot must have a real GP_DEV_BEL");
        }
        for (const t_gp_pack_pattern_conn& conn : pattern.conns) {
            VTR_ASSERT_MSG(ids.count(conn.from_block) == 1 && ids.count(conn.to_block) == 1,
                           "Pattern connection must refer to existing block_ids");
        }
    }

    return pats;
}

/**
 * @brief Collect the distinct GP_DEV_BEL kinds used by a pattern.
 */
static std::unordered_set<size_t> dims_in_pattern(const t_gp_pack_pattern& pattern) {
    std::unordered_set<size_t> dims;
    for (const t_gp_pack_pattern_block& block : pattern.blocks) {
        if (block.dim == e_gp_dev_bel::UNKNOWN || block.dim == e_gp_dev_bel::NUM) {
            continue;
        }
        dims.insert(static_cast<size_t>(block.dim));
    }
    return dims;
}

static std::vector<std::vector<e_gp_dev_bel>> gp_dev_bel_groups_from_patterns(const std::vector<t_gp_pack_pattern>& patterns) {
    const size_t n = k_gp_dev_bel_count;
    std::vector<std::unordered_set<size_t>> adj(n);

    for (const t_gp_pack_pattern& pattern : patterns) {
        const std::unordered_set<size_t> dims = dims_in_pattern(pattern);
        if (dims.size() < 2) {
            continue;
        }
        const size_t first = *dims.begin();
        for (size_t idx : dims) {
            adj[first].insert(idx);
            adj[idx].insert(first);
        }
    }

    std::vector<int> group_of(n, -1);
    std::vector<std::vector<e_gp_dev_bel>> groups;
    std::queue<size_t> q;

    for (size_t d = 0; d < n; d++) {
        const auto bel = static_cast<e_gp_dev_bel>(d);
        if (bel == e_gp_dev_bel::UNKNOWN || bel == e_gp_dev_bel::NUM) {
            continue;
        }
        if (group_of[d] >= 0) {
            continue;
        }

        const int gid = static_cast<int>(groups.size());
        groups.emplace_back();
        group_of[d] = gid;
        q.push(d);

        while (!q.empty()) {
            const size_t u = q.front();
            q.pop();
            groups[gid].push_back(static_cast<e_gp_dev_bel>(u));
            for (size_t v : adj[u]) {
                if (group_of[v] >= 0) {
                    continue;
                }
                group_of[v] = gid;
                q.push(v);
            }
        }
    }

    return groups;
}

const std::vector<std::vector<e_gp_dev_bel>>& gp_bel_groups() {
    static const std::vector<std::vector<e_gp_dev_bel>> groups = gp_dev_bel_groups_from_patterns(make_gp_pack_patterns());
    return groups;
}
