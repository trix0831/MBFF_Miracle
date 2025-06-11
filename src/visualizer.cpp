#include <cairo/cairo.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <array>
#include "MBFFOptimizer.h"
#include "CellLibrary.h"
#include <filesystem>  // C++17
namespace fs = std::filesystem;

using Coord  = int32_t;
struct Point { Coord x, y; };
struct Rect  { Coord xl, yl, xh, yh; };

enum class MacroCat { FSDNQ = 0, FSDN2, FSDN4, FSDN, OTHER };

void ensureDirExists(const std::string& caseName) {
    fs::path dirPath = fs::path("pic") / caseName;
    if (!fs::exists(dirPath)) {
        fs::create_directories(dirPath);  // creates intermediate dirs if needed
    }
}

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

void plotOutline(cairo_t* cr, int width, int height)
{
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0.5, 0.5, width - 1.0, height - 1.0);
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

    cairo_set_source_rgb(cr, 0.92, 0.71, 0.16);
    cairo_set_line_width(cr, 0.3);
    cairo_stroke(cr);
}

void plotInit(int width, int height,
          std::unordered_map<std::string, Instance *> name2inst_ff,
          std::unordered_map<std::string, Instance *> name2inst_gate,
          std::string caseName){
    const double downScale    = 1000.0;    // DBU → µm
    const double PADDING_UM   = (width+height)/40/downScale;      // in µm
    const int resolutionScale = 2;         // <--- Add resolution scaling factor

    const double widthUm   = width  / downScale;
    const double heightUm  = height / downScale;

    // Apply resolution scale to convert µm → pixels
    const int PIX_W = std::ceil((widthUm  + 2 * PADDING_UM) * resolutionScale);
    const int PIX_H = std::ceil((heightUm + 2 * PADDING_UM) * resolutionScale);

    auto* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, PIX_W, PIX_H);
    auto* cr      = cairo_create(surface);

    // Set white background
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_paint(cr);

    // Apply resolution scale and padding to transform everything into pixels
    cairo_translate(cr, PADDING_UM * resolutionScale, PADDING_UM * resolutionScale);
    cairo_scale(cr, resolutionScale, resolutionScale);  // <--- THIS SCALES EVERYTHING

    // Draw the die outline (now in µm again)
    plotOutline(cr, widthUm, heightUm);
    std::cout << "Plotting chip layout with dimensions: "
              << widthUm << "µm x " << heightUm << "µm\n";

    int ff_count = 0;
    for (auto& [name, inst] : name2inst_ff) {
        Point origin{ inst->x(), inst->y() };
        double w = inst->pCellLibrary()->width()  / downScale;
        double h = inst->pCellLibrary()->height() / downScale;
        plotMacro(cr, origin, w, h, {0.17, 0.88, 0.74});
        ff_count++;
    }

    int gate_count = 0;
    for (auto& [name, inst] : name2inst_gate) {
        Point origin{ inst->x(), inst->y() };
        double w = inst->pCellLibrary()->width()  / downScale;
        double h = inst->pCellLibrary()->height() / downScale;
        plotMacro(cr, origin, w, h, {0.99, 0.99, 0.18});
        gate_count++;
    }

    //merge the file name as caseName+_init.png
    std::string fileName = "pic/" + caseName + "/init.png";
    ensureDirExists(caseName );  // Ensure the directory exists
    cairo_surface_write_to_png(surface, fileName.c_str());
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

void plotMerge(int width, int height, std::unordered_map<std::string, Instance*> name2inst_ff,
          std::unordered_map<std::string, Instance*> name2inst_gate, std::vector<Instance*> mergedInstances,
          std::string caseName, int frame) {
    const double downScale    = 1000.0;    // DBU → µm
    const double PADDING_UM   = (width+height)/40/downScale;      // in µm
    const int resolutionScale = 2;         // <--- Add resolution scaling factor

    const double widthUm   = width  / downScale;
    const double heightUm  = height / downScale;

    // Apply resolution scale to convert µm → pixels
    const int PIX_W = std::ceil((widthUm  + 2 * PADDING_UM) * resolutionScale);
    const int PIX_H = std::ceil((heightUm + 2 * PADDING_UM) * resolutionScale);

    auto* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, PIX_W, PIX_H);
    auto* cr      = cairo_create(surface);

    // Set white background
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_paint(cr);

    // Apply resolution scale and padding to transform everything into pixels
    cairo_translate(cr, PADDING_UM * resolutionScale, PADDING_UM * resolutionScale);
    cairo_scale(cr, resolutionScale, resolutionScale);  // <--- THIS SCALES EVERYTHING

    // Draw the die outline (now in µm again)
    plotOutline(cr, widthUm, heightUm);
    std::cout << "Plotting chip layout with dimensions: "
              << widthUm << "µm x " << heightUm << "µm\n";

    //plot unmerged FF instances
    int ff_count_unmerged = 0;
    for (auto& [name, inst] : name2inst_ff) {
        if(!inst->merged){
            Point origin{ inst->x(), inst->y() };
            double w = inst->pCellLibrary()->width()  / downScale;
            double h = inst->pCellLibrary()->height() / downScale;
            plotMacro(cr, origin, w, h, {0.17, 0.88, 0.74});
            ff_count_unmerged++;
        }
    }

    //plot merged FF instances
    int ff_count_merged = 0;
    for (auto& inst : mergedInstances) {
        Point origin{ inst->x(), inst->y() };
        double w = inst->pCellLibrary()->width()  / downScale;
        double h = inst->pCellLibrary()->height() / downScale;

        //assign different color for 1 2 4 bit FF
        if(inst->pCellLibrary()->numBits() == 1){

        }
        plotMacro(cr, origin, w, h, {0.91, 0.46, 0.33});
        ff_count_merged++;
    }


    int gate_count = 0;
    for (auto& [name, inst] : name2inst_gate) {
        Point origin{ inst->x(), inst->y() };
        double w = inst->pCellLibrary()->width()  / downScale;
        double h = inst->pCellLibrary()->height() / downScale;
        plotMacro(cr, origin, w, h, {0.99, 0.99, 0.18});
        gate_count++;
    }

    if (frame == 0) {   
        std::cout << "Total FF (unmerged) instances plotted (green): " << ff_count_unmerged << "\n";
        std::cout << "Total FF (merged) instances plotted(orange): " << ff_count_merged << "\n";
        std::cout << "Total gate instances plotted: " << gate_count << "\n";
    }

    std::string fileName = "pic/" + caseName + "/merged_" + std::to_string(frame) + ".png";
    cairo_surface_write_to_png(surface, fileName.c_str());
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}