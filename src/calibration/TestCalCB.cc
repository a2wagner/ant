#include "TestCalCB.h"
#include "analysis/plot/HistogramFactories.h"
#include "analysis/data/Event.h"
#include "analysis/utils/combinatorics.h"

ant::calibration::TestCalCB::TestCalCB():
    BaseCalibrationModule("TestCalCB")
{
    const BinSettings energybins(1000);

    ggIM = HistFac.makeTH1D("ggIM","2 #gamma IM [MeV]","#",energybins,"ggIM");
}

void ant::calibration::TestCalCB::ApplyTo(std::unique_ptr<TDetectorRead>&)
{

}

void ant::calibration::TestCalCB::ProcessEvent(const ant::Event &event)
{
    const auto& tracks = event.Reconstructed().Particles().Get(ParticleTypeDatabase::Photon);

    for( auto comb = makeCombination(tracks,2); !comb.Done(); ++comb ) {

        TLorentzVector gg = *comb.at(0) + *comb.at(1);
        ggIM->Fill(gg.M());
    }
}

void ant::calibration::TestCalCB::Finish()
{

}

void ant::calibration::TestCalCB::ShowResult()
{

}


void ant::calibration::TestCalCB::BuildRanges(std::list<ant::TDataRecord::ID_t> &ranges)
{
}

void ant::calibration::TestCalCB::Update(const ant::TDataRecord::ID_t &id)
{
}