#include "UnpackerA2Geant.h"

#include "expconfig/ExpConfig.h"

#include "tree/TEvent.h"
#include "tree/TEventData.h"

#include "base/WrapTFile.h"
#include "base/Logger.h"

#include "TTree.h"

#include <memory>

using namespace std;
using namespace ant;

UnpackerA2Geant::UnpackerA2Geant() {}

UnpackerA2Geant::~UnpackerA2Geant() {
    delete id;
}

bool UnpackerA2Geant::OpenFile(const string& filename)
{
    // open a root file, ignore error silently
    inputfile = std_ext::make_unique<WrapTFileInput>();

    try {
        inputfile->OpenFile(filename);
    } catch (const std::runtime_error&) {
        return false;
    }

    // setup the "expected" A2 geant tree
    if(!inputfile->GetObject("h12", geant))
        return false;

    geant->SetBranchAddress("nhits",&fnhits);
    geant->SetBranchAddress("npart",&fnpart);
    geant->SetBranchAddress("ntaps",&fntaps);
    geant->SetBranchAddress("nvtaps",&fnvtaps);
    geant->SetBranchAddress("vhits",&fvhits);
    geant->SetBranchAddress("plab",plab);
    geant->SetBranchAddress("tctaps",tctaps);
    geant->SetBranchAddress("vertex",fvertex);
    geant->SetBranchAddress("beam",fbeam);
    geant->SetBranchAddress("dircos",dircos);
    geant->SetBranchAddress("ecryst",ecryst);
    geant->SetBranchAddress("tcryst",tcryst);
    geant->SetBranchAddress("ectapfs",ectapfs);
    geant->SetBranchAddress("ectapsl",ectapsl);
    geant->SetBranchAddress("elab",elab);
    geant->SetBranchAddress("eleak",&feleak);
    geant->SetBranchAddress("enai",&fenai);
    geant->SetBranchAddress("etot",&fetot);
    geant->SetBranchAddress("eveto",eveto);
    geant->SetBranchAddress("tveto",tveto);
    geant->SetBranchAddress("evtaps",evtaps);
    geant->SetBranchAddress("icryst",icryst);
    geant->SetBranchAddress("ictaps",ictaps);
    geant->SetBranchAddress("ivtaps",ivtaps);
    geant->SetBranchAddress("idpart",idpart);
    geant->SetBranchAddress("iveto",iveto);
    geant->SetBranchAddress("nmwpc",&fnmwpc);
    geant->SetBranchAddress("imwpc",imwpc);
    geant->SetBranchAddress("mposx",mposx);
    geant->SetBranchAddress("mposy",mposy);
    geant->SetBranchAddress("mposz",mposz);
    geant->SetBranchAddress("emwpc",emwpc);

    TTree* tid_tree = nullptr;

    if(inputfile->GetObject("h12_tid", tid_tree)) {
        if(tid_tree->GetEntries() != geant->GetEntries()) {
            throw Exception("Geant Tree and TID Tree size mismatch");
        }

        geant->AddFriend(tid_tree);
        geant->SetBranchAddress("tid", &id);

        tid_from_file = true;

    } else {

        tid_from_file = false;

        /// \todo think of some better timestamp?
        id = new TID(static_cast<std::uint32_t>(std::time(nullptr)),
                     0, // start with 0 as lower ID
                     std::list<TID::Flags_t>{TID::Flags_t::MC, TID::Flags_t::AdHoc} // mark as MC
                     );
    }

    if(geant->GetEntries() >= numeric_limits<std::uint32_t>::max()) {
        throw Exception("Tree file contains too many entries for building correct unique ID");
    }

    geant->GetEntry(0);


    // this unpacker has no chance to make a proper THeaderInfo
    // so we ask the ExpConfig if it has an idea...
    const auto& manualName = ExpConfig::Setup::ManualName;
    if(manualName.empty()) {
        throw ExpConfig::ExceptionNoConfig("This unpacker requires a manually set setup name");
    }
    // actually the given id does not matter since ManualName is set...
    auto config = ExpConfig::Unpacker<UnpackerA2GeantConfig>::Get(*id);

    // find some taggerdetectors
    // needed to create proper tagger hits from incoming photons
    for(const shared_ptr<Detector_t>& detector : config->GetDetectors()) {
        std_ext::AddToSharedPtrList<TaggerDetector_t, Detector_t>(
                    detector, taggerdetectors
                    );
        /// \todo check for multiply defined detectors...
        if(detector->Type == Detector_t::Type_t::CB)
            cb_detector = detector;
        if(detector->Type == Detector_t::Type_t::PID)
            pid_detector = detector;
        if(detector->Type == Detector_t::Type_t::TAPS)
            taps_detector = detector;
        if(detector->Type == Detector_t::Type_t::TAPSVeto)
            tapsveto_detector = detector;
    }

    if(taggerdetectors.empty())
        LOG(WARNING) << "No tagger detector found in config, there will be no taggerhits generated";




    LOG(INFO) << "Successfully opened '" << filename
              << "' with " << geant->GetEntries() << " entries, "
              << (tid_from_file ? "with" : " WITHOUT") << " TID match check";


    return true;
}

std::unique_ptr<TEvent> UnpackerA2Geant::NextEvent() noexcept
{
    if(current_entry>=geant->GetEntriesFast()-1)
        return nullptr;

    geant->GetEntry(++current_entry);

    // start with an empty reconstructed event
    auto event = TEvent::MakeReconstructed(*id);

    // however, vertex is some MCTrue information!
    event->MCTrue = std_ext::make_unique<TEventData>(); // do not set its ID, will be done by "real" MCTrue reader
    event->MCTrue->Target.Vertex = fvertex; // TVector3 has conversion constructor...

    const size_t n_total = fnhits+fnpart+fntaps+fnvtaps+fvhits;

    // approx. 3 detector read hits per detector, we just want to prevent re-allocation
    vector<TDetectorReadHit>& hits = event->Reconstructed->DetectorReadHits;
    hits.reserve(3*n_total);

    // all energies from A2geant are in GeV, but here we need MeV...
    const double GeVtoMeV = 1000.0;

    // fill CB Hits
    for(int i=0;i<fnhits;i++) {
        const unsigned ch = icryst[i]; // no -1 here!
        if(cb_detector->IsIgnored(ch))
            continue;
        const Detector_t::Type_t det = Detector_t::Type_t::CB;
        hits.emplace_back(
                    LogicalChannel_t{det, Channel_t::Type_t::Integral, ch},
                    vector<double>{GeVtoMeV*ecryst[i]}
                    );
        hits.emplace_back(
                    LogicalChannel_t{det, Channel_t::Type_t::Timing, ch},
                    vector<double>{tcryst[i]}
                    );
    }

    // fill PID Hits
    for(int i=0;i<fvhits;i++) {
        /// @todo Make PID channel mapping/rotation a Setup option?
        const unsigned ch = (23 - (iveto[i]-1) + 11) % 24;
        if(pid_detector->IsIgnored(ch))
            continue;
        const Detector_t::Type_t det = Detector_t::Type_t::PID;
        hits.emplace_back(
                    LogicalChannel_t{det, Channel_t::Type_t::Integral, ch},
                    vector<double>{GeVtoMeV*eveto[i]}
                    );
        hits.emplace_back(
                    LogicalChannel_t{det, Channel_t::Type_t::Timing, ch},
                    vector<double>{tveto[i]}
                    );
    }

    // fill TAPS Hits
    for(int i=0;i<fntaps;i++) {
        const unsigned ch = ictaps[i]-1;
        if(taps_detector->IsIgnored(ch))
            continue;
        const Detector_t::Type_t det = Detector_t::Type_t::TAPS;
        hits.emplace_back(
                    LogicalChannel_t{det, Channel_t::Type_t::Integral, ch},
                    vector<double>{GeVtoMeV*ectapsl[i]}
                    );
        /// \todo check if the short gate actually makes sense?
        hits.emplace_back(
                    LogicalChannel_t{det, Channel_t::Type_t::IntegralShort, ch},
                    vector<double>{GeVtoMeV*ectapfs[i]}
                    );
        hits.emplace_back(
                    LogicalChannel_t{det, Channel_t::Type_t::Timing, ch},
                    vector<double>{tctaps[i]}
                    );
    }

    // fill TAPSVeto Hits
    for(int i=0;i<fnvtaps;i++) {
        /// \todo check if -1 here is really correct, for now we throw silly exceptions
        if(ivtaps[i]==0)
            throw Exception("TAPS Veto index should start counting with 1");
        const unsigned ch = ivtaps[i]-1;
        if(tapsveto_detector->IsIgnored(ch))
            continue;
        const Detector_t::Type_t det = Detector_t::Type_t::TAPSVeto;
        hits.emplace_back(
                    LogicalChannel_t{det, Channel_t::Type_t::Integral, ch},
                    vector<double>{GeVtoMeV*evtaps[i]}
                    );
        /// \todo check if there's really no veto timing?
        hits.emplace_back(
                    LogicalChannel_t{det, Channel_t::Type_t::Timing, ch},
                    vector<double>{0}
                    );
    }

    // "reconstruct" a tagger electron from the photon
    const double photon_energy = GeVtoMeV*fbeam[4];

    for(const shared_ptr<TaggerDetector_t>& tagger : taggerdetectors) {
        // could the photon have been detected?
        unsigned ch;
        if(!tagger->TryGetChannelFromPhoton(photon_energy, ch))
            continue;
        if(tagger->IsIgnored(ch))
            continue;
        /// \todo create some random hits here?
        hits.emplace_back(
                    LogicalChannel_t{tagger->Type, Channel_t::Type_t::Timing, ch},
                    vector<double>{0}
                    );
    }


    if(!tid_from_file)
        ++(*id);

    return event;
}

double UnpackerA2Geant::PercentDone() const
{
    return double(current_entry) / double(geant->GetEntries());
}


