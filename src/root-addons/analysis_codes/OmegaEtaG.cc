#include "OmegaEtaG.h"

#include "TTree.h"
#include "TFile.h"
#include "TH2.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TDirectory.h"
#include "TCut.h"
#include "THStack.h"

#include "root-addons/analysis_codes/hstack.h"
#include "analysis/plot/root_draw.h"
#include "base/std_ext/string.h"

#include "TreeTools.h"

#include "analysis/physics/omega/omega.h"
#include "analysis/utils/particle_tools.h"

using namespace std;
using namespace ant;
using namespace ant::std_ext;
using namespace ant::analysis;

struct OmegaEtaG_hists {
    TH1* ggg_IM_free;
    TH1* ggg_IM_fit;
    TH1* mm_free;
    TH1* mm_fit;
    TH2* mm_gggIM_free;
    TH2* mm_gggIM_fit;
    TH1* bachelor;
    TH1* protonThetaE;
    TH1* ggIM;
    map<string, TH1*> pulls;
    TH1* fit_chi2dof;
    TH1* fit_prob;

    const std::string pref;

    OmegaEtaG_hists(const std::string& prefix) : pref(prefix) {}


    void DrawCanvas() {
        canvas("Kin Fit")
                << fit_chi2dof
                << fit_prob
                << endc;

        canvas cpulls("Pulls");
        for(auto& h : pulls)
            cpulls << h.second;
        cpulls << endc;

        canvas("Cut Varaibles")
                << ggg_IM_free << ggg_IM_fit
                << mm_free << mm_fit
                << drawoption("colz") << mm_gggIM_free << mm_gggIM_fit
                << endc;

        canvas("Results")
                << ggIM << bachelor
                << drawoption("colz") << protonThetaE
                << endc;
    }

    void Make(TTree* tree, const TCut& extra_cut, const double binscale) {
        const TCut cut_fit("EPB_chi2dof<4 && EPB_status==0");
        const TCut cut_vars("ggg_IM>700&&ggg_IM<840");

        const TCut prompt_random("TagW");

        const TCut all = prompt_random * (cut_fit + cut_vars + extra_cut);

        const BinSettings Ebins (1600*binscale,   0, 1600);

        const BinSettings Chi2bins (250*binscale, 0,   25);
        const BinSettings probbins (250*binscale, 0,   1);

        const BinSettings IMbins(1600*binscale,   0, 1600);
        const BinSettings MMbins(1600*binscale, 400, 2000);

        const BinSettings MMgggIMbins_X(600*binscale, 0, 1200);
        const BinSettings MMgggIMbins_Y(750*binscale, 500, 2000);

        const BinSettings pThetaBins(500*binscale, 0, 50);
        const BinSettings pEbins (1000*binscale,   0, 1000);


        ggg_IM_free   = Draw(tree, "ggg_IM",    prompt_random*extra_cut,         "3#gamma IM [MeV]", "", IMbins, pref+"_ggg_IM_free");
        ggg_IM_free->SetTitle("3#gamma IM");

        ggg_IM_fit    = Draw(tree, "ggg_IM",    prompt_random*(extra_cut*cut_fit), "3#gamma IM [MeV]", "", IMbins, pref+"_ggg_IM_fit");
        ggg_IM_fit->SetTitle("3#gamma IM, #chi^{2} cut");

        mm_free       = Draw(tree, "mmvect_IM", prompt_random*extra_cut,         "MM [MeV]", "", MMbins, pref+"_mm_free");
        mm_free->SetTitle("Missig Mass");

        mm_fit        = Draw(tree, "mmvect_IM", prompt_random*(extra_cut*cut_fit), "MM [MeV]", "", MMbins, pref+"_mm_fit");
        mm_fit->SetTitle("Missing Mass, #chi^{2} cut");

        mm_gggIM_free = Draw(tree, "mmvect_IM:ggg_IM", prompt_random*extra_cut,         "3#gamma IM[MeV]", "MM [MeV]", MMgggIMbins_X, MMgggIMbins_Y, pref+"_mm_gggIM_free");
        mm_gggIM_free->SetTitle("Missing Mass vs. 3#gamma IM");

        mm_gggIM_fit  = Draw(tree, "mmvect_IM:ggg_IM", prompt_random*(extra_cut*cut_fit), "3#gamma IM[MeV]", "MM [MeV]", MMgggIMbins_X, MMgggIMbins_Y, pref+"_mm_gggIM_fit");
        mm_gggIM_fit->SetTitle("Missing Mass vs. 3#gamma IM, #chi^{2} cut");

        fit_chi2dof   = Draw(tree, "EPB_chi2dof",     prompt_random*extra_cut,        "#chi^{2}/dof", "", Chi2bins, pref+"_fit_chi2dof");
        fit_prob      = Draw(tree, "EPB_probability", prompt_random*extra_cut,        "probabiliyt", "",  probbins, pref+"_fit_prob");

    //    auto bachelor_free     = Draw(tree, "BachelorE", prompt_random*extra_cut,             "E #gamma_{#omega} [MeV]", "", Ebins);
    //    auto bachelor_fit      = Draw(tree, "BachelorE", propmt_randomt*cut_fit,     "E #gamma_{#omega} [MeV]", "", Ebins);
        bachelor          = Draw(tree, "BachelorE", all,                        "E #gamma_{#omega} [MeV]", "", Ebins, pref+"_bachelor");
        bachelor->SetTitle("Bachelor Photon, (#chi^{2}, mm, 3#gammaIM)-cut");

        protonThetaE  = Draw(tree, "fitted_p_Theta:fitted_p_E", all, "Fitted Proton E [MeV]", "Fitted Proton #theta [#circ]", pEbins, pThetaBins, pref+"_protonThetaE");
        protonThetaE->SetTitle("Fitted Proton #theta vs. Energy, (#chi^{2}, mm, 3#gammaIM)-cut");

        ggIM          = Draw(tree, "ggIM",      all,                    "2#gamma IM [MeV]", "", IMbins, pref+"_ggIM");
        ggIM->SetTitle("2#gamma sub IM, (#chi^{2}, mm, 3#gammaIM)-cut");

    //    for(const auto& particle : {"Proton", "Photon0", "Photon1","Photon2"}) {
    //        for(const auto& var : {"Phi_pull","Theta_pull","Ek_pull"}) {
    //            const string branch = formatter() << "EPB_" << particle << "_" << var;
    //            cout << branch << endl;
    //            auto h = Draw(tree, branch, prompt_random*cut_fit, "", "", BinSettings(200,-20,20), pref+"_"+particle+"_"+var);
    //            if(h)
    //                pulls[branch] = h;
    //        }
    //    }
    };

};

void OmegaEtaG::Plot(TTree* tree, const TCut& extra_cut, const double binscale)
{
    OmegaEtaG_hists h("h");

    h.Make(tree, extra_cut, binscale);

    h.DrawCanvas();
}

void OmegaEtaG::PlotBGs(TTree *tree, const double binscale)
{
//    const unsigned nch = ant::analysis::physics::OmegaEtaG2::re().size();
//    for(unsigned i=0;i<nch;++i) {
//        cout << "===== " << i << " ============"<<endl;
//        OmegaEtaG_hists h(formatter()<<"BG"<<i);
//        const string sel = (formatter() << "SigBgFlag==" << i);
//        h.Make(tree, sel.c_str(), binscale);
//    }

//    OmegaEtaG_hists h(formatter() << "BG" << nch);
//    const string sel = (formatter() << "SigBgFlag>=" << nch);
//    h.Make(tree, sel.c_str(), binscale);
}

void OmegaEtaG::DataMC(TFile* mc_file, TFile* data_file, const double mcscale)
{
    canvas c("MC Data");
    for(int i=0;i<8; ++i) {
        TH1* mc   = nullptr;
        TH1* data = nullptr;

        const auto histname = Form("h1d_%d",i);

        mc_file->GetObject(histname, mc);
        if(!mc) {
            cout << "Could not get " << histname << " from mc file" << endl;
            continue;
        }
        cout << "MC: " << mc->GetName() << endl;
        mc->SetLineColor(kBlue);
        const string backup_title(mc->GetTitle());
        mc->SetTitle("mc");
        mc->Scale(mcscale);

        data_file->GetObject(histname, data);
        if(!data) {
            cout << "Could not get " << histname << " from mc file" << endl;
            continue;
        }
        data->SetLineColor(kBlack);
        data->SetTitle("data");


        hstack* stack = new hstack(Form("stack_%d",i), backup_title);
        *stack << mc << data;

        c << drawoption("nostack") << padoption::Legend << stack;

    }

    c << endc;
}

class StringColorManager {
protected:
    std::map<std::string, Color_t> used;
    static const std::vector<Color_t> cols;

public:

    Color_t Get(const std::string& s) {
        const auto entry = used.find(s);
        if(entry == used.end()) {
            const auto index = used.size() % cols.size();
            const auto color = cols.at(index);
            used.emplace(s,color);
            return color;
        } else {
            return entry->second;
        }
    }

    void Clear() {
        used.clear();
    }

    void PrintList() const {
        for(const auto& entry : used) {
            cout << entry.first << endl;
        }
    }

};

const std::vector<Color_t> StringColorManager::cols = {kRed, kGreen+1, kBlue, kMagenta, kCyan, kOrange, kPink+9, kSpring+10, kGray+1, kTeal, kGray, kYellow};




void OmegaEtaG::DataMCBGs(TFile* mc_file, TFile* data_file, const double mcscale)
{
    StringColorManager colors;

    auto channels = ant::analysis::physics::OmegaEtaG2::makeChannels();

    canvas c("MC Data");

//    for(const auto& hname : {"ggg_IM_free","ggg_IM_fit","mm_free","mm_fit","fit_chi2dof","fit_prob","bachelor","ggIM"}) {


//        // ------- Data

//        TH1* data = nullptr;

//        const string histname = formatter() << "h_" << hname;
//        data_file->GetObject(histname.c_str(), data);

//        if(!data) {
//            cout << "Could not get " << histname << " from mc file" << endl;
//            return;
//        }

//        const string backup_title = data->GetTitle();

//        data->SetLineColor(kBlack);
//        data->SetTitle("Data");

//        hstack* stack = new hstack(formatter() << "stack_" << hname, backup_title);
//        *stack << data;

//        const auto data_entries = data->Integral();

//        // ----- MC

//        TH1* mcsum = nullptr;

//        std::list<TH1*> mc_hists;

//        for(size_t bg=0; bg<=channels.size(); ++bg) {

//            const string histname = formatter() << "BG" << bg << "_" << hname;

//            TH1* mch = nullptr;

//            mc_file->GetObject(histname.c_str(), mch);
//            if(!mch) {
//                cout << "Could not get " << histname << " from mc file" << endl;
//                continue;
//            }
//            if(bg==channels.size()) {
//                mch->SetTitle("Others");
//                mch->SetLineColor(colors.Get("Others"));
//            } else {
//                const string chname = utils::ParticleTools::GetDecayString(channels.at(int(bg)));
//                mch->SetLineColor(colors.Get(to_string(bg)));
//                mch->SetTitle(chname.c_str());
//            }

//            if(bg==0) {
//                mcsum = dynamic_cast<TH1D*>(mch->Clone());
//                const string name = formatter() << "MCSUM_" << hname;
//                mcsum->SetName(name.c_str());
//                mcsum->SetTitle("MC Sum");
//                mcsum->SetLabelColor(colors.Get("MC Sum"));
//                mc_hists.push_back(mcsum);
//            } else {
//                mcsum->Add(mch);
//            }

//            mc_hists.push_back(mch);
//        }

//        const auto mc_entries   = mcsum->Integral();
//        const auto ratio = data_entries / mc_entries;

//        cout << "Scale factor " << ratio << endl;

//        mc_hists.sort([] (TH1* h1, TH1* h2){ return h1->Integral() > h2->Integral();});

//        for(auto& h : mc_hists) {
//            cout << h->GetTitle() << endl;
//            h->Scale(mcscale);
//            *stack << h;
//        }

//        c << drawoption("nostack") << padoption::Legend << stack;


//    } // for hname

//    c << endc;
}
