#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

using Coord  = int32_t;
using HashId = uint32_t;

struct Point { Coord x, y; };
struct Rect  { Coord xl, yl, xh, yh; };

enum class Orient : uint8_t { N, S, E, W, FN, FS, FE, FW };
enum class Status : uint8_t { UNPLACED, PLACED, FIXED, COVER };
enum class Direction : uint8_t { INPUT, OUTPUT, INOUT };

struct Pin {
    std::string name;
    std::string netName;
    Direction direction;
    Status status;
    Point pos;
    Orient orient;
    std::string layer;
    Rect port;  // For port geometry
};

struct PinProperty {
    std::string pinName;
    std::string propertyName;
    std::string value;
};

struct Net {
    std::string name;
    std::vector<std::string> connections;  // List of connected components/pins
    std::vector<Rect> routes;  // Routing information
};

struct Component {
    uint32_t id;
    std::string refName;
    std::string instName;
    Point pos;
    Orient orient;
    Status status;
};

struct Row {
    std::string name;
    std::string siteName;
    Point origin;
    Orient orient;
    int numX;
    int numY;
    int stepX;
    int stepY;
};

struct Track {
    std::string direction;  // "X" or "Y"
    int start;
    int numTracks;
    int step;
    std::vector<std::string> layers;
};

struct Design {
    std::string name;
    int32_t dbuPerMicron = 1000;
    Rect dieArea;
    std::vector<Component> comps;
    std::vector<Row> rows;
    std::vector<Track> tracks;
    std::vector<Pin> pins;
    std::vector<PinProperty> pinProperties;
    std::vector<Net> nets;
};
