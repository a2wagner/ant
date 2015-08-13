#include "FitFunction.h"

#include "base/interval.h"
#include "base/Logger.h"
#include "TF1Knobs.h"
#include "BaseFunctions.h"

#include "TF1.h"
#include "TH1.h"

#include <algorithm>

using namespace ant;
using namespace ant::calibration;
using namespace ant::calibration::gui;


ant::interval<double> FitFunction::getRange(const TF1* func)
{
    interval<double> i;
    func->GetRange(i.Start(), i.Stop());
    return i;
}

void FitFunction::setRange(TF1* func, const ant::interval<double>& i)
{
    func->SetRange(i.Start(), i.Stop());
}

void FitFunction::saveTF1(const TF1 *func, SavedState_t &out)
{
    auto range = getRange(func);
    out.push_back(range.Start());
    out.push_back(range.Stop());

    for(int i=0; i <func->GetNpar(); ++i ) {
        out.push_back(func->GetParameter(i));
    }
}

void FitFunction::loadTF1(SavedState_t::const_iterator &data_pos, TF1 *func)
{
    setRange(func,{*(data_pos++),*(data_pos++)});

    std::copy(data_pos,data_pos+func->GetNpar(),func->GetParameters());
}

FitFunction::~FitFunction()
{}
