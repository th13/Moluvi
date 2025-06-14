#ifndef FONT_MOJANGLES

#include "moluvi.h"

//--------------------------------------------------------------------------------
// MLFont: Mojangles
//--------------------------------------------------------------------------------

#define MJ_GLYPH_WIDTH 8
#define MJ_GLYPH_HEIGHT 8
#define MJ_GLYPH_SIZE (MJ_GLYPH_WIDTH * MJ_GLYPH_HEIGHT)

// clang-format off
const char glyphs_mojangles[128][MJ_GLYPH_SIZE] = {
    [' '] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['A'] = {
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['B'] = {
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['C'] = {
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['D'] = {
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['E'] = {
        0,1,1,1,1,1,0,0,
        0,1,0,0,0,0,0,0,
        0,1,1,1,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['F'] = {
        0,1,1,1,1,1,0,0,
        0,1,0,0,0,0,0,0,
        0,1,1,1,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['G'] = {
        0,0,1,1,1,1,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['H'] = {
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['I'] = {
        0,1,1,1,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,1,1,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['J'] = {
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['K'] = {
        0,1,0,0,0,1,0,0,
        0,1,0,0,1,0,0,0,
        0,1,1,1,0,0,0,0,
        0,1,0,0,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['L'] = {
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['M'] = {
        0,1,0,0,0,1,0,0,
        0,1,1,0,1,1,0,0,
        0,1,0,1,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['N'] = {
        0,1,0,0,0,1,0,0,
        0,1,1,0,0,1,0,0,
        0,1,0,1,0,1,0,0,
        0,1,0,0,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['O'] = {
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['P'] = {
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['Q'] = {
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,1,0,0,0,
        0,0,1,1,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['R'] = {
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['S'] = {
        0,0,1,1,1,1,0,0,
        0,1,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['T'] = {
        0,1,1,1,1,1,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['U'] = {
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['V'] = {
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,0,1,0,0,0,
        0,0,1,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['W'] = {
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,1,0,1,0,0,
        0,1,1,0,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['X'] = {
        0,1,0,0,0,1,0,0,
        0,0,1,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,1,0,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['Y'] = {
        0,1,0,0,0,1,0,0,
        0,0,1,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['Z'] = {
        0,1,1,1,1,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['a'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['b'] = {
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,1,1,0,0,0,
        0,1,1,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['c'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['d'] = {
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,1,1,0,1,0,0,
        0,1,0,0,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['e'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,1,0,0,
        0,1,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['f'] = {
        0,0,0,1,1,0,0,0,
        0,0,1,0,0,0,0,0,
        0,1,1,1,1,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['g'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
    },
    ['h'] = {
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,1,1,0,0,0,
        0,1,1,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['i'] = {
        0,0,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['j'] = {
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['k'] = {
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,1,0,0,0,
        0,1,0,1,0,0,0,0,
        0,1,1,0,0,0,0,0,
        0,1,0,1,0,0,0,0,
        0,1,0,0,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['l'] = {
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['m'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,1,0,1,0,0,0,
        0,1,0,1,0,1,0,0,
        0,1,0,1,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['n'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['o'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['p'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,1,1,0,0,0,
        0,1,1,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
    },
    ['q'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,0,1,0,0,
        0,1,0,0,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
    },
    ['r'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,1,1,0,0,0,
        0,1,1,0,0,1,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['s'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,
        0,1,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['t'] = {
        0,0,1,0,0,0,0,0,
        0,1,1,1,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['u'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['v'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['w'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,1,0,1,0,0,
        0,1,0,1,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['x'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,1,0,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['y'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
    },
    ['z'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,1,1,1,1,0,0,
        0,0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,1,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },

};
// clang-format on

static font_t font_mojangles = {
    .glyph_width = MJ_GLYPH_WIDTH,
    .glyph_height = MJ_GLYPH_HEIGHT,
    .glyphs = &glyphs_mojangles[0][0],
};

#endif // FONT_MOJANGLES
