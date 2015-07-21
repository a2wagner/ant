#ifndef DETECTORS_PID_H
#define DETECTORS_PID_H

#include "Detector_t.h"
#include "unpacker/UnpackerAcqu.h"
#include "tree/THeaderInfo.h"
#include "base/std_ext.h"
#include <cassert>

namespace ant {
namespace expconfig {
namespace detector {
struct PID :
        Detector_t,
        UnpackerAcquConfig // PID knows how to be filled from Acqu data
{


    virtual TVector3 GetPosition(unsigned channel) const override {
        return elements[channel].Position;
    }

    // for UnpackerAcquConfig
    virtual void BuildMappings(
            std::vector<hit_mapping_t>&,
            std::vector<scaler_mapping_t>&) const override;

protected:

    struct Element_t : Detector_t::Element_t {
        Element_t(
                unsigned channel,
                double phi_degrees,
                unsigned adc,
                unsigned tdc
                ) :
            Detector_t::Element_t(
                channel,
                TVector3(1,0,0) // start with unit vector in x/y plane
                ),
            ADC(adc),
            TDC(tdc)
        {
            Position.SetPhi(std_ext::degree_to_radian(phi_degrees));
        }
        unsigned ADC;
        unsigned TDC;
    };

    PID(const std::vector<Element_t>& elements_init) :
        Detector_t(Detector_t::Type_t::PID),
        elements(elements_init)
    {
        assert(elements.size() == 24);
    }

    const std::vector<Element_t> elements;
};

struct PID_2014 : PID {
    PID_2014() : PID(elements_init) {}
    virtual bool Matches(const THeaderInfo& headerInfo) const override {
        return std_ext::time_after(headerInfo.Timestamp, "2014-01-26");
    }
    static const std::vector<Element_t> elements_init;
};

struct PID_2009_07 : PID {
    PID_2009_07() : PID(elements_init) {}
    virtual bool Matches(const THeaderInfo& headerInfo) const override {
        return std_ext::time_between(headerInfo.Timestamp, "2009-07-13", "2014-01-25");
    }
    static const std::vector<Element_t> elements_init;
};

struct PID_2009_06 : PID {
    PID_2009_06() : PID(elements_init) {}
    virtual bool Matches(const THeaderInfo& headerInfo) const override {
        return std_ext::time_between(headerInfo.Timestamp, "2009-06-30", "2009-07-12");
    }
    static const std::vector<Element_t> elements_init;
};

struct PID_2009_05 : PID {
    PID_2009_05() : PID(elements_init) {}
    virtual bool Matches(const THeaderInfo& headerInfo) const override {
        return std_ext::time_between(headerInfo.Timestamp, "2009-05-10", "2009-06-29");
    }
    static const std::vector<Element_t> elements_init;
};

struct PID_2004 : PID {
    PID_2004() : PID(elements_init) {}
    virtual bool Matches(const THeaderInfo& headerInfo) const override {
        return std_ext::time_between(headerInfo.Timestamp, "2004-04-30", "2009-05-09");
    }
    static const std::vector<Element_t> elements_init;
};


}}} // namespace ant::expconfig::detector

#endif // DETECTORS_PID_H