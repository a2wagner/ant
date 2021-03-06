#pragma once

#include "Rtypes.h"
#include "TNamed.h"

#include <string>
#include <vector>

#ifndef __CINT__
#include "analysis/plot/root_draw.h"
#include "base/printable.h"
#include "base/interval.h"
#else
// disable the override keyword
#define override
#endif

class THStack;
class TH1;

namespace ant {

/**
 * @brief The hstack class
 *
 * Intelligent wrapper for ROOT's THStack.
 *
 * Overloads the << operator to add histograms to it, and of course
 * can be drawn the same way with the canvas class.
 *
 * The axis labels are always taken from the last histogram added.
 * @see ant::canvas
 *
 * Extra features include intelligent legend building,
 * it can be written to ROOT files (provided the referenced TH1's are also present),
 * it can be merged (with Ant-hadd in order to load the class ant::hstack properly)
 *
 */

#ifndef __CINT__
struct hstack :  TNamed, printable_traits
#else
struct hstack : TNamed
#endif
{

// for CINT, this class looks empty (except TNamed inheritance and some methods)
#ifndef __CINT__

    struct options_t {
        bool UseIntelliLegend;
        bool IgnoreEmptyHist;
        bool DrawNoStack;
        bool ShowEntriesInLegend;
        // by default we don't use any of those fancy options
        options_t(bool useIntelliLegend = false,
                  bool ignoreEmptyHist = false,
                  bool drawNoStack = false,
                  bool showEntriesInLegend = false) :
            UseIntelliLegend(useIntelliLegend),
            IgnoreEmptyHist(ignoreEmptyHist),
            DrawNoStack(drawNoStack),
            ShowEntriesInLegend(showEntriesInLegend)
        {}

        static const options_t all_enabled;

        bool operator== (const options_t& rhs) const;

        template<typename Archive>
        void serialize(Archive archive) {
            archive(UseIntelliLegend, IgnoreEmptyHist, DrawNoStack, ShowEntriesInLegend);
        }
    };

    hstack(const std::string& name, const std::string& title="",
           const options_t& options_ = {});

    bool IsCompatible(const hstack& other) const;

    hstack(hstack&&) = default;
    hstack& operator= (hstack&&) = default;

    hstack& operator<< (TH1* hist);
    hstack& operator<< (const drawoption& c);

    virtual std::ostream& Print( std::ostream& s) const override;

    template<typename Archive>
    void serialize(Archive archive) {
        archive(static_cast<TNamed&>(*this),
                hists, xlabel, ylabel, options);
        checkHists();
    }

    static double Global_MC_Scaling;
    static std::map<TH1*, double> Scaled_Hists;
    void UpdateMCScaling();

    static interval<double> GlobalYAxisRange;
    static interval<interval<double>> GlobalLegendPosition;

protected:

    struct hist_t {

        hist_t(TH1* ptr, const std::string& option) :
            Path(GetPath(ptr)),
            Ptr(ptr),
            Option(option)
        {}

        // clear the Ptr on copy
        // checkHists() needs to be called then
        hist_t(const hist_t& other) {
            Path = other.Path;
            Option = other.Option;
            Ptr = nullptr;
        }
        hist_t& operator= (const hist_t&) = delete;

        // move is ok
        hist_t& operator= (hist_t&&) = default;
        hist_t(hist_t&&) = default;

        std::string Path;
        TH1* Ptr = nullptr;
        std::string Option;

        hist_t() {}


        template<typename Archive>
        void load(Archive archive) {
            archive(Path, Option);
            Ptr = GetPtr(Path);
        }
        template<typename Archive>
        void save(Archive archive) const {
            archive(Path, Option);
        }

        static TH1* GetPtr(const std::string& path);
        static std::string GetPath(const TH1* ptr);
    };

    using hists_t = std::vector<hist_t>;
    hists_t hists;

    std::string current_option;

    std::string xlabel;
    std::string ylabel;
    std::string title;

    options_t options;

    void checkHists();


#endif // __CINT__

public:

    virtual void Print(const char* option) const override;
    virtual void Print() const; // *MENU*
    virtual void Draw(const char* option) override;
    virtual void Browse(TBrowser* b) override;

    virtual void SetGlobalMCScaling(double scaling); // *MENU*
    virtual void SetGlobalYAxisRange(double low, double high); // *MENU*
    virtual void SetGlobalLegendPosition(double x1=0.5, double y1=0.67, double x2=0.88, double y2=0.88); // *MENU*


    // to be used with Ant-hadd
    Long64_t Merge(TCollection* li);

    hstack();
    virtual ~hstack();
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
    ClassDef(hstack, 0)
#ifdef __clang__
#pragma clang diagnostic pop
#endif

private:
    // prevent ROOTcint from creating copy-constructors
    hstack(const hstack&);
    hstack& operator= (const hstack&);

};

}
