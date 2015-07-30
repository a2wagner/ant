

#include "analysis/input/DataReader.h"
#include "analysis/input/ant/AntReader.h"
#include "analysis/input/goat/GoatReader.h"
#include "analysis/OutputManager.h"

#include "analysis/physics/Physics.h"
#include "analysis/physics/omega/omega.h"
#include "analysis/physics/common/DataOverview.h"
#include "analysis/physics/common/CandidatesAnalysis.h"

#include "expconfig/ExpConfig.h"

#include "unpacker/Unpacker.h"
#include "unpacker/RawFileReader.h"

#include "tree/UnpackerReader.h"
#include "tree/UnpackerWriter.h"
#include "tree/THeaderInfo.h"

#include "base/std_ext.h"
#include "base/Logger.h"
#include "base/CmdLine.h"
#include "base/ReadTFiles.h"

#include "TRint.h"
#include "TClass.h"
#include "TTree.h"
#include "TFile.h"

#include <sstream>
#include <string>
#include <chrono>

using namespace std;
using namespace ant::output;
using namespace ant;
using namespace ant::analysis;


bool running = true;
void myCrashHandler(int sig);


int main(int argc, char** argv) {
    SetupLogger();
    el::Helpers::setCrashHandler(myCrashHandler);

    TCLAP::CmdLine cmd("ant", ' ', "0.1");
    auto cmd_verbose = cmd.add<TCLAP::ValueArg<int>>("v","verbose","Verbosity level (0..9)", false, 0,"int");
    auto cmd_input  = cmd.add<TCLAP::MultiArg<string>>("i","input","Input files",true,"string");
    auto cmd_setup  = cmd.add<TCLAP::ValueArg<string>>("s","setup","Choose setup",false,"","string");
    auto cmd_unpackerout  = cmd.add<TCLAP::ValueArg<string>>("u","unpackerout","Unpacker stage output file",false,"","string");


    cmd.parse(argc, argv);
    if(cmd_verbose->isSet()) {
        el::Loggers::setVerboseLevel(cmd_verbose->getValue());
    }

    // build the general ROOT file manager first
    auto filemanager = make_shared<ReadTFiles>();
    for(const auto& inputfile : cmd_input->getValue()) {
        VLOG(5) << "ROOT File Manager: Looking at file " << inputfile;
        if(filemanager->OpenFile(inputfile))
            LOG(INFO) << "Opened file '" << inputfile << "' as ROOT file";
        else
            VLOG(5) << "Could not add " << inputfile << " to ROOT file manager";
    }

    // then init the unpacker root input file manager
    auto unpackerFile = std_ext::make_unique<tree::UnpackerReader>(filemanager);

    // search for header info?
    if(unpackerFile->OpenInput()) {
        LOG(INFO) << "Found complete set of input ROOT trees for unpacker";
        THeaderInfo headerInfo;
        if(unpackerFile->GetUniqueHeaderInfo(headerInfo)) {
            VLOG(5) << "Found unique header info " << headerInfo;
            if(!headerInfo.SetupName.empty()) {
                ExpConfig::ManualSetupName = headerInfo.SetupName;
                LOG(INFO) << "Using header info to manually set the setup name to " << ExpConfig::ManualSetupName;
            }
        }
    }

    // override the setup name from cmd line
    if(cmd_setup->isSet()) {
        ExpConfig::ManualSetupName = cmd_setup->getValue();
        if(ExpConfig::ManualSetupName.empty())
            LOG(INFO) << "Commandline override to auto-search for setup config (might fail)";
        else
            LOG(INFO) << "Commandline override setup name to '" << ExpConfig::ManualSetupName << "'";
    }


    // now we can try to open the files with an unpacker
    std::unique_ptr<Unpacker::Reader> unpacker = nullptr;
    for(const auto& inputfile : cmd_input->getValue()) {
        VLOG(5) << "Unpacker: Looking at file " << inputfile;
        try {
            auto unpacker_ = Unpacker::Get(inputfile);
            if(unpacker != nullptr && unpacker_ != nullptr) {
                LOG(ERROR) << "Can only handle one unpacker, but input files suggest to use more than one.";
                return 1;
            }
            unpacker = move(unpacker_);
        }
        catch(Unpacker::Exception e) {
            VLOG(5) << "Unpacker: " << e.what();
            unpacker = nullptr;
        }
        catch(RawFileReader::Exception e) {
            LOG(WARNING) << "Unpacker: Error opening file "<<inputfile<<": " << e.what();
            unpacker = nullptr;
        }
        catch(ExpConfig::ExceptionNoConfig) {
            LOG(ERROR) << "The inputfile " << inputfile << " cannot be unpacked without a manually specified setupname. "
                       << "Consider using " << cmd_setup->longID();
            return 1;
        }
    }

    // select the right source for the unpacker stage
    if(unpacker != nullptr && unpackerFile->IsOpen()) {
        LOG(WARNING) << "Found file suitable for unpacker and ROOT file for unpacker stage, preferring raw data file";
    }
    else if(unpackerFile->IsOpen()) {
        LOG(INFO) << "Running unpacker stage from input ROOT file(s)";
        unpacker = move(unpackerFile);
    }
    else if(unpacker != nullptr) {
        LOG(INFO) << "Running unpacker stage from raw data file";
    }


    unique_ptr<tree::UnpackerWriter> unpacker_writer = nullptr;
    if(cmd_unpackerout->isSet()) {
        unpacker_writer = std_ext::make_unique<tree::UnpackerWriter>(cmd_unpackerout->getValue());
        LOG(INFO) << "Writing unpacker stage output to " << cmd_unpackerout->getValue();
    }

    if(!unpacker) {
        LOG(ERROR) << "No unpacker available, exit.";
        return 1;
    }

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    unsigned nItems = 0;
    while(auto item = unpacker->NextItem()) {
        if(!running)
            break;
        if(unpacker_writer)
            unpacker_writer->Fill(item);
        nItems++;
    }

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    cout << "Processed " << nItems << " unpacker items, speed "
         << nItems/elapsed_seconds.count() << " Items/s" << endl;

    return 0;
}

void myCrashHandler(int sig) {
    if(sig == SIGINT) {
        running = false;
        return;
    }
    // FOLLOWING LINE IS ABSOLUTELY NEEDED AT THE END IN ORDER TO ABORT APPLICATION
    el::Helpers::crashAbort(sig);
}