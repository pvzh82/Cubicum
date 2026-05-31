#pragma once
#include <cstdint>
#include <utility>

// Atlas terrain.png: 256x256px, 16 columns x 16 rows, each cell 16px
static constexpr int ATLAS_SIZE = 16;

// Returns {col, row} in the atlas for a given block type and face orientation.
// Row 0 is the top row of the PNG (v = 0..1/16 after vertical flip on load).
inline std::pair<int, int> getAtlasCell(uint8_t blockType, bool isTop, bool isBottom) {
    if (blockType == 1) {        // grass
        if (isTop)    return {0, 0};  // grass top (green)
        if (isBottom) return {2, 0};  // dirt underneath
        return {3, 0};               // grass side (brown + green strip)
    } else if (blockType == 2) { // dirt
        return {2, 0};
    } else {                     // stone (type 3) and fallback
        return {1, 0};
    }
}
