#pragma once

#include "TID.h"

#include "TNamed.h"

#include <string>

namespace ant {

/**
 * @brief The TAntHeader class holds information about the histogram file
 */
#ifndef __CINT__
struct TAntHeader : TNamed, printable_traits
#else
struct TAntHeader : TNamed
#endif
{
    ant::TID FirstID;
    ant::TID LastID;
    std::string SetupName;
    std::string CmdLine;
    std::string WorkingDir;
    std::string GitInfo;
    std::string GitInfoDatabase;

    TAntHeader(const std::string& title="");

    /**
     * @brief IsCompatible checks if other TAntHeader might be from same calibration iteration
     * @param other
     * @return
     */
    bool IsCompatible(const TAntHeader& other) const;


#ifndef __CINT__
    virtual std::ostream& Print( std::ostream& s) const override;
#endif

    // for convenience when used within ROOT shell
    virtual void Print(Option_t*) const override;
    virtual void Print() const; //*MENU*
    virtual void Browse(TBrowser* b) override;

    // to be used with Ant-hadd
    Long64_t Merge(TCollection* li);

    virtual ~TAntHeader();
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
    ClassDef(TAntHeader, ANT_UNPACKER_ROOT_VERSION)
#ifdef __clang__
#pragma clang diagnostic pop
#endif
};

}
