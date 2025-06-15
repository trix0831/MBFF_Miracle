#ifndef SDC_DESIGN_HPP
#define SDC_DESIGN_HPP
#include <string>
#include <vector>
#include <utility>

namespace sdc {

//------------------------------------------------------------------------------
struct CreateClock {
    std::string            name;
    double                 period            = 0.0;
    std::pair<double,double> waveform{0.0,0.0};   // rising / falling
    std::string            portExpr;              // [get_ports {...}]
};

struct SetLoad {
    double      pinLoad = 0.0;
    std::string portExpr;
};

struct ClockLatency     { double value = 0.0; std::string clockExpr; };
struct ClockUncertainty { double value = 0.0; std::string clockExpr; };
struct ClockTransition  { double value = 0.0; std::string clockExpr; };

struct InputDelay  { double delay = 0.0; std::string clockExpr; std::string portExpr; };
struct OutputDelay { double delay = 0.0; std::string clockExpr; std::string portExpr; };

struct SdcLimits {
    std::string limitExpr;
    double      val  = 0.0;
    std::string targetExpr;        // e.g. [current_design]
};

// Central container -----------------------------------------------------------
struct Sdc {
    // header
    std::string version;
    std::string timeUnit, resistanceUnit, capacitanceUnit, voltageUnit, currentUnit;

    // constraints
    std::vector<CreateClock>      clocks;
    std::vector<SetLoad>          loads;
    std::vector<ClockLatency>     latencies;
    std::vector<ClockUncertainty> uncertainties;
    std::vector<ClockTransition>  transitions;
    std::vector<InputDelay>       inputDelays;
    std::vector<OutputDelay>      outputDelays;
    std::vector<SdcLimits>        limits;
};

} // namespace sdc
#endif /* SDC_DESIGN_HPP */
