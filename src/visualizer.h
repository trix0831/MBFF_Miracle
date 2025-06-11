#pragma once

#include <cairo/cairo.h>
#include <string>
#include <map>
#include <vector>
#include <array>
#include <cstdint>
#include <unordered_map>
#include "MBFFOptimizer.h"
#include "CellLibrary.h"

using Coord = int32_t;

struct Point {
    Coord x, y;
};

struct Rect {
    Coord xl, yl, xh, yh;
};

enum class MacroCat { FSDNQ = 0, FSDN2, FSDN4, FSDN, OTHER };

// Categorize a macro name into predefined types
MacroCat categorizeMacro(const std::string& refName);

// Get RGB color of the corresponding macro category
std::array<double, 3> colourOf(MacroCat c);

// Plot outline of the chip
void plotOutline(cairo_t* cr, int width, int height);

// Plot a macro at the given position and color
void plotMacro(cairo_t* cr, const Point& originDBU,
               double widthUm, double heightUm,
               const std::array<double, 3>& fillRGB);

// Main plotting function for FF and gate instances
void plot(int width, int height,
          std::unordered_map<std::string, CellLibrary*> name2ffLib,
          std::unordered_map<std::string, CellLibrary*> name2gateLib,
          std::unordered_map<std::string, Instance*> name2inst_ff,
          std::unordered_map<std::string, Instance*> name2inst_gate);
