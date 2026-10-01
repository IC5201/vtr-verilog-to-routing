/**
 * @file
 * @brief Implementation of the DeviceDB used by global placement.
 */

#include "device_db.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include "device_grid.h"
#include "physical_types.h"
#include "vtr_assert.h"
#include "vtr_log.h"
#include "vtr_util.h"

namespace {

FBS::TileType tile_type_from_fbs_name(const std::string& name) {
    for (size_t i = 0; i < FBS::k_tile_type_count; i++) {
        auto type = static_cast<FBS::TileType>(i);
        if (name == FBS::tile_type_name(type)) {
            return type;
        }
    }
    // Vivado dump names DSP tiles DSP_L / DSP_R; FBS uses DSP48_*.
    if (name == "DSP_L") {
        return FBS::TileType::DSP48_L;
    }
    if (name == "DSP_R") {
        return FBS::TileType::DSP48_R;
    }
    return FBS::TileType::EMPTY;
}

/**
 * @brief Fallback when no layout CSV is available: map a VTR physical tile
 *        name onto a 7-series tile kind.
 */
FBS::TileType tile_type_from_vtr_name(const std::string& name) {
    FBS::TileType exact = tile_type_from_fbs_name(name);
    if (exact != FBS::TileType::EMPTY || name == "EMPTY") {
        return exact;
    }

    std::string n = name;
    for (char& c : n) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    if (n.find("empty") != std::string::npos) {
        return FBS::TileType::EMPTY;
    }
    if (n.find("liob18") != std::string::npos) {
        return n.find("sing") != std::string::npos ? FBS::TileType::LIOB18_SING : FBS::TileType::LIOB18;
    }
    if (n.find("riob18") != std::string::npos) {
        return n.find("sing") != std::string::npos ? FBS::TileType::RIOB18_SING : FBS::TileType::RIOB18;
    }
    if (n.find("liob33") != std::string::npos || n.find("liob") != std::string::npos) {
        return n.find("sing") != std::string::npos ? FBS::TileType::LIOB33_SING : FBS::TileType::LIOB33;
    }
    if (n.find("riob33") != std::string::npos || n.find("riob") != std::string::npos) {
        return n.find("sing") != std::string::npos ? FBS::TileType::RIOB33_SING : FBS::TileType::RIOB33;
    }
    if (n.find("io") != std::string::npos || n.find("pad") != std::string::npos) {
        return FBS::TileType::LIOB33;
    }
    if (n.find("dsp48_l") != std::string::npos || n.find("dsp_l") != std::string::npos) {
        return FBS::TileType::DSP48_L;
    }
    if (n.find("dsp") != std::string::npos || n.find("mult") != std::string::npos) {
        return FBS::TileType::DSP48_R;
    }
    if (n.find("bram_l") != std::string::npos) {
        return FBS::TileType::BRAM_L;
    }
    if (n.find("mem") != std::string::npos || n.find("ram") != std::string::npos) {
        return FBS::TileType::BRAM_R;
    }
    if (n.find("clblm_r") != std::string::npos) {
        return FBS::TileType::CLBLM_R;
    }
    if (n.find("clblm") != std::string::npos) {
        return FBS::TileType::CLBLM_L;
    }
    if (n.find("clbll_r") != std::string::npos) {
        return FBS::TileType::CLBLL_R;
    }
    if (n.find("clbll") != std::string::npos || n.find("clb") != std::string::npos
        || n.find("lab") != std::string::npos) {
        return FBS::TileType::CLBLL_L;
    }
    return FBS::TileType::EMPTY;
}

std::string find_layout_csv() {
    if (const char* env = std::getenv("GP_LAYOUT_CSV")) {
        if (vtr::file_exists(env)) {
            return env;
        }
        VTR_LOG_WARN("GP_LAYOUT_CSV is set to '%s' but the file does not exist.\n", env);
    }
    static const char* k_candidates[] = {
        "layout_gp.csv",
        "device_grid.csv",
        "data/processed/layout_gp.csv",
        "data/processed/device_grid.csv",
    };
    for (const char* path : k_candidates) {
        if (vtr::file_exists(path)) {
            return path;
        }
    }
    return {};
}

/// @brief Strip CR/LF/spaces so Windows CSV headers still match `tile`.
std::string trim_csv_field(std::string field) {
    while (!field.empty() && (field.back() == '\r' || field.back() == '\n' || field.back() == ' ')) {
        field.pop_back();
    }
    size_t start = 0;
    while (start < field.size() && field[start] == ' ') {
        start++;
    }
    return field.substr(start);
}

/// @brief Split one CSV row on commas and trim each field.
std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) {
        fields.push_back(trim_csv_field(field));
    }
    return fields;
}

int find_column(const std::vector<std::string>& header, const std::string& name) {
    for (size_t i = 0; i < header.size(); i++) {
        if (header[i] == name) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

/**
 * @brief Overlay Xilinx tile kinds from a processed layout CSV.
 *
 * Accepts either `device_grid.csv` (`x,y,tile`) or `layout_gp.csv`
 * (`vpr_x,vpr_y,...,fbs_from_xilinx`).
 */
size_t apply_layout_csv(vtr::NdMatrix<PLACE::GridSite, 2>& grid,
                        size_t width,
                        size_t height,
                        const std::string& csv_path) {
    std::ifstream in(csv_path);
    if (!in) {
        VTR_LOG_WARN("Could not open GP layout CSV '%s'.\n", csv_path.c_str());
        return 0;
    }

    std::string header_line;
    if (!std::getline(in, header_line)) {
        return 0;
    }
    std::vector<std::string> header = split_csv_line(header_line);
    int x_col = find_column(header, "x");
    int y_col = find_column(header, "y");
    int tile_col = find_column(header, "tile");
    if (x_col < 0)
        x_col = find_column(header, "vpr_x");
    if (y_col < 0)
        y_col = find_column(header, "vpr_y");
    if (tile_col < 0)
        tile_col = find_column(header, "fbs_from_xilinx");
    if (x_col < 0 || y_col < 0 || tile_col < 0) {
        VTR_LOG_WARN("GP layout CSV '%s' is missing x/y/tile columns.\n", csv_path.c_str());
        return 0;
    }

    size_t applied = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        std::vector<std::string> fields = split_csv_line(line);
        int max_col = std::max(x_col, std::max(y_col, tile_col));
        if (static_cast<int>(fields.size()) <= max_col) {
            continue;
        }
        size_t x = static_cast<size_t>(std::stoul(fields[x_col]));
        size_t y = static_cast<size_t>(std::stoul(fields[y_col]));
        if (x >= width || y >= height) {
            continue;
        }
        FBS::TileType type = tile_type_from_fbs_name(fields[tile_col]);
        if (type == FBS::TileType::EMPTY && fields[tile_col] != "EMPTY") {
            VTR_LOG_WARN("Unknown FBS tile name '%s' at (%zu,%zu) in %s.\n",
                         fields[tile_col].c_str(), x, y, csv_path.c_str());
            continue;
        }
        grid[x][y].type = type;
        applied++;
    }
    return applied;
}

} // namespace

namespace PLACE {

DeviceDB DeviceDB::from_vtr_device(const DeviceGrid& device_grid) {
    DeviceDB db;
    size_t num_layers = 0;
    std::tie(num_layers, db.width_, db.height_) = device_grid.dim_sizes();
    VTR_ASSERT_MSG(num_layers >= 1, "Device grid must have at least one layer");

    db.grid_.resize({db.width_, db.height_});
    for (size_t x = 0; x < db.width_; x++) {
        for (size_t y = 0; y < db.height_; y++) {
            t_physical_tile_loc tile_loc(static_cast<int>(x), static_cast<int>(y), 0);
            const t_physical_tile_type* tile_type = device_grid.get_physical_type(tile_loc);
            VTR_ASSERT(tile_type != nullptr);

            GridSite& site = db.grid_[x][y];
            site.type = tile_type_from_vtr_name(tile_type->name);
            site.width_offset = device_grid.get_width_offset(tile_loc);
            site.height_offset = device_grid.get_height_offset(tile_loc);
            site.tile_width = tile_type->width;
            site.tile_height = tile_type->height;
        }
    }

    const std::string csv_path = find_layout_csv();
    if (!csv_path.empty()) {
        size_t applied = apply_layout_csv(db.grid_, db.width_, db.height_, csv_path);
        VTR_LOG("DeviceDB overlayed %zu tiles from %s\n", applied, csv_path.c_str());
    } else {
        VTR_LOG_WARN("No GP layout CSV found (set GP_LAYOUT_CSV). "
                     "DeviceDB is using VTR tile-name heuristics.\n");
    }

    std::vector<unsigned> counts(FBS::k_tile_type_count, 0);
    for (size_t x = 0; x < db.width_; x++) {
        for (size_t y = 0; y < db.height_; y++) {
            counts[static_cast<size_t>(db.grid_[x][y].type)]++;
        }
    }
    VTR_LOG("DeviceDB tile counts:");
    for (size_t i = 0; i < FBS::k_tile_type_count; i++) {
        if (counts[i] == 0) {
            continue;
        }
        VTR_LOG(" %s=%u", FBS::tile_type_name(static_cast<FBS::TileType>(i)), counts[i]);
    }
    VTR_LOG("\n");

    return db;
}

const GridSite& DeviceDB::grid_site(size_t x, size_t y) const {
    VTR_ASSERT(x < width_ && y < height_);
    return grid_[x][y];
}

bool DeviceDB::is_root_tile(size_t x, size_t y) const {
    const GridSite& site = grid_site(x, y);
    return site.width_offset == 0 && site.height_offset == 0;
}

} // namespace PLACE
