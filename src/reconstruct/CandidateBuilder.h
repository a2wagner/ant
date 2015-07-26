#pragma once

#include "expconfig/Detector_t.h"
#include "tree/TCluster.h"
#include "tree/TEvent.h"

#include <map>
#include <list>
#include <memory>

namespace ant {

namespace expconfig {
namespace detector {
class CB;
class PID;
class TAPS;
class TAPSVeto;
}
}


namespace reconstruct {

class CandidateBuilder {
protected:
    std::shared_ptr<expconfig::detector::CB>  cb;
    std::shared_ptr<expconfig::detector::PID> pid;
    std::shared_ptr<expconfig::detector::TAPS> taps;
    std::shared_ptr<expconfig::detector::TAPSVeto> tapsveto;

    void Build_PID_CB(
            std::map<Detector_t::Type_t, std::list< TCluster > >& sorted_clusters,
            TEvent::candidates_t& candidates
            );

    void Build_TAPS_Veto(
            std::map<Detector_t::Type_t, std::list< TCluster > >& sorted_clusters,
            TEvent::candidates_t& candidates
            );

    void Catchall(
            std::map<Detector_t::Type_t, std::list< TCluster > >& sorted_clusters,
            TEvent::candidates_t& candidates
            );

public:

    using sorted_detectors_t = std::map<Detector_t::Type_t, std::shared_ptr<Detector_t> >;

    CandidateBuilder(const sorted_detectors_t& sorted_detectors);
    virtual ~CandidateBuilder() = default;

    // this method shall fill the TEvent reference
    // with tracks built from the given sorted clusters
    /// \todo make this method abstract and create proper derived track builders
    virtual void Build(
            std::map<Detector_t::Type_t, std::list< TCluster > >&& sorted_clusters,
            TEvent::candidates_t& tracks
            );
};

}} // namespace ant::reconstruct