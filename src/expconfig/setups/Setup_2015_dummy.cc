#include "Setup.h"

#include <limits>

using namespace std;

namespace ant {
namespace expconfig {
namespace setup {

class Setup_2015_dummy :
        public Setup
{
public:
    virtual std::string GetName() const override {
        return "Setup_2015_dummy";
    }

    Setup_2015_dummy() {
        // just CB at the moment...
        AddDetector<detector::CB>();
    }

    virtual double GetBeamEnergy() const override {
        // don't be afraid to use NaN if you don't know a value
        return numeric_limits<double>::quiet_NaN();
    }

    bool Matches(const THeaderInfo& header) const override {
        if(!Setup::Matches(header))
            return false;
        // just to get the Test case working
        return std_ext::time_between(header.Timestamp, "2015-01-29", "2015-01-29");
    }

    void BuildMappings(std::vector<hit_mapping_t>& hit_mappings,
                       std::vector<scaler_mapping_t>& scaler_mappings) const
    {
        Setup::BuildMappings(hit_mappings, scaler_mappings);
        // you may tweak the mapping at this location here
        // for example, ignore elements
    }
};

// if you forget this, your setup is never found....
AUTO_REGISTER_SETUP(Setup_2015_dummy)

}}} // namespace ant::expconfig::setup