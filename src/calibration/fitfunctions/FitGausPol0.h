#pragma once

#include "FitFunction.h"

#include "base/interval.h"
#include "calibration/gui/GUIbase.h"

#include <memory>

#include "TF1Knobs.h"

#include <list>
#include <string>
#include "base/std_ext.h"
#include "base/interval.h"

#include "Rtypes.h"

class TH1;
class TF1;

namespace ant {
namespace calibration {
namespace gui {

class FitGausPol0: public PeakingFitFunktion {
protected:
    TF1* func = nullptr;

    class AmpKnop: public VirtualKnob {
    protected:
        TF1* func = nullptr;
    public:

        AmpKnop(const std::string& name, TF1* Func);

        virtual double get() const override;
        virtual void set(double a) override;

    };

    class SigmaKnob: public VirtualKnob {
    protected:
        TF1* func = nullptr;
    public:

        SigmaKnob(const std::string& n, TF1* Func);

        virtual double get() const override;
        virtual void set(double a) override;

    };

public:
    FitGausPol0();

    virtual ~FitGausPol0();

    virtual void Draw() override;

    virtual void Fit(TH1* hist) override;
    virtual void SetDefaults(TH1* hist) override;

    virtual void SetRange(ant::interval<double> i) override;
    virtual ant::interval<double> GetRange() const override;

    virtual SavedState_t Save() const override;
    virtual void Load(const SavedState_t &data) override;

    virtual double GetPeakPosition() const override;
};

}
}
}
