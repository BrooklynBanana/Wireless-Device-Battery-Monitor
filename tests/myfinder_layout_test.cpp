#include "../src/myfinder_layout.h"
#include <cassert>
#include <cstdio>
#include <vector>

int main() {
    constexpr int w = 640, h = 24;
    std::vector<std::uint32_t> image(w * h);
    auto glyph = [&](int x, std::uint32_t color) {
        for (int y = 6; y < 18; ++y) for (int i = x; i < x + 8; ++i) image[y * w + i] = color;
    };
    glyph(40, 0xffffff); // application menu: must not affect the right-hand anchor
    for (int x = 500; x <= 620; x += 24) glyph(x, 0xffffff);
    assert(myfinder::GroupLeft(image.data(), w, h, 24) == 496);
    glyph(476, 0xffffff); // a newly appearing system indicator expands the group
    assert(myfinder::GroupLeft(image.data(), w, h, 24) == 472);
    for (auto& p : image) p ^= 0xffffff; // dark glyphs on a light bar
    assert(myfinder::GroupLeft(image.data(), w, h, 24) == 472);
    for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
        unsigned v = unsigned(x / 3); image[y * w + x] = v * 0x010101;
    }
    assert(myfinder::GroupLeft(image.data(), w, h, 24) == -1); // smooth transparency gradient
    std::fill(image.begin(), image.end(), 0);
    glyph(40, 0xffffff);
    assert(myfinder::GroupLeft(image.data(), w, h, 24) == -1); // no right-hand group
    assert(myfinder::GroupLeft(nullptr, w, h, 24) == -1);
    puts("MyFinder layout: all checks passed");
}
