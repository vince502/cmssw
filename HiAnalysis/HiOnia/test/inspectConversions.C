// inspectConversions.C - Inspect oniaPhotonCandidates structure
// Usage: root -l -b -q inspectConversions.C

#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <vector>

void inspectConversions() {
    TFile *f = TFile::Open("root://cms-xrd-global.cern.ch//store/hidata/OORun2025/IonPhysics1/MINIAOD/PromptReco-v1/000/394/154/00000/35d3344a-07b5-4ed1-8c1d-6e1036c9ad4c.root");
    if (!f || f->IsZombie()) {
        std::cout << "Cannot open file" << std::endl;
        return;
    }
    
    TTree *events = (TTree*)f->Get("Events");
    
    // Set up branch
    std::vector<pat::CompositeCandidate> *convCands = nullptr;
    events->SetBranchAddress("patCompositeCandidates_oniaPhotonCandidates_conversions_RECO.", &convCands);
    
    int nEventsWithConv = 0;
    int maxEvents = 100;
    
    for (Long64_t i = 0; i < std::min((Long64_t)maxEvents, events->GetEntries()); i++) {
        events->GetEntry(i);
        
        if (convCands->size() > 0) {
            nEventsWithConv++;
            std::cout << "\n=== Event " << i << " has " << convCands->size() << " conversion candidates ===" << std::endl;
            
            for (size_t j = 0; j < convCands->size(); j++) {
                const pat::CompositeCandidate& cand = convCands->at(j);
                std::cout << "  Cand " << j << ": pT=" << cand.pt() << " eta=" << cand.eta() 
                          << " phi=" << cand.phi() << " mass=" << cand.mass() << std::endl;
                std::cout << "    nDaughters: " << cand.numberOfDaughters() << std::endl;
                
                // Print userFloat names
                std::cout << "    userFloats: ";
                const std::vector<std::string>& ufNames = cand.userFloatNames();
                for (const auto& name : ufNames) {
                    std::cout << name << "=" << cand.userFloat(name) << " ";
                }
                std::cout << std::endl;
                
                // Print userInt names
                std::cout << "    userInts: ";
                const std::vector<std::string>& uiNames = cand.userIntNames();
                for (const auto& name : uiNames) {
                    std::cout << name << "=" << cand.userInt(name) << " ";
                }
                std::cout << std::endl;
            }
            
            if (nEventsWithConv >= 3) break; // Show first 3 events with conversions
        }
    }
    
    std::cout << "\nTotal events with conversions in first " << maxEvents << ": " << nEventsWithConv << std::endl;
    f->Close();
}
