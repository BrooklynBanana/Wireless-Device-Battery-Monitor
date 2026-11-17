#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>
namespace myfinder {
// Search the right half for the rightmost cluster of high-contrast glyphs, ignoring smooth backgrounds.
inline int GroupLeft(const std::uint32_t* pixels, int width, int height, int maxGap) {
    if (!pixels || width < 8 || height < 4 || maxGap < 1) return -1;
    int left = -1, blank = 0;
    for (int x = width - 3; x >= std::max(2, width / 2); --x) {
        int hits = 0;
        for (int y = 1; y < height - 1 && hits < 2; ++y) {
            auto a = pixels[y * width + x - 2], b = pixels[y * width + x + 2];
            int contrast = 0;
            for (int shift : {0, 8, 16})
                contrast = std::max(contrast, std::abs(int((a >> shift) & 255) - int((b >> shift) & 255)));
            if (contrast >= 40) ++hits;
        }
        if (hits >= 2) { left = x - 2; blank = 0; }
        else if (++blank > (left < 0 ? maxGap * 3 : maxGap)) break;
    }
    // Reaching the search boundary is ambiguous, e.g. textured wallpaper.
    return left > width / 2 ? left : -1;
}
}
