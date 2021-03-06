#pragma once

#include "analysis/physics/Physics.h"
#include "analysis/utils/A2GeoAcceptance.h"
#include "base/Tree.h"
#include "base/interval.h"
#include "analysis/utils/particle_tools.h"
#include "base/std_ext/math.h"

#include "analysis/utils/Fitter.h"
#include "base/interval.h"
#include "analysis/plot/PromptRandomHist.h"
#include "TTree.h"
#include <map>

#include "base/WrapTTree.h"

class TH1D;
class TH2D;
class TH3D;


namespace ant {

namespace analysis {
namespace physics {

class OmegaMCTruePlots: public Physics {
public:
    struct PerChannel_t {
        std::string title;
        TH2D* proton_E_theta = nullptr;

        PerChannel_t(const std::string& Title, HistogramFactory& hf);

        void Show();
        void Fill(const TEventData& d);
    };

    std::map<std::string,PerChannel_t> channels;

    OmegaMCTruePlots(const std::string& name, OptionsPtr opts);

    virtual void ProcessEvent(const TEvent& event, manager_t& manager) override;
    virtual void Finish() override;
    virtual void ShowResult() override;
};

class OmegaBase: public Physics {

public:
    enum class DataMode {
        MCTrue,
        Reconstructed
    };

protected:
    utils::A2SimpleGeometry geo;
    double calcEnergySum(const TParticleList& particles) const;
    TParticleList getGeoAccepted(const TParticleList& p) const;
    unsigned geoAccepted(const TCandidateList& cands) const;

    DataMode mode = DataMode::Reconstructed;

    virtual void Analyse(const TEventData& data, const TEvent& event, manager_t& manager) =0;



public:
    OmegaBase(const std::string &name, OptionsPtr opts);
    virtual ~OmegaBase() = default;

    virtual void ProcessEvent(const TEvent& event, manager_t& manager) override;
    virtual void Finish() override;
    virtual void ShowResult() override;


};

class OmegaEtaG: public OmegaBase {

protected:

    TH2D* ggg_gg;
    TH2D* ggg_gg_bg;    // if not from omega decay
    TH2D* ggg_gg_all;
    //TH1D* gg_bg;        // if from omega but not pi0 or eta decay

    TH1D* ggg;
    TH1D* ggg_omega;
    TH1D* ggg_bg;
    TH1D* ggg_omega_pi0oreta;

    TH2D* ggg_gg_omega_eta;
    TH2D* ggg_gg_omega_pi0;

    TH1D* steps;

    IntervalD omega_range = IntervalD(680,780);

    struct perDecayhists_t {
        TH1D* gg = nullptr;
        TH1D* ggg = nullptr;
        TH1D* mm = nullptr;
        TH1D* angle_p;
        TH1D* angle_p_ggg;
        TH1D* p_phi_diff;
        TH2D* calc_proton_energy_theta;
        TH2D* calc_proton_special;
        TH1D* nCand;
    };

    perDecayhists_t makePerDecayHists(const std::string &title="");

    std::map<std::string, perDecayhists_t> gg_decays;

    virtual void Analyse(const TEventData& data, const TEvent& event, manager_t&) override;

    BinSettings imbinning = BinSettings(1000);
    BinSettings mmbinning = BinSettings(1000, 400,1400);

public:
    OmegaEtaG(const std::string& name, OptionsPtr opts);
    virtual ~OmegaEtaG() = default;
    virtual void ShowResult() override;
};



class OmegaMCTree : public Physics {
protected:
    TTree* tree = nullptr;
    TLorentzVector proton_vector;
    TLorentzVector omega_vector;
    TLorentzVector eta_vector;
    TLorentzVector gamma1_vector;
    TLorentzVector gamma2_vector;
    TLorentzVector gamma3_vector;

public:

    OmegaMCTree(const std::string& name, OptionsPtr opts);
    virtual ~OmegaMCTree();

    virtual void ProcessEvent(const TEvent& event, manager_t& manager) override;
    virtual void ShowResult() override;
    TLorentzVector getGamma1() const;
    void setGamma1(const TLorentzVector& value);
};


class OmegaEtaG2 : public OmegaBase {
public:
    struct OmegaTree_t : WrapTTree {
        OmegaTree_t();

        ADD_BRANCH_T(std::vector<TLorentzVector>, photons)
        ADD_BRANCH_T(TLorentzVector,              p)
        ADD_BRANCH_T(double,                      p_Time)
        ADD_BRANCH_T(double,                      p_PSA_Angle)
        ADD_BRANCH_T(double,                      p_PSA_Radius)
        ADD_BRANCH_T(int,                         p_detector)

        ADD_BRANCH_T(TLorentzVector,              p_true)
        ADD_BRANCH_T(TLorentzVector,              p_fitted)

        ADD_BRANCH_T(TLorentzVector,              ggg)
        ADD_BRANCH_T(TLorentzVector,              mm)
        ADD_BRANCH_T(double,                      copl_angle)
        ADD_BRANCH_T(double,                      p_mm_angle)

        ADD_BRANCH_T(std::vector<double>,         ggIM)

        ADD_BRANCH_T(std::vector<double>,         BachelorE)

        ADD_BRANCH_T(double,                      ggIM_real)  // only if Signal/Ref
        ADD_BRANCH_T(std::vector<double>,         ggIM_comb)  // only if Signal/Ref

        ADD_BRANCH_T(double,   TaggW)
        ADD_BRANCH_T(double,   TaggW_tight)
        ADD_BRANCH_T(double,   TaggE)
        ADD_BRANCH_T(double,   TaggT)
        ADD_BRANCH_T(unsigned, TaggCh)

        ADD_BRANCH_T(double,   KinFitChi2)
        ADD_BRANCH_T(unsigned, KinFitIterations)

        ADD_BRANCH_T(double,   CBSumE)
        ADD_BRANCH_T(double,   CBAvgTime)
        ADD_BRANCH_T(unsigned, nPhotonsCB)
        ADD_BRANCH_T(unsigned, nPhotonsTAPS)

        ADD_BRANCH_T(bool,     p_matched)

        ADD_BRANCH_T(int,      Channel)

    };

protected:
    void Analyse(const TEventData &data, const TEvent& event, manager_t&) override;


    TH1D* missed_channels = nullptr;
    TH1D* found_channels  = nullptr;


    //======== Tree ===============================================================

    TTree*  tree = nullptr;
    OmegaTree_t t;

    static const std::vector<std::vector<std::size_t>> combs;


    //======== Settings ===========================================================

    const double cut_ESum;
    const double cut_Copl;
    const interval<double> photon_E_cb;
    const interval<double> photon_E_taps;
    const interval<double> proton_theta;

    TH1D* steps;

    ant::analysis::PromptRandom::Switch promptrandom;
    utils::KinFitter fitter;

    bool AcceptedPhoton(const TParticlePtr& photon);
    bool AcceptedProton(const TParticlePtr& proton);

    TParticleList FilterPhotons(const TParticleList& list);
    TParticleList FilterProtons(const TParticleList& list);

public:

    OmegaEtaG2(const std::string& name, OptionsPtr opts);
    virtual ~OmegaEtaG2();

    using decaytree_t = ant::Tree<const ParticleTypeDatabase::Type&>;

    struct ReactionChannel_t {
        std::string name="";
        std::shared_ptr<decaytree_t> tree=nullptr;
        ReactionChannel_t(const std::shared_ptr<decaytree_t>& t);
        ReactionChannel_t(const std::string& n);
        ReactionChannel_t() = default;
        ~ReactionChannel_t();
    };

    struct ReactionChannelList_t {
        static const int other_index;
        std::map<int, ReactionChannel_t> channels;
        int identify(const TParticleTree_t &tree) const;
    };

    static ReactionChannelList_t makeChannels();
    static const ReactionChannelList_t reaction_channels;

};

}
}
}

std::string to_string(const ant::analysis::physics::OmegaBase::DataMode& m);
