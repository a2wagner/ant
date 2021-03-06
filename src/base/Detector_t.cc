#include "Detector_t.h"

#include <sstream>

using namespace ant;

const Detector_t::Any_t Detector_t::Any_t::None(0);
const Detector_t::Any_t Detector_t::Any_t::Tracker(Type_t::MWPC0 | Type_t::MWPC1);
const Detector_t::Any_t Detector_t::Any_t::CB_Apparatus(Detector_t::Any_t::Tracker | Type_t::PID | Type_t::CB );
const Detector_t::Any_t Detector_t::Any_t::TAPS_Apparatus(Type_t::TAPS | Type_t::TAPSVeto);
const Detector_t::Any_t Detector_t::Any_t::Calo(Type_t::CB | Type_t::TAPS);
const Detector_t::Any_t Detector_t::Any_t::Veto(Type_t::PID | Type_t::TAPSVeto);


std::ostream& ant::Detector_t::Any_t::Print(std::ostream& stream) const  {
    if(bitfield == 0)
        return stream << "None";

    using i_t = typename std::underlying_type<Type_t>::type;
    using bitfield_t = decltype(bitfield);

    bitfield_t temp = bitfield;

    i_t i_max = 0;
    while(temp >>= 1)
        i_max++;

    i_t i = 0;
    temp = 1;
    while(temp<=bitfield) {
        if(bitfield & temp) {
            stream << Detector_t::ToString(
                          static_cast<Detector_t::Type_t>(i)
                          );
            if(i<i_max)
                stream << "|";
        }
        ++i;
        temp <<= 1;
    }
    return stream;
}


const char* ant::Detector_t::ToString(const Type_t& type)
{
    switch(type) {
    case Detector_t::Type_t::CB :
        return "CB";
    case Detector_t::Type_t::Cherenkov:
        return "Cherenkov";
    case Detector_t::Type_t::MWPC0:
        return "MWPC0";
    case Detector_t::Type_t::MWPC1:
        return "MWPC1";
    case Detector_t::Type_t::PID:
        return "PID";
    case Detector_t::Type_t::Tagger:
        return "Tagger";
    case Detector_t::Type_t::TaggerMicro:
        return "TaggerMicro";
    case Detector_t::Type_t::EPT:
        return "EPT";
    case Detector_t::Type_t::Moeller:
        return "Moeller";
    case Detector_t::Type_t::PairSpec:
        return "PairSpec";
    case Detector_t::Type_t::TAPS:
        return "TAPS";
    case Detector_t::Type_t::TAPSVeto:
        return "TAPSVeto";
    case Detector_t::Type_t::Trigger:
        return "Trigger";
    case Detector_t::Type_t::Raw:
        return "Raw";
    }
    throw std::runtime_error("Not implemented");
}


ant::Detector_t::Any_t::operator std::string() const {
    std::stringstream s;
    Print(s);
    return s.str();
}


const char* ant::Channel_t::ToString(const Type_t& type)
{
    switch(type) {
    case Channel_t::Type_t::BitPattern:
        return "BitPattern";
    case Channel_t::Type_t::Integral:
        return "Integral";
    case Channel_t::Type_t::IntegralAlternate:
        return "IntegralAlternate";
    case Channel_t::Type_t::IntegralShort:
        return "IntegralShort";
    case Channel_t::Type_t::IntegralShortAlternate:
        return "IntegralShortAlternate";
    case Channel_t::Type_t::Timing:
        return "Timing";
    case Channel_t::Type_t::Raw:
        return "Raw";
    }
    throw std::runtime_error("Not implemented");
}
