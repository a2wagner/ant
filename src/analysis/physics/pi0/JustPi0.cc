#include "JustPi0.h"

#include "utils/particle_tools.h"

#include "expconfig/ExpConfig.h"

#include "TH1D.h"

#include <memory>
#include <cassert>

using namespace ant;
using namespace ant::analysis;
using namespace ant::analysis::physics;
using namespace ant::analysis::data;
using namespace std;



JustPi0::JustPi0(const string& name, PhysOptPtr opts) :
    Physics(name, opts)
{
    for(unsigned mult=1;mult<=opts->Get<unsigned>("nPi0",1);mult++) {
        multiPi0.emplace_back(std_ext::make_unique<MultiPi0>(HistFac, mult));
    }
}

void JustPi0::ProcessEvent(const Event& event)
{
    const auto& data = event.Reconstructed;
    for(auto& m : multiPi0)
        m->ProcessData(data);
}

void JustPi0::ShowResult()
{
    for(auto& m : multiPi0)
        m->ShowResult();
}




JustPi0::MultiPi0::MultiPi0(SmartHistFactory& histFac, unsigned nPi0) :
    multiplicity(nPi0),
    fitter(std_ext::formatter() << multiplicity << "Pi0", 2*multiplicity),
    h_missingmass(promptrandom),
    h_fitprobability(promptrandom),
    IM_2g_byMM(promptrandom),
    IM_2g_byFit(promptrandom),
    IM_2g_fitted(promptrandom)
{
    std::string multiplicity_str = std_ext::formatter() << multiplicity << "Pi0";
    SmartHistFactory HistFac(multiplicity_str, histFac, multiplicity_str);

    promptrandom.AddPromptRange({-2.5,2.5});
    promptrandom.AddRandomRange({-50,-5});
    promptrandom.AddRandomRange({  5,50});

    steps = HistFac.makeTH1D("Steps","","#",BinSettings(10),"steps");

    Proton_Coplanarity = HistFac.makeTH1D("p Coplanarity","#delta#phi / degree","",BinSettings(400,-180,180),"Proton_Coplanarity");


    h_missingmass.MakeHistograms(HistFac, "h_missingmass","Missing Mass",BinSettings(400,400, 1400),"MM / MeV","#");
    h_fitprobability.MakeHistograms(HistFac, "fit_probability","KinFitter probability",BinSettings(150,0,1),"p","#");

    BinSettings bins_IM(500,0,700);

    IM_2g_byMM.MakeHistograms(HistFac, "IM_2g_byMM","IM 2#gamma by MM",bins_IM,"IM / MeV","#");
    IM_2g_byFit.MakeHistograms(HistFac, "IM_2g_byFit","IM 2#gamma by Fit",bins_IM,"IM / MeV","#");
    IM_2g_fitted.MakeHistograms(HistFac, "IM_2g_fitted","IM 2#gamma fitted",bins_IM,"IM / MeV","#");


    const auto setup = ant::ExpConfig::Setup::GetLastFound();
    fitter.LoadSigmaData(setup->GetPhysicsFilesDirectory()+"/FitterSigmas.root");
}

void JustPi0::MultiPi0::ProcessData(const Event::Data& data)
{
    const auto nPhotons_expected = multiplicity*2;

    steps->Fill("Seen",1);

    // cut on energy sum and number of candidates

    if(data.Trigger.CBEnergySum <= 550)
        return;
    steps->Fill("CBESum>550MeV",1);

    const auto& cands = data.Candidates;
    const auto nCandidates = cands.size();
    const auto nCandidates_expected = nPhotons_expected+1;
    if(nCandidates != nCandidates_expected)
        return;
    std::string nCandidates_cutstr = std_ext::formatter() << "nCandidates==" << nCandidates_expected;
    steps->Fill(nCandidates_cutstr.c_str(),1);


    // use any candidate as proton, and do the analysis (ignore ParticleID stuff)

    for(auto i_proton=cands.begin();i_proton!=cands.end();i_proton++) {

        const auto proton = std::make_shared<Particle>(ParticleTypeDatabase::Proton, *i_proton);
        std::vector<ParticlePtr> photons;
        for(auto i_photon=cands.begin();i_photon!=cands.end();i_photon++) {
            if(i_photon == i_proton)
                continue;
            photons.emplace_back(make_shared<Particle>(ParticleTypeDatabase::Photon, *i_photon));
        }
        assert(photons.size() == nPhotons_expected);


        TLorentzVector photon_sum(0,0,0,0);
        for(const auto& p : photons) {
            photon_sum += *p;
        }

        // proton coplanarity
        const double d_phi = std_ext::radian_to_degree(TVector2::Phi_mpi_pi(proton->Phi()-photon_sum.Phi() - M_PI ));
        Proton_Coplanarity->Fill(d_phi);

        const interval<double> Proton_Copl_cut(-19, 19);
        if(!Proton_Copl_cut.Contains(d_phi))
            continue;
        const string copl_str = std_ext::formatter() << "Copl p in " << Proton_Copl_cut;
        steps->Fill(copl_str.c_str(),1);

        for(const TaggerHit& taggerhit : data.TaggerHits) {
            steps->Fill("Seen taggerhits",1.0);

            promptrandom.SetTaggerHit(taggerhit.Time);
            if(promptrandom.State() == PromptRandom::Case::Outside)
                continue;

            // simple missing mass cut
            const TLorentzVector beam_target = taggerhit.GetPhotonBeam() + TLorentzVector(0, 0, 0, ParticleTypeDatabase::Proton.Mass());
            const TLorentzVector v_mm = beam_target - photon_sum;
            const double mm = v_mm.M();

            h_missingmass.Fill(mm);
            const interval<double> MM_cut(850, 1000);
            if(MM_cut.Contains(mm)) {
                const string MM_str = std_ext::formatter() << "MM in " << MM_cut;
                steps->Fill(MM_str.c_str(),1.0);
                utils::ParticleTools::FillIMCombinations([this] (double x) {IM_2g_byMM.Fill(x);},  2, photons);
            }

            // more sophisticated fitter
            fitter.SetEgammaBeam(taggerhit.PhotonEnergy);
            fitter.SetProton(proton);
            fitter.SetPhotons(photons);
            auto fit_result = fitter.DoFit();

            if(fit_result.Status == APLCON::Result_Status_t::Success) {
                steps->Fill("Fit OK",1.0);
                h_fitprobability.Fill(fit_result.Probability);
                const interval<double> fitprob_cut(0.8, 1);
                if(fitprob_cut.Contains(fit_result.Probability)) {
                    const string fitprob_str = std_ext::formatter() << "Fit p in " << fitprob_cut;
                    steps->Fill(fitprob_str.c_str(), 1.0);
                    utils::ParticleTools::FillIMCombinations([this] (double x) {IM_2g_byFit.Fill(x);},  2, photons);
                    utils::ParticleTools::FillIMCombinations([this] (double x) {IM_2g_fitted.Fill(x);},  2, fitter.GetFittedPhotons());
                }

            }

        }


    }
}

void JustPi0::MultiPi0::ShowResult()
{
    canvas(std_ext::formatter() << "JustPi0: " << multiplicity << "Pi0")
            << steps
            << Proton_Coplanarity
            << h_missingmass.subtracted
            << IM_2g_byMM.subtracted
            << h_fitprobability.subtracted
            << IM_2g_byFit.subtracted
            << IM_2g_fitted.subtracted
            << endc;
}

AUTO_REGISTER_PHYSICS(JustPi0)