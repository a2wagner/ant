#include "TAPSVeto.h"
#include <cassert>

#include "tree/TID.h"

#include "detail/TAPSVeto_2013_BaF2_elements.h"
#include "detail/TAPSVeto_2013_PbWO4_elements.h"
#include "detail/TAPSVeto_2014_BaF2_elements.h"

using namespace std;
using namespace ant;
using namespace ant::expconfig::detector;

void TAPSVeto::BuildMappings(vector<UnpackerAcquConfig::hit_mapping_t> &hit_mappings,
                             vector<UnpackerAcquConfig::scaler_mapping_t>&) const {
    for(const BaF2_Element_t& element : BaF2_elements)  {

        if(element.Ignored)
            continue;

        // TAC provides timing information

        hit_mappings.emplace_back(Type,
                                  Channel_t::Type_t::Timing,
                                  element.Channel,
                                  element.TAC);

        // the flag UseSensitiveChannels swaps the mapping
        // of LGS/LG to type Integral/IntegralAlternate
        // only Integral is used in the clustering

        hit_mappings.emplace_back(Type,
                                  Channel_t::Type_t::Integral,
                                  element.Channel,
                                  element.LGS
                                  );

    }

    // the PbWO4 are a bit simpler

    for(const PbWO4_Element_t& element : PbWO4_elements)  {

        if(element.Ignored)
            continue;

        hit_mappings.emplace_back(Type,
                                  Channel_t::Type_t::Timing,
                                  element.Channel,
                                  element.TDC);

        /// \todo Provide switch to use sensitive/non-sensitive?

        hit_mappings.emplace_back(Type,
                                  Channel_t::Type_t::Integral,
                                  element.Channel,
                                  element.QDCH
                                  );

        hit_mappings.emplace_back(Type,
                                  Channel_t::Type_t::IntegralAlternate,
                                  element.Channel,
                                  element.QDCH
                                  );
    }
}

void TAPSVeto::InitElements()
{
    size_t nElements = 0;

    // apply the z-position depending on Cherenkov
    // we assume that the channel elements are consecutive
    const double zpos = CherenkovInstalled ? 174.2 : 145.7;
    /// \bug check z-position, Acqu config is inconsistent
    for(auto& baf2 : BaF2_elements) {
        baf2.Position.SetZ(zpos);
        nElements++;
    }
    for(auto& pbwo4 : PbWO4_elements) {
        pbwo4.Position.SetZ(zpos);
        nElements++;
    }

    assert(nElements == 384);

    elements.resize(nElements);
    for(auto& element : BaF2_elements) {
        elements[element.Channel] = std::addressof(element);
    }

    for(auto& element : PbWO4_elements) {
        elements[element.Channel] = std::addressof(element);
    }

}

bool TAPSVeto::IsPbWO4(const unsigned channel) const
{
    if(PbWO4_elements.empty())
        return false;

    const unsigned elementsPerSector = (BaF2_elements.size()+PbWO4_elements.size())/NSectors;

    const unsigned channelFirstSector = channel % elementsPerSector;

    const unsigned PbWO4_elementsPerSector = PbWO4_elements.size() / NSectors;

    return channelFirstSector < PbWO4_elementsPerSector;
}

bool TAPSVeto_2013::Matches(const TID& tid) const
{
    return std_ext::time_between(tid.Timestamp, "2013-11-01", "2013-31-12");
}

bool TAPSVeto_2014::Matches(const TID& tid) const
{
    return std_ext::time_after(tid.Timestamp, "2014-01-01");
}
