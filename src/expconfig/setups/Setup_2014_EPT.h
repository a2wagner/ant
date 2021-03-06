#include "Setup.h"

namespace ant {
namespace expconfig {
namespace setup {

class Setup_2014_EPT : public Setup
{
public:

    Setup_2014_EPT(const std::string& name, OptionsPtr opt);

    virtual double GetElectronBeamEnergy() const override;

    bool Matches(const TID& tid) const override;

    void BuildMappings(std::vector<hit_mapping_t>& hit_mappings,
                       std::vector<scaler_mapping_t>& scaler_mappings) const override;

    virtual ExpConfig::Reconstruct::candidatebuilder_config_t GetCandidateBuilderConfig() const override;
};

}}} // namespace ant::expconfig::setup
