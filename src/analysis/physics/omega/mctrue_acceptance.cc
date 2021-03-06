#include "physics/omega/mctrue_acceptance.h"
#include "plot/root_draw.h"

#include "utils/particle_tools.h"

#include "base/std_ext/string.h"

using namespace std;
using namespace ant;
using namespace ant::std_ext;
using namespace ant::analysis;
using namespace ant::analysis::physics;

const char* to_char(const std::string& s) {
    return s.c_str();
}

MCTrueAcceptance::MCTrueAcceptance(const std::string& name,OptionsPtr opts):
    Physics(name, opts), events_seen(0)
{
    detect = HistFac.makeTH1D("Geometric Acceptance (MC True)","Particle Type","% events | all particles found", BinSettings(0),"detection");
    for( auto& type : ParticleTypeDatabase::DetectableTypes() ) {
        detect->Fill(to_char(formatter() << "all " << type->PrintName() << " detected"), 1.0);
        detect->Fill(to_char(formatter() << "all " << type->PrintName() << " in CB"), 1.0);
        detect->Fill(to_char(formatter() << "all " << type->PrintName() << " in TAPS"), 1.0);
        detect->Fill(to_char(formatter() << "all " << type->PrintName() << " in CB & TAPS"), 1.0);
    }
    detect->Reset();
}

MCTrueAcceptance::det_hit_count_t MCTrueAcceptance::AllAccepted(const TParticleList& particles) {
    det_hit_count_t acc;

    for( auto& p : particles ) {
        const auto d = geo.DetectorFromAngles(*p);

        if(d == Detector_t::Type_t::CB ) {
            acc.cb++;
        } else if( d == Detector_t::Type_t::TAPS ) {
            acc.taps++;
        }
    }

    return acc;
}

bool MCTrueAcceptance::alldetectable(const TParticleList& particles) const
{
    for(const auto& p : particles) {
        if( Detector_t::Any_t::None == geo.DetectorFromAngles(p->Theta(), p->Phi()))
            return false;
    }
    return true;
}

void MCTrueAcceptance::ProcessEvent(const TEvent& event, manager_t&)
{

    detect->Fill("Events", 1);

    if(alldetectable(event.MCTrue->Particles.GetAll())) {
        detect->Fill("All Detected",1);
    }

    for( auto& type : ParticleTypeDatabase::DetectableTypes() ) {

        const TParticleList& particles = event.MCTrue->Particles.Get(*type);

        if(particles.size() > 0) {

            det_hit_count_t hit = AllAccepted(particles);

            if( particles.size() == hit.taps + hit.cb)
                detect->Fill(to_char("all " + type->PrintName() + " detected"), 1.0);
            if( particles.size() == hit.cb )
                detect->Fill(to_char("all " + type->PrintName() + " in CB"), 1.0);
            else if( particles.size() == hit.taps)
                detect->Fill(to_char("all " + type->PrintName() + " in TAPS"), 1.0);
            else if( particles.size() == hit.taps + hit.cb)
                detect->Fill(to_char("all " + type->PrintName() + " in CB & TAPS"), 1.0);
        }

    }

    events_seen++;

}

void MCTrueAcceptance::Finish()
{
    detect->Scale(100.0/events_seen);
}

void MCTrueAcceptance::ShowResult()
{
    canvas("MCTrueAcceptance") << detect << endc;
}

AUTO_REGISTER_PHYSICS(MCTrueAcceptance)
