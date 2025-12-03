// Flatten the dimuon tree structure (hionia/myTree)
// Usage: root -l -b -q 'flattenDimuonTree.C("combined_V1.root", "output_flat.root")'

#include "TFile.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include <iostream>
#include <vector>

void flattenDimuonTree(const char* inputFile = "combined_V1.root", 
                       const char* outputFile = "FlatDimuonTree.root") {
    
    std::cout << "Opening input file: " << inputFile << std::endl;
    TFile* fin = TFile::Open(inputFile, "READ");
    if (!fin || fin->IsZombie()) {
        std::cerr << "ERROR: Cannot open input file!" << std::endl;
        return;
    }
    
    TTree* tree = (TTree*)fin->Get("hionia/myTree");
    if (!tree) {
        std::cerr << "ERROR: Cannot find hionia/myTree!" << std::endl;
        fin->Close();
        return;
    }
    
    std::cout << "Input tree has " << tree->GetEntries() << " events" << std::endl;
    
    // Input branches - Event info
    UInt_t eventNb, runNb, LS;
    Float_t zVtx;
    Int_t Centrality;
    Short_t nPV;
    
    // Input branches - Dimuon
    Short_t Reco_QQ_size;
    Short_t Reco_QQ_type[1000];
    Short_t Reco_QQ_sign[1000];
    std::vector<float> *Reco_QQ_4mom_pt = 0;
    std::vector<float> *Reco_QQ_4mom_eta = 0;
    std::vector<float> *Reco_QQ_4mom_phi = 0;
    std::vector<float> *Reco_QQ_4mom_m = 0;
    Short_t Reco_QQ_mupl_idx[1000];
    Short_t Reco_QQ_mumi_idx[1000];
    ULong64_t Reco_QQ_trig[1000];
    Float_t Reco_QQ_ctau[1000];
    Float_t Reco_QQ_ctauErr[1000];
    Float_t Reco_QQ_cosAlpha[1000];
    Float_t Reco_QQ_VtxProb[1000];
    Float_t Reco_QQ_dca[1000];
    
    // Input branches - Muons
    Short_t Reco_mu_size;
    Short_t Reco_mu_type[1000];
    Short_t Reco_mu_charge[1000];
    std::vector<float> *Reco_mu_4mom_pt = 0;
    std::vector<float> *Reco_mu_4mom_eta = 0;
    std::vector<float> *Reco_mu_4mom_phi = 0;
    std::vector<float> *Reco_mu_4mom_m = 0;
    ULong64_t Reco_mu_trig[1000];
    Bool_t Reco_mu_InTightAcc[1000];
    Bool_t Reco_mu_InLooseAcc[1000];
    Bool_t Reco_mu_highPurity[1000];
    Bool_t Reco_mu_TMOneStaTight[1000];
    Bool_t Reco_mu_isPF[1000];
    Bool_t Reco_mu_isTracker[1000];
    Bool_t Reco_mu_isGlobal[1000];
    Bool_t Reco_mu_isSoftCutBased[1000];
    Float_t Reco_mu_softMvaRun3Value[1000];
    Bool_t Reco_mu_isHybridSoft[1000];
    Bool_t Reco_mu_isMediumCutBased[1000];
    Bool_t Reco_mu_isTightCutBased[1000];
    Int_t Reco_mu_nPixValHits[1000];
    Int_t Reco_mu_nMuValHits[1000];
    Int_t Reco_mu_nTrkHits[1000];
    Float_t Reco_mu_normChi2_inner[1000];
    Int_t Reco_mu_nPixWMea[1000];
    Int_t Reco_mu_nTrkWMea[1000];
    
    // Set branch addresses - Event
    tree->SetBranchAddress("eventNb", &eventNb);
    tree->SetBranchAddress("runNb", &runNb);
    tree->SetBranchAddress("LS", &LS);
    tree->SetBranchAddress("zVtx", &zVtx);
    tree->SetBranchAddress("Centrality", &Centrality);
    tree->SetBranchAddress("nPV", &nPV);
    
    // Set branch addresses - Dimuon
    tree->SetBranchAddress("Reco_QQ_size", &Reco_QQ_size);
    tree->SetBranchAddress("Reco_QQ_type", Reco_QQ_type);
    tree->SetBranchAddress("Reco_QQ_sign", Reco_QQ_sign);
    tree->SetBranchAddress("Reco_QQ_4mom_pt", &Reco_QQ_4mom_pt);
    tree->SetBranchAddress("Reco_QQ_4mom_eta", &Reco_QQ_4mom_eta);
    tree->SetBranchAddress("Reco_QQ_4mom_phi", &Reco_QQ_4mom_phi);
    tree->SetBranchAddress("Reco_QQ_4mom_m", &Reco_QQ_4mom_m);
    tree->SetBranchAddress("Reco_QQ_mupl_idx", Reco_QQ_mupl_idx);
    tree->SetBranchAddress("Reco_QQ_mumi_idx", Reco_QQ_mumi_idx);
    tree->SetBranchAddress("Reco_QQ_trig", Reco_QQ_trig);
    tree->SetBranchAddress("Reco_QQ_ctau", Reco_QQ_ctau);
    tree->SetBranchAddress("Reco_QQ_ctauErr", Reco_QQ_ctauErr);
    tree->SetBranchAddress("Reco_QQ_cosAlpha", Reco_QQ_cosAlpha);
    tree->SetBranchAddress("Reco_QQ_VtxProb", Reco_QQ_VtxProb);
    tree->SetBranchAddress("Reco_QQ_dca", Reco_QQ_dca);
    
    // Set branch addresses - Muons
    tree->SetBranchAddress("Reco_mu_size", &Reco_mu_size);
    tree->SetBranchAddress("Reco_mu_type", Reco_mu_type);
    tree->SetBranchAddress("Reco_mu_charge", Reco_mu_charge);
    tree->SetBranchAddress("Reco_mu_4mom_pt", &Reco_mu_4mom_pt);
    tree->SetBranchAddress("Reco_mu_4mom_eta", &Reco_mu_4mom_eta);
    tree->SetBranchAddress("Reco_mu_4mom_phi", &Reco_mu_4mom_phi);
    tree->SetBranchAddress("Reco_mu_4mom_m", &Reco_mu_4mom_m);
    tree->SetBranchAddress("Reco_mu_trig", Reco_mu_trig);
    tree->SetBranchAddress("Reco_mu_InTightAcc", Reco_mu_InTightAcc);
    tree->SetBranchAddress("Reco_mu_InLooseAcc", Reco_mu_InLooseAcc);
    tree->SetBranchAddress("Reco_mu_highPurity", Reco_mu_highPurity);
    tree->SetBranchAddress("Reco_mu_TMOneStaTight", Reco_mu_TMOneStaTight);
    tree->SetBranchAddress("Reco_mu_isPF", Reco_mu_isPF);
    tree->SetBranchAddress("Reco_mu_isTracker", Reco_mu_isTracker);
    tree->SetBranchAddress("Reco_mu_isGlobal", Reco_mu_isGlobal);
    tree->SetBranchAddress("Reco_mu_isSoftCutBased", Reco_mu_isSoftCutBased);
    tree->SetBranchAddress("Reco_mu_softMvaRun3Value", Reco_mu_softMvaRun3Value);
    tree->SetBranchAddress("Reco_mu_isHybridSoft", Reco_mu_isHybridSoft);
    tree->SetBranchAddress("Reco_mu_isMediumCutBased", Reco_mu_isMediumCutBased);
    tree->SetBranchAddress("Reco_mu_isTightCutBased", Reco_mu_isTightCutBased);
    tree->SetBranchAddress("Reco_mu_nPixValHits", Reco_mu_nPixValHits);
    tree->SetBranchAddress("Reco_mu_nMuValHits", Reco_mu_nMuValHits);
    tree->SetBranchAddress("Reco_mu_nTrkHits", Reco_mu_nTrkHits);
    tree->SetBranchAddress("Reco_mu_normChi2_inner", Reco_mu_normChi2_inner);
    tree->SetBranchAddress("Reco_mu_nPixWMea", Reco_mu_nPixWMea);
    tree->SetBranchAddress("Reco_mu_nTrkWMea", Reco_mu_nTrkWMea);
    
    // Create output file and tree
    TFile* fout = new TFile(outputFile, "RECREATE");
    TTree* outTree = new TTree("dimuonTree", "Flattened dimuon tree");
    
    // Output branches - Event info
    UInt_t o_eventNb, o_runNb, o_LS;
    Float_t o_zVtx;
    Int_t o_Centrality;
    Short_t o_nPV;
    
    outTree->Branch("eventNb", &o_eventNb);
    outTree->Branch("runNb", &o_runNb);
    outTree->Branch("LS", &o_LS);
    outTree->Branch("zVtx", &o_zVtx);
    outTree->Branch("Centrality", &o_Centrality);
    outTree->Branch("nPV", &o_nPV);
    
    // Output branches - Dimuon
    Float_t QQ_mass, QQ_pt, QQ_eta, QQ_phi;
    Short_t QQ_type, QQ_sign;
    Float_t QQ_ctau, QQ_ctauErr, QQ_cosAlpha, QQ_VtxProb, QQ_dca;
    ULong64_t QQ_trigBits;
    
    outTree->Branch("QQ_mass", &QQ_mass);
    outTree->Branch("QQ_pt", &QQ_pt);
    outTree->Branch("QQ_eta", &QQ_eta);
    outTree->Branch("QQ_phi", &QQ_phi);
    outTree->Branch("QQ_type", &QQ_type);
    outTree->Branch("QQ_sign", &QQ_sign);
    outTree->Branch("QQ_ctau", &QQ_ctau);
    outTree->Branch("QQ_ctauErr", &QQ_ctauErr);
    outTree->Branch("QQ_cosAlpha", &QQ_cosAlpha);
    outTree->Branch("QQ_VtxProb", &QQ_VtxProb);
    outTree->Branch("QQ_dca", &QQ_dca);
    outTree->Branch("QQ_trigBits", &QQ_trigBits);
    
    // Output branches - Muon+
    Float_t mupl_pt, mupl_eta, mupl_phi, mupl_mass;
    Short_t mupl_charge, mupl_type;
    Bool_t mupl_InTightAcc, mupl_InLooseAcc, mupl_highPurity;
    Bool_t mupl_isTracker, mupl_isGlobal, mupl_isSoftCutBased;
    Float_t mupl_softMvaRun3Value;
    Bool_t mupl_isHybridSoft, mupl_isMediumCutBased, mupl_isTightCutBased;
    Int_t mupl_nPixValHits, mupl_nMuValHits, mupl_nTrkHits;
    Float_t mupl_normChi2_inner;
    ULong64_t mupl_trigBits;
    
    outTree->Branch("mupl_pt", &mupl_pt);
    outTree->Branch("mupl_eta", &mupl_eta);
    outTree->Branch("mupl_phi", &mupl_phi);
    outTree->Branch("mupl_mass", &mupl_mass);
    outTree->Branch("mupl_charge", &mupl_charge);
    outTree->Branch("mupl_type", &mupl_type);
    outTree->Branch("mupl_InTightAcc", &mupl_InTightAcc);
    outTree->Branch("mupl_InLooseAcc", &mupl_InLooseAcc);
    outTree->Branch("mupl_highPurity", &mupl_highPurity);
    outTree->Branch("mupl_isTracker", &mupl_isTracker);
    outTree->Branch("mupl_isGlobal", &mupl_isGlobal);
    outTree->Branch("mupl_isSoftCutBased", &mupl_isSoftCutBased);
    outTree->Branch("mupl_softMvaRun3Value", &mupl_softMvaRun3Value);
    outTree->Branch("mupl_isHybridSoft", &mupl_isHybridSoft);
    outTree->Branch("mupl_isMediumCutBased", &mupl_isMediumCutBased);
    outTree->Branch("mupl_isTightCutBased", &mupl_isTightCutBased);
    outTree->Branch("mupl_nPixValHits", &mupl_nPixValHits);
    outTree->Branch("mupl_nMuValHits", &mupl_nMuValHits);
    outTree->Branch("mupl_nTrkHits", &mupl_nTrkHits);
    outTree->Branch("mupl_normChi2_inner", &mupl_normChi2_inner);
    outTree->Branch("mupl_trigBits", &mupl_trigBits);
    
    // Output branches - Muon-
    Float_t mumi_pt, mumi_eta, mumi_phi, mumi_mass;
    Short_t mumi_charge, mumi_type;
    Bool_t mumi_InTightAcc, mumi_InLooseAcc, mumi_highPurity;
    Bool_t mumi_isTracker, mumi_isGlobal, mumi_isSoftCutBased;
    Float_t mumi_softMvaRun3Value;
    Bool_t mumi_isHybridSoft, mumi_isMediumCutBased, mumi_isTightCutBased;
    Int_t mumi_nPixValHits, mumi_nMuValHits, mumi_nTrkHits;
    Float_t mumi_normChi2_inner;
    ULong64_t mumi_trigBits;
    
    outTree->Branch("mumi_pt", &mumi_pt);
    outTree->Branch("mumi_eta", &mumi_eta);
    outTree->Branch("mumi_phi", &mumi_phi);
    outTree->Branch("mumi_mass", &mumi_mass);
    outTree->Branch("mumi_charge", &mumi_charge);
    outTree->Branch("mumi_type", &mumi_type);
    outTree->Branch("mumi_InTightAcc", &mumi_InTightAcc);
    outTree->Branch("mumi_InLooseAcc", &mumi_InLooseAcc);
    outTree->Branch("mumi_highPurity", &mumi_highPurity);
    outTree->Branch("mumi_isTracker", &mumi_isTracker);
    outTree->Branch("mumi_isGlobal", &mumi_isGlobal);
    outTree->Branch("mumi_isSoftCutBased", &mumi_isSoftCutBased);
    outTree->Branch("mumi_softMvaRun3Value", &mumi_softMvaRun3Value);
    outTree->Branch("mumi_isHybridSoft", &mumi_isHybridSoft);
    outTree->Branch("mumi_isMediumCutBased", &mumi_isMediumCutBased);
    outTree->Branch("mumi_isTightCutBased", &mumi_isTightCutBased);
    outTree->Branch("mumi_nPixValHits", &mumi_nPixValHits);
    outTree->Branch("mumi_nMuValHits", &mumi_nMuValHits);
    outTree->Branch("mumi_nTrkHits", &mumi_nTrkHits);
    outTree->Branch("mumi_normChi2_inner", &mumi_normChi2_inner);
    outTree->Branch("mumi_trigBits", &mumi_trigBits);
    
    // Loop over events
    Long64_t nEntries = tree->GetEntries();
    Long64_t nDimuons = 0;
    
    for (Long64_t i = 0; i < nEntries; i++) {
        if (i % 10000 == 0) {
            std::cout << "Processing event " << i << " / " << nEntries << std::endl;
        }
        
        tree->GetEntry(i);
        
        // Copy event info
        o_eventNb = eventNb;
        o_runNb = runNb;
        o_LS = LS;
        o_zVtx = zVtx;
        o_Centrality = Centrality;
        o_nPV = nPV;
        
        // Loop over dimuon candidates
        for (Short_t iQQ = 0; iQQ < Reco_QQ_size; iQQ++) {
            Short_t idx_pl = Reco_QQ_mupl_idx[iQQ];
            Short_t idx_mi = Reco_QQ_mumi_idx[iQQ];
            
            // Sanity check
            if (idx_pl < 0 || idx_pl >= Reco_mu_size || idx_mi < 0 || idx_mi >= Reco_mu_size) {
                std::cerr << "WARNING: Invalid muon index! Event " << i 
                          << " QQ " << iQQ << " mupl_idx=" << idx_pl << " mumi_idx=" << idx_mi 
                          << " mu_size=" << Reco_mu_size << std::endl;
                continue;
            }
            
            // Fill dimuon info
            QQ_mass = (*Reco_QQ_4mom_m)[iQQ];
            QQ_pt = (*Reco_QQ_4mom_pt)[iQQ];
            QQ_eta = (*Reco_QQ_4mom_eta)[iQQ];
            QQ_phi = (*Reco_QQ_4mom_phi)[iQQ];
            QQ_type = Reco_QQ_type[iQQ];
            QQ_sign = Reco_QQ_sign[iQQ];
            QQ_ctau = Reco_QQ_ctau[iQQ];
            QQ_ctauErr = Reco_QQ_ctauErr[iQQ];
            QQ_cosAlpha = Reco_QQ_cosAlpha[iQQ];
            QQ_VtxProb = Reco_QQ_VtxProb[iQQ];
            QQ_dca = Reco_QQ_dca[iQQ];
            QQ_trigBits = Reco_QQ_trig[iQQ];
            
            // Fill muon+ info
            mupl_pt = (*Reco_mu_4mom_pt)[idx_pl];
            mupl_eta = (*Reco_mu_4mom_eta)[idx_pl];
            mupl_phi = (*Reco_mu_4mom_phi)[idx_pl];
            mupl_mass = (*Reco_mu_4mom_m)[idx_pl];
            mupl_charge = Reco_mu_charge[idx_pl];
            mupl_type = Reco_mu_type[idx_pl];
            mupl_InTightAcc = Reco_mu_InTightAcc[idx_pl];
            mupl_InLooseAcc = Reco_mu_InLooseAcc[idx_pl];
            mupl_highPurity = Reco_mu_highPurity[idx_pl];
            mupl_isTracker = Reco_mu_isTracker[idx_pl];
            mupl_isGlobal = Reco_mu_isGlobal[idx_pl];
            mupl_isSoftCutBased = Reco_mu_isSoftCutBased[idx_pl];
            mupl_softMvaRun3Value = Reco_mu_softMvaRun3Value[idx_pl];
            mupl_isHybridSoft = Reco_mu_isHybridSoft[idx_pl];
            mupl_isMediumCutBased = Reco_mu_isMediumCutBased[idx_pl];
            mupl_isTightCutBased = Reco_mu_isTightCutBased[idx_pl];
            mupl_nPixValHits = Reco_mu_nPixValHits[idx_pl];
            mupl_nMuValHits = Reco_mu_nMuValHits[idx_pl];
            mupl_nTrkHits = Reco_mu_nTrkHits[idx_pl];
            mupl_normChi2_inner = Reco_mu_normChi2_inner[idx_pl];
            mupl_trigBits = Reco_mu_trig[idx_pl];
            
            // Fill muon- info
            mumi_pt = (*Reco_mu_4mom_pt)[idx_mi];
            mumi_eta = (*Reco_mu_4mom_eta)[idx_mi];
            mumi_phi = (*Reco_mu_4mom_phi)[idx_mi];
            mumi_mass = (*Reco_mu_4mom_m)[idx_mi];
            mumi_charge = Reco_mu_charge[idx_mi];
            mumi_type = Reco_mu_type[idx_mi];
            mumi_InTightAcc = Reco_mu_InTightAcc[idx_mi];
            mumi_InLooseAcc = Reco_mu_InLooseAcc[idx_mi];
            mumi_highPurity = Reco_mu_highPurity[idx_mi];
            mumi_isTracker = Reco_mu_isTracker[idx_mi];
            mumi_isGlobal = Reco_mu_isGlobal[idx_mi];
            mumi_isSoftCutBased = Reco_mu_isSoftCutBased[idx_mi];
            mumi_softMvaRun3Value = Reco_mu_softMvaRun3Value[idx_mi];
            mumi_isHybridSoft = Reco_mu_isHybridSoft[idx_mi];
            mumi_isMediumCutBased = Reco_mu_isMediumCutBased[idx_mi];
            mumi_isTightCutBased = Reco_mu_isTightCutBased[idx_mi];
            mumi_nPixValHits = Reco_mu_nPixValHits[idx_mi];
            mumi_nMuValHits = Reco_mu_nMuValHits[idx_mi];
            mumi_nTrkHits = Reco_mu_nTrkHits[idx_mi];
            mumi_normChi2_inner = Reco_mu_normChi2_inner[idx_mi];
            mumi_trigBits = Reco_mu_trig[idx_mi];
            
            outTree->Fill();
            nDimuons++;
        }
    }
    
    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "Input events: " << nEntries << std::endl;
    std::cout << "Output dimuons: " << nDimuons << std::endl;
    std::cout << "Average dimuons/event: " << (float)nDimuons/nEntries << std::endl;
    
    fout->cd();
    outTree->Write();
    fout->Close();
    fin->Close();
    
    std::cout << "\nFlattened tree written to: " << outputFile << std::endl;
    std::cout << "Usage example:" << std::endl;
    std::cout << "  root -l " << outputFile << std::endl;
    std::cout << "  dimuonTree->Draw(\"QQ_mass\", \"QQ_sign==0 && QQ_VtxProb>0.01\")" << std::endl;
}
