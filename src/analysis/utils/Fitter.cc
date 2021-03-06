#include "Fitter.h"

#include "expconfig/ExpConfig.h"

#include "utils/particle_tools.h"

#include "base/std_ext/memory.h"
#include "base/std_ext/math.h"
#include "base/std_ext/system.h"
#include "base/std_ext/string.h"
#include "base/ParticleType.h"
#include "base/WrapTFile.h"
#include "base/Logger.h"
#include "base/Paths.h"

#include "APLCON.hpp" // external project

#include "TTree.h"
#include "TVector3.h"
#include "TH1D.h"
#include "TF1.h"

#include <cassert>
#include <functional>


using namespace std;
using namespace ant;
using namespace ant::std_ext;
using namespace ant::analysis;
using namespace ant::analysis::utils;


Fitter::Fitter(const string& fittername)
{
    APLCON::Fit_Settings_t settings = APLCON::Fit_Settings_t::Default;
    settings.MaxIterations = 30;
    aplcon = make_unique<APLCON>(fittername, settings);
}

void Fitter::LinkVariable(Fitter::FitParticle& particle)
{
    aplcon->LinkVariable(particle.Name,
                         particle.Addresses(),
                         particle.Addresses_Sigma(),
                         particle.Addresses_Pulls());
}

double Fitter::EnergyResolution(const TParticlePtr& p) const
{
    assert(p->Candidate!=nullptr);

    if(p->Type() != ParticleTypeDatabase::Photon)
        throw Exception("Currently only photons are supported");

    if(p->Candidate->Detector & Detector_t::Type_t::CB) {

        return 1.07134e-02 * p->Ek();

    } else if(p->Candidate->Detector & Detector_t::Type_t::TAPS) {

        return 3.5E-2 * p->Ek();

    } else {
        LOG(WARNING) << "Photon not in (CB,TAPS?)";
    }


    return 0.0;
}

double Fitter::ThetaResolution(const TParticlePtr& p) const
{
    assert(p->Candidate!=nullptr);

    if(p->Type() != ParticleTypeDatabase::Photon)
        throw Exception("Currently only photons are supported");

    if(p->Candidate->Detector & Detector_t::Type_t::CB) {

        return cb_sigma_theta.GetSigma(p->Candidate->FindCaloCluster()->CentralElement, p->Ek());

    } if(p->Candidate->Detector & Detector_t::Type_t::TAPS) {

        return taps_sigma_theta.GetSigma(p->Candidate->FindCaloCluster()->CentralElement, p->Ek());

    } else {
        LOG(WARNING) << "Photon not in (CB,TAPS?)";
    }

    return 0.0;
}

double Fitter::PhiResolution(const TParticlePtr& p) const
{
    assert(p->Candidate!=nullptr);

    if(p->Type() != ParticleTypeDatabase::Photon)
        throw Exception("Currently only photons are supported");

    if(p->Candidate->Detector & Detector_t::Type_t::CB) {

        return cb_sigma_phi.GetSigma(p->Candidate->FindCaloCluster()->CentralElement, p->Ek());

    } else if(p->Candidate->Detector & Detector_t::Type_t::TAPS) {

        return taps_sigma_phi.GetSigma(p->Candidate->FindCaloCluster()->CentralElement, p->Ek());

    } else {
        LOG(WARNING) << "Photon not in (CB,TAPS?)";
    }

    return 0.0;
}

void Fitter::SetPhotonEkThetaPhi(FitParticle& photon, const TParticlePtr& p) const
{
    photon.Particle = p;

    photon.Ek.Value  = p->Ek();
    photon.Ek.Sigma = EnergyResolution(p);

    photon.Theta.Value  = p->Theta();
    photon.Theta.Sigma = ThetaResolution(p);

    photon.Phi.Value  = p->Phi();
    photon.Phi.Sigma = PhiResolution(p);
}

double Fitter::fct_TaggerEGausSigma(double)
{
    return  3.0/sqrt(12.0);
}

void Fitter::FitParticle::Var_t::SetupBranches(TTree* tree, const string& prefix)
{
    tree->Branch(prefix.c_str(), addressof(Value));
    tree->Branch((prefix+"_pull").c_str(), addressof(Pull));
    tree->Branch((prefix+"_sigma").c_str(), addressof(Sigma));
}

void Fitter::FitParticle::SetupBranches(TTree* tree, const string& prefix)
{
    Ek.SetupBranches(tree, prefix+"_"+Name+"_Ek");
    Theta.SetupBranches(tree, prefix+"_"+Name+"_Theta");
    Phi.SetupBranches(tree, prefix+"_"+Name+"_Phi");
}

TLorentzVector Fitter::FitParticle::GetVector(const std::vector<double>& EkThetaPhi, const double m)
{
    const mev_t E = EkThetaPhi[0] + m;
    const mev_t p = sqrt( sqr(E) - sqr(m) );

    const double theta_ = EkThetaPhi[1];
    const double phi_ = EkThetaPhi[2];

    return TLorentzVector(
                p*TMath::Sin(theta_)*TMath::Cos(phi_),
                p*TMath::Sin(theta_)*TMath::Sin(phi_),
                p*TMath::Cos(theta_),
                E
                );
}

Fitter::angular_sigma::angular_sigma()
{
}

Fitter::angular_sigma::~angular_sigma()
{}

double Fitter::angular_sigma::GetSigma(const unsigned element, const double E) const
{
    const double vp0 = p0->GetBinContent(int(element));
    const double vp1 = p1->GetBinContent(int(element));
    const double vp2 = p2->GetBinContent(int(element));

    return f(E, vp0, vp1, vp2);

}

double Fitter::angular_sigma::f(const double x, const double p0, const double p1, const double p2) noexcept
{
    //return p0*exp(p1*x)+p2;
    return exp(p0 + p1*x) + p2;
}

double Fitter::angular_sigma::f_root(const double* x, const double* p) noexcept
{
    return f(x[0], p[0], p[1], p[2]);
}

TF1* Fitter::angular_sigma::GetTF1(const std::string& name)
{
    auto f = new TF1(name.c_str(), f_root, 0, 1600, 3);
    f->SetParName(0, "#alpha");
    f->SetParName(1, "#beta");
    f->SetParName(2, "Offset");
    return f;
}




void Fitter::angular_sigma::Load(WrapTFile& f, const string& prefix, const int bins)
{
    p0 = LoadHist(f, prefix+"_p0", bins);
    p1 = LoadHist(f, prefix+"_p1", bins);
    p2 = LoadHist(f, prefix+"_p2", bins);
}

Fitter::angular_sigma::Hist Fitter::angular_sigma::LoadHist(WrapTFile& f, const string& name, const int bins)
{
    auto h = f.GetSharedHist<TH1D>(name);

    if(!h)
        throw std::runtime_error("Could not load histogram " + name);

    if(h->GetNbinsX() != bins)
        throw std::runtime_error(formatter() << "TH1D " << name << " has wrong number of bins: " << h->GetNbinsX()  << ", expected: " << bins);

    return h;
}

void Fitter::LoadSigmaData(const string& filename)
{
    const auto& setup = ant::ExpConfig::Setup::GetLastFound();
    if(!setup)
        throw Exception("No Setup found!");

    unique_ptr<WrapTFileInput> f;
    if(!std_ext::system::testopen(filename)) {
        LOG(WARNING) << "Could not read sigmas from '" << filename << "', using default.";
        f = std_ext::make_unique<WrapTFileInput>(string(ANT_PATH_DATABASE)+"/default/physics_files/FitterSigmas.root");
    }
    else {
        f = std_ext::make_unique<WrapTFileInput>(filename);
    }


    const auto& CB = setup->GetDetector(Detector_t::Type_t::CB);
    if(!CB)
        throw Exception("No CB detector defined in setup");

    const auto nCB = CB->GetNChannels();

    const auto& TAPS = setup->GetDetector(Detector_t::Type_t::TAPS);
    if(!TAPS)
        throw Exception("No TAPS detector defined in setup");

    const auto nTAPS = TAPS->GetNChannels();

    cb_sigma_theta.Load(*f, "CB_sigma_Theta", int(nCB));
    cb_sigma_phi.Load(  *f, "CB_sigma_Phi",   int(nCB));

    taps_sigma_theta.Load(*f, "TAPS_sigma_Theta", int(nTAPS));
    taps_sigma_phi.Load(  *f, "TAPS_sigma_Phi",   int(nTAPS));
}

KinFitter::KinFitter(const std::string& name, unsigned numGammas) :
    Fitter(name)
{
    // nothing to do in this special case
    // used by "KinFit-free" TreeFitter
    if(numGammas==0)
        return;

    for(unsigned i=0; i<numGammas;++i) {
        Photons.emplace_back(make_shared<FitParticle>("Photon"+to_string(i)));
    }

    Beam = std_ext::make_unique<PhotonBeamVector>("Beam");
    aplcon->LinkVariable(Beam->Name,
                         Beam->Adresses() ,
                         Beam->Adresses_Sigma() );

    Proton = std_ext::make_unique<FitParticle>("Proton");
    LinkVariable(*Proton);

    vector<string> namesLInv      = { Beam->Name, Proton->Name };

    for ( auto& photon: Photons)
    {
        LinkVariable(*photon);
        namesLInv.push_back(photon->Name);
    }

    auto LorentzInvariance = [] (const vector<vector<double>>& values)
    {
        // beam    TLorentzVector(0.0, 0.0, PhotonEnergy(), PhotonEnergy());
        // target  TLorentzVector(0.0, 0.0, 0.0, ParticleTypeDatabase::Proton.Mass())
        const auto& Ebeam  = values[0][0];
        const auto& proton = values[1];

        //  Beam-LV:
        const TLorentzVector beam(0, 0, Ebeam, Ebeam);
        const TLorentzVector tg(0,0,0,ParticleTypeDatabase::Proton.Mass());

        TLorentzVector constraint = tg + beam;

        constraint -= FitParticle::GetVector(proton, ParticleTypeDatabase::Proton.Mass());

        const auto s = values.size();
        for ( unsigned photon = 0 ; photon < s-2 ; ++ photon)
            constraint -= FitParticle::GetVector(values[photon + 2], ParticleTypeDatabase::Photon.Mass());

        return vector<double>(
               { constraint.X(),
                 constraint.Y(),
                 constraint.Z(),
                 constraint.E()} );
    };

    aplcon->AddConstraint("LInv",namesLInv,LorentzInvariance);

}

void KinFitter::SetEgammaBeam(const double ebeam)
{
    Beam->E        = ebeam;
    Beam->E_before = ebeam;
    Beam->Sigma = fct_TaggerEGausSigma(ebeam);
}

void KinFitter::SetProton(const TParticlePtr& proton)
{
    if (proton->Candidate == nullptr)
        throw Exception(aplcon->GetName() + ": Proton-Candidate for Kinfitter not set!");


    Proton->Ek.Value    = proton->Ek();
    Proton->Theta.Value = proton->Theta();
    Proton->Phi.Value   = proton->Phi();

    Proton->Ek.Sigma    = 0.0; // unmeasured



    if(proton->Candidate->Detector & Detector_t::Type_t::CB) {
        Proton->Theta.Sigma = degree_to_radian(5.43);
        Proton->Phi.Sigma   = degree_to_radian(5.31);
    } else if(proton->Candidate->Detector & Detector_t::Type_t::TAPS) {
        Proton->Theta.Sigma = degree_to_radian(2.89);
        Proton->Phi.Sigma   = degree_to_radian(4.45);
    } else {
        LOG(WARNING) << "Proton is not in (CB,TAPS)?";
    }

    Proton->Particle = proton;
}

void KinFitter::SetPhotons(const std::vector<TParticlePtr>& photons)
{
    if(Photons.size() != photons.size())
        throw Exception("Given number of photons does not match configured fitter");

    for ( unsigned i = 0 ; i < Photons.size() ; ++ i)
        SetPhotonEkThetaPhi(*Photons[i], photons[i]);
}

TParticlePtr KinFitter::GetFittedProton() const
{
    auto p = make_shared<TParticle>(ParticleTypeDatabase::Proton,
                                    Proton->Ek.Value,
                                    Proton->Theta.Value,
                                    Proton->Phi.Value);
    p->Candidate = Proton->Particle->Candidate;
    return p;
}

TParticleList KinFitter::GetFittedPhotons() const
{
    TParticleList photons;
    for(unsigned i=0;i<Photons.size();i++) {
        const auto& photon = *Photons[i];
        auto p = make_shared<TParticle>(ParticleTypeDatabase::Photon,
                                        photon.Ek.Value, photon.Theta.Value, photon.Phi.Value);
        p->Candidate = photon.Particle->Candidate;
        photons.emplace_back(move(p));
    }
    return photons;
}

double KinFitter::GetFittedBeamE() const
{
    return Beam->E;
}



void KinFitter::SetupBranches(TTree* tree, string branch_prefix)
{
    if(branch_prefix.empty())
        branch_prefix = aplcon->GetName();

    Proton->SetupBranches(tree, branch_prefix);
    for(auto& p : Photons) {
        p->SetupBranches(tree, branch_prefix);
    }

    tree->Branch((branch_prefix+"_chi2dof").c_str(),     &result_chi2ndof);
    tree->Branch((branch_prefix+"_iterations").c_str(),  &result_iterations);
    tree->Branch((branch_prefix+"_status").c_str(),      &result_status);
    tree->Branch((branch_prefix+"_probability").c_str(), &result_probability);
}

APLCON::Result_t KinFitter::DoFit() {

    const auto res = aplcon->DoFit();

    result_chi2ndof    = res.ChiSquare / res.NDoF;
    result_iterations  = res.NIterations;
    result_status      = static_cast<int>(res.Status);
    result_probability = res.Probability;

    return res;
}

KinFitter::PhotonBeamVector::PhotonBeamVector(const string& name):
    Name(name)
{

}

TreeFitter::TreeFitter(const string& name,
                       ParticleTypeTree ptree,
                       unsigned kinFitGammas,
                       nodesetup_t::getter nodeSetup) :
    KinFitter(name, kinFitGammas),
    tree(MakeTree(ptree))
{

    vector<tree_t> tree_leaves;

    tree->GetUniquePermutations(tree_leaves, permutations);
    current_perm = permutations.end();

    // handle special case of no kinfit
    if(kinFitGammas==0) {
        for(unsigned i=0;i<tree_leaves.size();i++) {
            Photons.emplace_back(make_shared<FitParticle>("Photon"+to_string(i)));
            LinkVariable(*Photons.back());
        }
    }
    else if(tree_leaves.size()>kinFitGammas)
        throw Exception("Given ptree has too many leaves for given kinFitGammas");

    LOG(INFO) << "Initialized TreeFitter '" << name
              << "' for " << utils::ParticleTools::GetDecayString(ptree, false)
              << " with " << permutations.size() << " permutations"
              << (kinFitGammas>0 ? ", including KinFit" : "");

    // setup fitter variables, collect leave names for constraint
    vector<string> leave_names;
    for(unsigned i=0;i<tree_leaves.size();i++) {
        node_t& node = tree_leaves[i]->Get();
        node.Leave = Photons[i]; // link via shared_ptr
        leave_names.emplace_back(node.Leave->Name);
    }

    // prepare the calculation at each constraint
    // the Map_nodes call can be done once and the
    // calculation is stored in little functions
    using sum_daughters_t = function<void()>;
    using node_constraint_t = function<double()>;

    vector<sum_daughters_t> sum_daughters;
    vector<node_constraint_t> node_constraints;

    tree->Map_nodes([&sum_daughters, &node_constraints, nodeSetup] (const tree_t& tnode) {
        // do not include leaves
        if(tnode->IsLeaf())
            return;

        // do not include beamparticles
        if(tnode->Get().TypeTree->Get() == ParticleTypeDatabase::BeamTarget)
            return;

        // always sum up the tree nodes
        sum_daughters.emplace_back([tnode] () {
            node_t& node = tnode->Get();
            node.LVSum.SetPtEtaPhiE(0,0,0,0);
            assert(!tnode->Daughters().empty());
            for(const auto& d : tnode->Daughters())
                node.LVSum += d->Get().LVSum;
        });

        const nodesetup_t& setup = nodeSetup(tnode->Get().TypeTree);
        if(setup.Excluded)
            return;

        const auto IM_Sigma = setup.IM_Sigma;
        LOG(INFO) << "IM constraint for " << tnode->Get().TypeTree->Get().Name()
                  << " with sigma=" << IM_Sigma;
        node_constraints.emplace_back([tnode, IM_Sigma] () {
            node_t& node = tnode->Get();
            const double IM_calc = node.LVSum.M();
            const double IM_expected = tnode->Get().TypeTree->Get().Mass();
            return (IM_calc - IM_expected)/IM_Sigma;
        });
    });

    LOG(INFO) << "Have " << node_constraints.size() << " constraints at " << sum_daughters.size() << " nodes";

    // define the constraint
    auto IM_at_nodes = [tree_leaves, sum_daughters, node_constraints] (const vector<vector<double>>& v) {
        assert(v.empty() || v.size() == tree_leaves.size());
        // assign values v to leaves' LVSum
        for(unsigned i=0;i<v.size();i++) {
            auto& node = tree_leaves[i]->Get();
            const auto m = node.TypeTree->Get().Mass();
            node.LVSum = FitParticle::GetVector(v[i], m);
        }

        // sum daughters' Particle
        for(const auto f : sum_daughters)
            f();

        // calculate the IM constraint by evaluating the pre-defined functions
        vector<double> IM_diff(node_constraints.size());
        for(unsigned i=0;i<node_constraints.size();i++)
            IM_diff[i] = node_constraints[i]();
        return IM_diff;
    };

    aplcon->AddConstraint("IM_at_nodes",leave_names, IM_at_nodes);
}

void TreeFitter::SetLeaves(const TParticleList& photons)
{
    if(photons.size() != Photons.size())
        throw Exception("Given leave particles does not match configured TreeFitter");

    current_perm = permutations.begin();
    current_comb_ptr = std_ext::make_unique<current_comb_t>(photons, current_perm->size());
}

bool TreeFitter::NextFit(APLCON::Result_t& fit_result)
{
    assert(!permutations.empty());

    if(!current_comb_ptr)
        return false;

    auto& current_comb = *current_comb_ptr;

    if(current_perm == permutations.end()) {
        current_perm = permutations.begin();
        ++current_comb;
    }

    if(current_comb.Done())
        return false;

    const auto k = current_perm->size();
    const auto n = Photons.size();

    // by construction, the leaves are 0..k-1
    for(unsigned i=0;i<k;i++) {
        const TParticlePtr& p = current_comb.at(current_perm->at(i));
        SetPhotonEkThetaPhi(*Photons[i], p);
    }

    auto it_not_comb = current_comb.begin_not();
    assert(n>=k);
    assert((unsigned)distance(it_not_comb, current_comb.end_not()) == n - k);

    // and by construction, the non-leaves are from k..n-1
    for(unsigned i=k;i<n;i++) {
        SetPhotonEkThetaPhi(*Photons[i], *it_not_comb);
        ++it_not_comb;
    }

    if(Proton && Beam) {
        SetProton(Proton->Particle);
        SetEgammaBeam(Beam->E_before);
    }


    fit_result = KinFitter::DoFit();

    ++current_perm;

    return true;
}

TreeFitter::tree_t TreeFitter::MakeTree(ParticleTypeTree ptree)
{

    auto t = ptree->DeepCopy<node_t>([] (const ParticleTypeTree& n) { return n; });

    if(t->Get().TypeTree->Get() == ParticleTypeDatabase::BeamTarget) {
        for(const auto& daughter : t->Daughters()) {
            if(daughter->Get().TypeTree->Get() == ParticleTypeDatabase::Nucleon) {
                LOG(INFO) << "Removing nucleon from tree";
                daughter->Unlink();
                break;
            }
        }
    }

    t->Sort();
    return t;
}


