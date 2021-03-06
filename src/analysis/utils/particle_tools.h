#pragma once

#include "tree/TParticle.h"
#include "base/ParticleTypeTree.h"

#include <string>

#include "TLorentzVector.h"

class TH1;
class TTree;

namespace ant {
namespace analysis {
namespace utils {

struct ParticleVars {
    double E;
    double Theta;
    double Phi;
    double IM;

    ParticleVars(const TLorentzVector& lv, const ParticleTypeDatabase::Type& type) noexcept;
    ParticleVars(const TParticle& p) noexcept;
    ParticleVars(double e=0.0, double theta=0.0, double phi=0.0, double im=0.0) noexcept:
        E(e), Theta(theta), Phi(phi), IM(im) {}
    ParticleVars(const ParticleVars&) = default;
    ParticleVars(ParticleVars&&) = default;
    ParticleVars& operator=(const ParticleVars&) =default;
    ParticleVars& operator=(ParticleVars&&) =default;
    virtual void SetBranches(TTree* tree, const std::string& prefix);
    virtual void Clear();
};

struct ParticleTools {

    /**
     * @brief Construct a string describing the particle dacay tree using the particle::PrintName()s
     * @param particles
     * @return
     */
    static std::string GetDecayString(const TParticleTree_t& particletree);

    static std::string GetDecayString(const ParticleTypeTree& particletypetree, bool usePrintName = true);

    /**
     * @brief SanitizeDecayString replaces all special characters by _
     * @param decaystring input decaystring
     * @return string suitable to be used as histogram name prefix
     */
    static std::string SanitizeDecayString(std::string decaystring);


    /** @brief Construct a string describing the production channel (e.g. gamma p -> p pi0 for pion production)
     * @param particle
     * @return
     */
    static std::string GetProductionChannelString(const TParticleTree_t& particletree);

    static ParticleTypeTree GetProducedParticle(const ParticleTypeTree& particletypetree);


    /**
     * @brief Find the first Particle of given type in particle list
     * @param type Type to search for
     * @param particles List to search in
     * @return The Particle found
     */
    static const TParticlePtr FindParticle(const ParticleTypeDatabase::Type& type, const TParticleList& particles);

    static const TParticlePtr FindParticle(const ParticleTypeDatabase::Type& type, const TParticleTree_t& particletree,
                                           size_t maxlevel = std::numeric_limits<size_t>::max());

    static const TParticleList FindParticles(const ParticleTypeDatabase::Type& type, const TParticleTree_t& particletree,
                                             size_t maxlevel = std::numeric_limits<size_t>::max());


    /**
     * @brief FillIMCombinations loops over all n tuples of given particles, builds sum and fills invariant mass
     * @param h histogram to be filled with Fill(invariant mass)
     * @param n multiplicity or number of particles drawn from particles
     * @param particles list of particles
     */
    static void FillIMCombinations(TH1* h, unsigned n, const TParticleList& particles);

    static void FillIMCombinations(std::function<void(double)> filler, unsigned n, const TParticleList& particles);


    static bool SortParticleByName(const TParticlePtr& a, const TParticlePtr& b);

    static bool MatchByParticleName(const TParticlePtr& a, const ParticleTypeDatabase::Type& b);

};

}
}

}
