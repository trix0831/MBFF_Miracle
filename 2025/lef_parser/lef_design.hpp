#pragma once
/*---------------------------------------------------------------------------*
 *  LEF data model – pure structs, no logic
 *---------------------------------------------------------------------------*/
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace lef {

/* ── Geometry helpers ──────────────────────────────────────────────────── */
using Coord = double;                       // LEF is typically micron-based

struct Point { Coord x{}, y{}; };
struct Rect  { Coord xl{}, yl{}, xh{}, yh{}; };

/* ── Leaf structs ──────────────────────────────────────────────────────── */
struct Units            { int    dbuPerMicron{0}; };
struct ManufacturingGrid{ double grid{0.0}; };

struct PortRect { std::string layer; Rect  box;            };
struct Polygon  { std::string layer; std::vector<Coord> pts; };

struct Pin {
    std::string            name;
    std::string            direction;          // INPUT / OUTPUT / INOUT / UNKNOWN
    std::string            use;                // SIGNAL / POWER / etc.
    std::vector<PortRect>  ports;
    std::vector<Polygon>   polygons;
};

struct ObsRect { std::string layer; Rect box; };

struct Site {
    std::string name;
    std::string symmetry;
    Coord sizeX{0.0}, sizeY{0.0};
};

struct Macro {
    uint32_t                    id{0};
    std::string                 name, cls, symmetry, site;
    Point                       origin;
    Coord                       sizeX{0.0},  sizeY{0.0};
    std::vector<Pin>            pins;
    std::vector<ObsRect>        obstructions;
    std::vector<Polygon>        obsPolygons;
};

/* ── Top-level container (analogue of DEF “Design”) ────────────────────── */
struct Library {
    std::string                               name;      // optional
    Units                                     units;
    ManufacturingGrid                         mfgGrid;
    std::unordered_map<std::string, Site>     sites;
    std::unordered_map<std::string, Macro>    macros;
};

} // namespace lef
