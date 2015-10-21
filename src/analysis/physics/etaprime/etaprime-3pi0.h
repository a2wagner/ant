#pragma once

#include "analysis/physics/Physics.h"
class TH1D;
class TTree;

namespace ant {
namespace analysis {
namespace physics {

class Etap3pi0 : public Physics {

protected:

    const std::vector<std::vector<std::pair<unsigned,unsigned>>> combinations =
    {
        { {0, 1}, {2, 3}, {4, 5} },
        { {0, 1}, {2, 4}, {3, 5} },
        { {0, 1}, {2, 5}, {3, 4} },

        { {0, 2}, {1, 3}, {4, 5} },
        { {0, 2}, {1, 4}, {3, 5} },
        { {0, 2}, {1, 5}, {3, 4} },

        { {0, 3}, {1, 2}, {4, 5} },
        { {0, 3}, {1, 4}, {2, 5} },
        { {0, 3}, {1, 5}, {2, 4} },

        { {0, 4}, {1, 2}, {3, 5} },
        { {0, 4}, {1, 3}, {2, 5} },
        { {0, 4}, {1, 5}, {2, 3} },

        { {0, 5}, {1, 2}, {3, 4} },
        { {0, 5}, {1, 3}, {2, 4} },
        { {0, 5}, {1, 4}, {2, 3} }
    };

    struct ParticleVars {
        double E;
        double Theta;
        double Phi;
        double IM;
        ParticleVars(const TLorentzVector& lv, const ParticleTypeDatabase::Type& type) noexcept;
        ParticleVars(const data::Particle& p) noexcept;
        ParticleVars(double e=0.0, double theta=0.0, double phi=0.0, double im=0.0) noexcept:
            E(e), Theta(theta), Phi(phi), IM(im) {}
        ParticleVars(const ParticleVars&) = default;
        ParticleVars(ParticleVars&&) = default;
        ParticleVars& operator=(const ParticleVars&) =default;
        ParticleVars& operator=(ParticleVars&&) =default;
        void SetBranches(TTree* tree, const std::string& name);
    };


    using MesonCandidate = std::pair<data::ParticlePtr,double>;    // <particle,chi2>
    struct result_t {
        double Chi2 = std::numeric_limits<double>::infinity();
        double Chi2_etaprime = std::numeric_limits<double>::infinity();
        bool success = false;

        std::vector<data::ParticlePtr> g_final;
        std::vector<MesonCandidate> mesons;
        TLorentzVector etaprime;

        result_t() : g_final(6), mesons(3), etaprime(0,0,0,0){}

        void FillIm(const ParticleTypeDatabase::Type& type, TH1D* hist);
        void FillImEtaPrime(TH1D* hist);

        // void AddBranches();
        // void FillHists();
    };

    struct DalitzVars
    {
        double TMean;

        double s1;
        double s2;
        double s3;

        double x;
        double y;

        double z;

        DalitzVars(result_t r);
    };

    std::string dataset;

    TH1D* hNgamma;
    TH1D* hNgammaMC;

    TH1D* h2g;
    TH1D* h6g;

    TH1D* ch_3pi0_IM_etap;
    TH1D* ch_3pi0_IM_pi0;

    TH1D* ch_eta2pi0_IM_etap;
    TH1D* ch_eta2pi0_IM_pions;
    TH1D* ch_eta2pi0_IM_etas;

    TH1D* dalitz_z;

    TH2D* dalitz_xy;

    TTree* tree;

    ParticleVars pi01;
    ParticleVars pi02;
    ParticleVars pi03;
    ParticleVars MMproton;
    ParticleVars etaprime;
    double imsqrP12;
    double imsqrP13;
    double imsqrP23;



    void FillCrossChecks(const data::ParticleList& photons, const data::ParticleList& mcphotons);

    Etap3pi0::result_t Make3pi0(const data::ParticleList& photons);
    Etap3pi0::result_t MakeEta2pi0(const data::ParticleList& photons);


    Etap3pi0::result_t MakeMC3pi0(const data::Event::Data &mcEvt);
public:
    Etap3pi0(const std::string& name, PhysOptPtr opts);
    virtual void ProcessEvent(const data::Event& event) override;
    virtual void ShowResult() override;
};


}}}