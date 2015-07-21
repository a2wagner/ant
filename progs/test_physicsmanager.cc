#include "analysis/input/goat/GoatReader.h"
#include "analysis/data/Event.h"
#include "analysis/OutputManager.h"
#include "analysis/physics/Physics.h"

#include "base/Logger.h"

#include "TH1D.h"

#include <iostream>

using namespace std;
using namespace ant::output;
using namespace ant;

int main(int argc, char** argv) {
    SetupLogger(argc, argv);
    LOG(INFO) << "Debug Program. Please run with --v=9";

    OutputManager om;

    om.SetNewOutput("out.root");
    auto h1 = new TH1D("a","A",10,0,10);
    h1->Fill(2);

    om.SetNewOutput("out2.root");
    auto h2 = new TH1D("b","B",10,0,10);
    h2->Fill(3);

    PhysicsManager pm;

    pm.AddPhysics<DebugPhysics>("aaaa");

    return 0;
}