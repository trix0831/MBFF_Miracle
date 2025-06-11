// ─────────────────────────────────── visualizer.cpp ───────────────────────────────────
#include <cairo/cairo.h>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <array>

#include "lef_parser/lef_parser.hpp"
#include "def_parser/def_parser.hpp"

enum class MacroCat { FSDNQ = 0, FSDN2, FSDN4, FSDN, OTHER };

static inline MacroCat categorizeMacro(const std::string& refName)
{
    if (refName.find("FSDNQ") != std::string::npos &&
        refName.find("FSDNQ") == refName.find("FSDN")) return MacroCat::FSDNQ;
    if (refName.find("FSDN2") != std::string::npos &&
        refName.find("FSDN2") == refName.find("FSDN")) return MacroCat::FSDN2;
    if (refName.find("FSDN4") != std::string::npos &&
        refName.find("FSDN4") == refName.find("FSDN")) return MacroCat::FSDN4;
    if (refName.find("FSDN") != std::string::npos) return MacroCat::FSDN;
    return MacroCat::OTHER;
}

static inline std::array<double,3> colourOf(MacroCat c)
{
    switch (c) {
        case MacroCat::FSDNQ: return {0.0, 0.4, 0.8};
        case MacroCat::FSDN2: return {0.0, 0.6, 0.0};
        case MacroCat::FSDN4: return {0.5, 0.0, 0.5};
        case MacroCat::FSDN:  return {0.9, 0.9, 0.0};
        default:              return {0.6, 0.6, 0.6};
    }
}

void plotOutline(cairo_t* cr, const Rect& die)
{
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_rectangle(cr, 0, 0, die.xh / 1000.0, die.yh / 1000.0);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr,
                    die.xl / 1000.0, die.yl / 1000.0,
                    (die.xh - die.xl) / 1000.0,
                    (die.yh - die.yl) / 1000.0);
    cairo_fill_preserve(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1.0);
    cairo_stroke(cr);
}

void plotMacro(cairo_t* cr, const Point& originDBU,
               double widthUm, double heightUm,
               const std::array<double,3>& fillRGB)
{
    const double xUm = originDBU.x / 1000.0;
    const double yUm = originDBU.y / 1000.0;

    cairo_set_source_rgb(cr, fillRGB[0], fillRGB[1], fillRGB[2]);
    cairo_rectangle(cr, xUm, yUm, widthUm, heightUm);
    cairo_fill_preserve(cr);

    cairo_set_source_rgb(cr, 0, 0.5, 0);
    cairo_set_line_width(cr, 0.05);
    cairo_stroke(cr);
}

struct BBox { double xUm, yUm, wUm, hUm; };

BBox instBoundingBox(const Point& posDBU,
                     const lef::Macro& mac,
                     Orient orient,
                     int dbuPerMicron)
{
    const double px = posDBU.x / static_cast<double>(dbuPerMicron);
    const double py = posDBU.y / static_cast<double>(dbuPerMicron);
    const double ox = mac.origin.x;
    const double oy = mac.origin.y;
    const double w  = mac.sizeX;
    const double h  = mac.sizeY;

    switch (orient) {
        case Orient::N:  return { px - ox,           py - oy,           w,  h };
        case Orient::S:  return { px - (w - ox),     py - (h - oy),     w,  h };
        case Orient::E:  return { px - oy,           py - (w - ox),     h,  w };
        case Orient::W:  return { px - (h - oy),     py - ox,           h,  w };
        case Orient::FN: return { px - (w - ox),     py - oy,           w,  h };
        case Orient::FS: return { px - ox,           py - (h - oy),     w,  h };
        case Orient::FE: return { px - oy,           py - ox,           h,  w };
        case Orient::FW: return { px - (h - oy),     py - (w - ox),     h,  w };
        default:         return { px, py, w, h };
    }
}

int main()
{
    std::vector<std::string> lefFiles = {
        "../testcase1/SNPSHOPT25/lef/snps25hopt.lef",
        "../testcase1/SNPSLOPT25/lef/snps25lopt.lef",
        "../testcase1/SNPSROPT25/lef/snps25ropt.lef",
        "../testcase1/SNPSSLOPT25/lef/snps25slopt.lef"
    };

    std::vector<lef::Library> lefLibs;
    lefLibs.reserve(lefFiles.size());
    for (const auto& path : lefFiles) {
        std::ifstream fin(path);
        if (!fin) { std::cerr << "Cannot open LEF file '" << path << "'\n"; return 1; }
        try {
            lef::LefParser parser(fin);
            lefLibs.push_back(parser.parse());
            std::cout << "Parsed " << lefLibs.back().macros.size() << " macros from " << path << '\n';
        } catch (const std::exception& e) {
            std::cerr << "LEF parse error in " << path << " : " << e.what() << '\n';
            return 1;
        }
    }

    Design design;
    {
        std::ifstream fin("../testcase1/testcase1.def");
        if (!fin) { std::cerr << "Failed to open DEF file '../testcase1/testcase1.def'\n"; return 1; }
        try { DefParser parser(fin); design = parser.parse(); }
        catch (const std::exception& e) { std::cerr << "DEF parse error: " << e.what() << '\n'; return 1; }
    }

    const double PADDING = 50, SCALE = 12.0;
    const int WIDTH  = ((design.dieArea.xh - design.dieArea.xl) / 1000 + 2 * PADDING) * SCALE;
    const int HEIGHT = ((design.dieArea.yh - design.dieArea.yl) / 1000 + 2 * PADDING) * SCALE;

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
    cairo_t* cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9); cairo_paint(cr);
    cairo_scale(cr, SCALE, SCALE);
    cairo_translate(cr, PADDING, PADDING);
    plotOutline(cr, design.dieArea);

    std::size_t skipped = 0, plotCount = 0;
    std::map<std::string, int> macroCounts;
    std::array<std::size_t,5> catCount{0,0,0,0,0};

    std::map<std::string, int> fsdnSubtypes, fsdnqSubtypes;

    for (const auto& cmp : design.comps) {
        const lef::Macro* mac = nullptr;
        for (const auto& lib : lefLibs) {
            if (auto it = lib.macros.find(cmp.refName); it != lib.macros.end()) {
                mac = &it->second;
                break;
            }
        }
        if (!mac) { ++skipped; continue; }

        BBox bb = instBoundingBox(cmp.pos, *mac, cmp.orient, design.dbuPerMicron);
        MacroCat cat = categorizeMacro(cmp.refName);
        ++catCount[static_cast<int>(cat)];
        macroCounts[cmp.refName]++;

        if (cat == MacroCat::FSDN && cmp.refName.find("FSDN_V2_1") != std::string::npos) fsdnSubtypes["FSDN_V2_1"]++;
        else if (cat == MacroCat::FSDN && cmp.refName.find("FSDN_V2_2") != std::string::npos) fsdnSubtypes["FSDN_V2_2"]++;
        else if (cat == MacroCat::FSDN && cmp.refName.find("FSDN_V2_4") != std::string::npos) fsdnSubtypes["FSDN_V2_4"]++;

        if (cat == MacroCat::FSDNQ && cmp.refName.find("FSDNQ_V3_1") != std::string::npos) fsdnqSubtypes["FSDNQ_V3_1"]++;
        else if (cat == MacroCat::FSDNQ && cmp.refName.find("FSDNQ_V3_2") != std::string::npos) fsdnqSubtypes["FSDNQ_V3_2"]++;
        else if (cat == MacroCat::FSDNQ && cmp.refName.find("FSDNQ_V3_4") != std::string::npos) fsdnqSubtypes["FSDNQ_V3_4"]++;

        plotMacro(cr,
                  { static_cast<int>(bb.xUm * 1000), static_cast<int>(bb.yUm * 1000) },
                  bb.wUm, bb.hUm, colourOf(cat));
        ++plotCount;
    }

    std::cout << "\n=== Final Summary ===\n";
    std::cout << "Total components in DEF file           : " << design.comps.size() << '\n'
              << "Total unique macros referenced          : " << macroCounts.size() << '\n'
              << "Total macros available across LEFs      : " << [&]{ std::size_t t=0; for(auto& l:lefLibs) t+=l.macros.size(); return t; }() << '\n'
              << "Total macros plotted                    : " << plotCount << '\n'
              << "Components skipped (macro not found)    : " << skipped << "\n\n";

    std::cout << "Macro type counts (filtered by consecutive keywords):\n"
              << "  FSDNQ (blue)   : " << catCount[0] << '\n'
              << "    - FSDNQ_V3_1 : " << fsdnqSubtypes["FSDNQ_V3_1"] << '\n'
              << "    - FSDNQ_V3_2 : " << fsdnqSubtypes["FSDNQ_V3_2"] << '\n'
              << "    - FSDNQ_V3_4 : " << fsdnqSubtypes["FSDNQ_V3_4"] << '\n'
              << "  FSDN2 (green)  : " << catCount[1] << '\n'
              << "  FSDN4 (purple) : " << catCount[2] << '\n'
              << "  FSDN  (yellow) : " << catCount[3] << '\n'
              << "    - FSDN_V2_1  : " << fsdnSubtypes["FSDN_V2_1"] << '\n'
              << "    - FSDN_V2_2  : " << fsdnSubtypes["FSDN_V2_2"] << '\n'
              << "    - FSDN_V2_4  : " << fsdnSubtypes["FSDN_V2_4"] << '\n'
              << "  OTHER (grey)   : " << catCount[4] << '\n';

    cairo_surface_write_to_png(surface, "chip_layout.png");
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return 0;
}
