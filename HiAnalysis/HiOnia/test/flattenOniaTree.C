// Flatten OniaTree: Create flat trees for dimuon and dielectron analysis
// Usage: root -l -b -q 'flattenOniaTree.C("input.root", "output.root")'
//
// Output trees:
//   - DimuonTree: flat tree with mu+/mu- daughter info
//   - DielectronTree: flat tree with e+/e- daughter info

#include <TFile.h>
#include <TTree.h>
#include <TLorentzVector.h>
#include <TClonesArray.h>
#include <iostream>
#include <vector>

void flattenOniaTree(const char* inputFile = "OniaTree_Combined_PbPb2025_DATA.root",
                     const char* outputFile = "FlatOniaTree.root") {
    
    // Open input file
    TFile* fIn = TFile::Open(inputFile, "READ");
    if (!fIn || fIn->IsZombie()) {
        std::cerr << "Error: Cannot open input file!" << std::endl;
        return;
    }
    
    // Try to get the combined tree first, then legacy
    TTree* tree = nullptr;
    tree = (TTree*)fIn->Get("hioniaCombined/OniaTree");
    if (!tree) {
        tree = (TTree*)fIn->Get("hionia/myTree");
    }
    if (!tree) {
        std::cerr << "Error: Cannot find OniaTree!" << std::endl;
        fIn->Close();
        return;
    }
    
    // Create output file
    TFile* fOut = new TFile(outputFile, "RECREATE");
    
    // Dimuon flat tree
    TTree* muTree = new TTree("DimuonTree", "Flat dimuon tree");
    
    // Event info
    UInt_t mu_runNb, mu_eventNb, mu_LS;
    Int_t mu_Centrality;
    Float_t mu_zVtx;
    ULong64_t mu_HLTriggers;
    
    // Dimuon variables
    Float_t mu_mass, mu_pt, mu_eta, mu_phi, mu_y;
    Float_t mu_VtxProb, mu_ctau, mu_ctauErr, mu_cosAlpha;
    Int_t mu_charge;
    ULong64_t mu_trig;
    
    // mu+ (positive muon) variables
    Float_t mupl_pt, mupl_eta, mupl_phi;
    Int_t mupl_charge;
    Int_t mupl_nTrkHits, mupl_nPixWMea, mupl_nTrkWMea;
    Float_t mupl_dxy, mupl_dz;
    Bool_t mupl_isGlobal, mupl_isTracker, mupl_isSoft, mupl_isPF;
    
    // mu- (negative muon) variables
    Float_t mumi_pt, mumi_eta, mumi_phi;
    Int_t mumi_charge;
    Int_t mumi_nTrkHits, mumi_nPixWMea, mumi_nTrkWMea;
    Float_t mumi_dxy, mumi_dz;
    Bool_t mumi_isGlobal, mumi_isTracker, mumi_isSoft, mumi_isPF;
    
    // Set branches for dimuon tree
    muTree->Branch("runNb", &mu_runNb);
    muTree->Branch("eventNb", &mu_eventNb);
    muTree->Branch("LS", &mu_LS);
    muTree->Branch("Centrality", &mu_Centrality);
    muTree->Branch("zVtx", &mu_zVtx);
    muTree->Branch("HLTriggers", &mu_HLTriggers);
    
    muTree->Branch("mass", &mu_mass);
    muTree->Branch("pt", &mu_pt);
    muTree->Branch("eta", &mu_eta);
    muTree->Branch("phi", &mu_phi);
    muTree->Branch("y", &mu_y);
    muTree->Branch("VtxProb", &mu_VtxProb);
    muTree->Branch("ctau", &mu_ctau);
    muTree->Branch("ctauErr", &mu_ctauErr);
    muTree->Branch("cosAlpha", &mu_cosAlpha);
    muTree->Branch("charge", &mu_charge);
    muTree->Branch("trig", &mu_trig);
    
    muTree->Branch("mupl_pt", &mupl_pt);
    muTree->Branch("mupl_eta", &mupl_eta);
    muTree->Branch("mupl_phi", &mupl_phi);
    muTree->Branch("mupl_charge", &mupl_charge);
    muTree->Branch("mupl_nTrkHits", &mupl_nTrkHits);
    muTree->Branch("mupl_nPixWMea", &mupl_nPixWMea);
    muTree->Branch("mupl_nTrkWMea", &mupl_nTrkWMea);
    muTree->Branch("mupl_dxy", &mupl_dxy);
    muTree->Branch("mupl_dz", &mupl_dz);
    muTree->Branch("mupl_isGlobal", &mupl_isGlobal);
    muTree->Branch("mupl_isTracker", &mupl_isTracker);
    muTree->Branch("mupl_isSoft", &mupl_isSoft);
    muTree->Branch("mupl_isPF", &mupl_isPF);
    
    muTree->Branch("mumi_pt", &mumi_pt);
    muTree->Branch("mumi_eta", &mumi_eta);
    muTree->Branch("mumi_phi", &mumi_phi);
    muTree->Branch("mumi_charge", &mumi_charge);
    muTree->Branch("mumi_nTrkHits", &mumi_nTrkHits);
    muTree->Branch("mumi_nPixWMea", &mumi_nPixWMea);
    muTree->Branch("mumi_nTrkWMea", &mumi_nTrkWMea);
    muTree->Branch("mumi_dxy", &mumi_dxy);
    muTree->Branch("mumi_dz", &mumi_dz);
    muTree->Branch("mumi_isGlobal", &mumi_isGlobal);
    muTree->Branch("mumi_isTracker", &mumi_isTracker);
    muTree->Branch("mumi_isSoft", &mumi_isSoft);
    muTree->Branch("mumi_isPF", &mumi_isPF);
    
    // Dielectron flat tree
    TTree* eleTree = new TTree("DielectronTree", "Flat dielectron tree");
    
    // Event info
    UInt_t ele_runNb, ele_eventNb, ele_LS;
    Int_t ele_Centrality;
    Float_t ele_zVtx;
    ULong64_t ele_HLTriggers;
    
    // Dielectron variables
    Float_t ee_mass, ee_pt, ee_eta, ee_phi, ee_y;
    Float_t ee_VtxProb;
    Int_t ee_charge;
    ULong64_t ee_trig;
    
    // e+ (positron) variables
    Float_t ep_pt, ep_eta, ep_phi, ep_y;
    Int_t ep_charge;
    Float_t ep_dxy, ep_dz;
    Float_t ep_sigmaIEtaIEta, ep_hOverE, ep_eOverP;
    Float_t ep_deltaEtaIn, ep_deltaPhiIn;
    Float_t ep_r9, ep_fbrem;
    Float_t ep_pfChIso, ep_pfNeuIso, ep_pfPhoIso;
    Short_t ep_convVeto, ep_nMissingHits;
    Float_t ep_scEta, ep_scPhi, ep_scEn;
    
    // e- (electron) variables
    Float_t em_pt, em_eta, em_phi, em_y;
    Int_t em_charge;
    Float_t em_dxy, em_dz;
    Float_t em_sigmaIEtaIEta, em_hOverE, em_eOverP;
    Float_t em_deltaEtaIn, em_deltaPhiIn;
    Float_t em_r9, em_fbrem;
    Float_t em_pfChIso, em_pfNeuIso, em_pfPhoIso;
    Short_t em_convVeto, em_nMissingHits;
    Float_t em_scEta, em_scPhi, em_scEn;
    
    // Set branches for dielectron tree
    eleTree->Branch("runNb", &ele_runNb);
    eleTree->Branch("eventNb", &ele_eventNb);
    eleTree->Branch("LS", &ele_LS);
    eleTree->Branch("Centrality", &ele_Centrality);
    eleTree->Branch("zVtx", &ele_zVtx);
    eleTree->Branch("HLTriggers", &ele_HLTriggers);
    
    eleTree->Branch("mass", &ee_mass);
    eleTree->Branch("pt", &ee_pt);
    eleTree->Branch("eta", &ee_eta);
    eleTree->Branch("phi", &ee_phi);
    eleTree->Branch("y", &ee_y);
    eleTree->Branch("VtxProb", &ee_VtxProb);
    eleTree->Branch("charge", &ee_charge);
    eleTree->Branch("trig", &ee_trig);
    
    eleTree->Branch("ep_pt", &ep_pt);
    eleTree->Branch("ep_eta", &ep_eta);
    eleTree->Branch("ep_phi", &ep_phi);
    eleTree->Branch("ep_y", &ep_y);
    eleTree->Branch("ep_charge", &ep_charge);
    eleTree->Branch("ep_dxy", &ep_dxy);
    eleTree->Branch("ep_dz", &ep_dz);
    eleTree->Branch("ep_sigmaIEtaIEta", &ep_sigmaIEtaIEta);
    eleTree->Branch("ep_hOverE", &ep_hOverE);
    eleTree->Branch("ep_eOverP", &ep_eOverP);
    eleTree->Branch("ep_deltaEtaIn", &ep_deltaEtaIn);
    eleTree->Branch("ep_deltaPhiIn", &ep_deltaPhiIn);
    eleTree->Branch("ep_r9", &ep_r9);
    eleTree->Branch("ep_fbrem", &ep_fbrem);
    eleTree->Branch("ep_pfChIso", &ep_pfChIso);
    eleTree->Branch("ep_pfNeuIso", &ep_pfNeuIso);
    eleTree->Branch("ep_pfPhoIso", &ep_pfPhoIso);
    eleTree->Branch("ep_convVeto", &ep_convVeto);
    eleTree->Branch("ep_nMissingHits", &ep_nMissingHits);
    eleTree->Branch("ep_scEta", &ep_scEta);
    eleTree->Branch("ep_scPhi", &ep_scPhi);
    eleTree->Branch("ep_scEn", &ep_scEn);
    
    eleTree->Branch("em_pt", &em_pt);
    eleTree->Branch("em_eta", &em_eta);
    eleTree->Branch("em_phi", &em_phi);
    eleTree->Branch("em_y", &em_y);
    eleTree->Branch("em_charge", &em_charge);
    eleTree->Branch("em_dxy", &em_dxy);
    eleTree->Branch("em_dz", &em_dz);
    eleTree->Branch("em_sigmaIEtaIEta", &em_sigmaIEtaIEta);
    eleTree->Branch("em_hOverE", &em_hOverE);
    eleTree->Branch("em_eOverP", &em_eOverP);
    eleTree->Branch("em_deltaEtaIn", &em_deltaEtaIn);
    eleTree->Branch("em_deltaPhiIn", &em_deltaPhiIn);
    eleTree->Branch("em_r9", &em_r9);
    eleTree->Branch("em_fbrem", &em_fbrem);
    eleTree->Branch("em_pfChIso", &em_pfChIso);
    eleTree->Branch("em_pfNeuIso", &em_pfNeuIso);
    eleTree->Branch("em_pfPhoIso", &em_pfPhoIso);
    eleTree->Branch("em_convVeto", &em_convVeto);
    eleTree->Branch("em_nMissingHits", &em_nMissingHits);
    eleTree->Branch("em_scEta", &em_scEta);
    eleTree->Branch("em_scPhi", &em_scPhi);
    eleTree->Branch("em_scEn", &em_scEn);
    
    // ========================================
    // Input branch variables
    // ========================================
    UInt_t in_runNb, in_LS;
    ULong64_t in_eventNb;
    Int_t in_Centrality;
    Float_t in_zVtx;
    ULong64_t in_HLTriggers;
    
    // Dimuon input
    Short_t Reco_QQ_size = 0;
    Short_t Reco_mu_size = 0;
    TClonesArray* Reco_QQ_4mom = nullptr;
    TClonesArray* Reco_mu_4mom = nullptr;
    std::vector<float>* Reco_QQ_4mom_pt = nullptr;
    std::vector<float>* Reco_QQ_4mom_eta = nullptr;
    std::vector<float>* Reco_QQ_4mom_phi = nullptr;
    std::vector<float>* Reco_QQ_4mom_m = nullptr;
    std::vector<float>* Reco_mu_4mom_pt = nullptr;
    std::vector<float>* Reco_mu_4mom_eta = nullptr;
    std::vector<float>* Reco_mu_4mom_phi = nullptr;
    std::vector<float>* Reco_mu_4mom_m = nullptr;
    
    Short_t Reco_QQ_mupl_idx[100], Reco_QQ_mumi_idx[100];
    Short_t Reco_QQ_sign[100];
    Float_t Reco_QQ_VtxProb[100], Reco_QQ_ctau[100], Reco_QQ_ctauErr[100], Reco_QQ_cosAlpha[100];
    ULong64_t Reco_QQ_trig[100];
    
    Short_t Reco_mu_charge[100];
    Int_t Reco_mu_nTrkHits[100], Reco_mu_nPixWMea[100], Reco_mu_nTrkWMea[100];
    Float_t Reco_mu_dxy[100], Reco_mu_dz[100];
    Bool_t Reco_mu_isGlobal[100], Reco_mu_isTracker[100], Reco_mu_isSoft[100], Reco_mu_isPF[100];
    
    // Dielectron input
    Int_t Reco_ee_size = 0;
    Int_t Reco_ele_size = 0;
    std::vector<float>* Reco_ee_pt = nullptr;
    std::vector<float>* Reco_ee_eta = nullptr;
    std::vector<float>* Reco_ee_phi = nullptr;
    std::vector<float>* Reco_ee_y = nullptr;
    std::vector<float>* Reco_ee_mass = nullptr;
    std::vector<float>* Reco_ele_pt = nullptr;
    std::vector<float>* Reco_ele_eta = nullptr;
    std::vector<float>* Reco_ele_phi = nullptr;
    std::vector<float>* Reco_ele_y = nullptr;
    
    Short_t Reco_ee_ele1Idx[100], Reco_ee_ele2Idx[100];
    Short_t Reco_ee_charge[100];
    Float_t Reco_ee_vProb[100];
    ULong64_t Reco_ee_trigBits[100];
    
    Short_t Reco_ele_charge[100];
    Float_t Reco_ele_dxy[100], Reco_ele_dz[100];
    Float_t Reco_ele_sigmaIEtaIEta[100], Reco_ele_hOverE[100], Reco_ele_eOverP[100];
    Float_t Reco_ele_deltaEtaIn[100], Reco_ele_deltaPhiIn[100];
    Float_t Reco_ele_r9[100], Reco_ele_fbrem[100];
    Float_t Reco_ele_pfChIso[100], Reco_ele_pfNeuIso[100], Reco_ele_pfPhoIso[100];
    Short_t Reco_ele_convVeto[100], Reco_ele_nMissingHits[100];
    Float_t Reco_ele_scEta[100], Reco_ele_scPhi[100], Reco_ele_scEn[100];
    
    // Set input branch addresses - Event info
    tree->SetBranchAddress("runNb", &in_runNb);
    tree->SetBranchAddress("eventNb", &in_eventNb);
    tree->SetBranchAddress("LS", &in_LS);
    tree->SetBranchAddress("Centrality", &in_Centrality);
    tree->SetBranchAddress("zVtx", &in_zVtx);
    tree->SetBranchAddress("HLTriggers", &in_HLTriggers);
    
    // Dimuon branches
    bool hasMuons = (tree->GetBranch("Reco_QQ_size") != nullptr);
    bool useVectorMom = (tree->GetBranch("Reco_QQ_4mom_pt") != nullptr);
    
    if (hasMuons) {
        tree->SetBranchAddress("Reco_QQ_size", &Reco_QQ_size);
        tree->SetBranchAddress("Reco_mu_size", &Reco_mu_size);
        
        if (useVectorMom) {
            tree->SetBranchAddress("Reco_QQ_4mom_pt", &Reco_QQ_4mom_pt);
            tree->SetBranchAddress("Reco_QQ_4mom_eta", &Reco_QQ_4mom_eta);
            tree->SetBranchAddress("Reco_QQ_4mom_phi", &Reco_QQ_4mom_phi);
            tree->SetBranchAddress("Reco_QQ_4mom_m", &Reco_QQ_4mom_m);
            tree->SetBranchAddress("Reco_mu_4mom_pt", &Reco_mu_4mom_pt);
            tree->SetBranchAddress("Reco_mu_4mom_eta", &Reco_mu_4mom_eta);
            tree->SetBranchAddress("Reco_mu_4mom_phi", &Reco_mu_4mom_phi);
            tree->SetBranchAddress("Reco_mu_4mom_m", &Reco_mu_4mom_m);
        } else {
            Reco_QQ_4mom = new TClonesArray("TLorentzVector", 100);
            Reco_mu_4mom = new TClonesArray("TLorentzVector", 100);
            tree->SetBranchAddress("Reco_QQ_4mom", &Reco_QQ_4mom);
            tree->SetBranchAddress("Reco_mu_4mom", &Reco_mu_4mom);
        }
        
        tree->SetBranchAddress("Reco_QQ_mupl_idx", Reco_QQ_mupl_idx);
        tree->SetBranchAddress("Reco_QQ_mumi_idx", Reco_QQ_mumi_idx);
        tree->SetBranchAddress("Reco_QQ_sign", Reco_QQ_sign);
        tree->SetBranchAddress("Reco_QQ_VtxProb", Reco_QQ_VtxProb);
        tree->SetBranchAddress("Reco_QQ_ctau", Reco_QQ_ctau);
        tree->SetBranchAddress("Reco_QQ_ctauErr", Reco_QQ_ctauErr);
        if (tree->GetBranch("Reco_QQ_cosAlpha")) tree->SetBranchAddress("Reco_QQ_cosAlpha", Reco_QQ_cosAlpha);
        tree->SetBranchAddress("Reco_QQ_trig", Reco_QQ_trig);
        
        tree->SetBranchAddress("Reco_mu_charge", Reco_mu_charge);
        if (tree->GetBranch("Reco_mu_nTrkHits")) tree->SetBranchAddress("Reco_mu_nTrkHits", Reco_mu_nTrkHits);
        if (tree->GetBranch("Reco_mu_nPixWMea")) tree->SetBranchAddress("Reco_mu_nPixWMea", Reco_mu_nPixWMea);
        if (tree->GetBranch("Reco_mu_nTrkWMea")) tree->SetBranchAddress("Reco_mu_nTrkWMea", Reco_mu_nTrkWMea);
        if (tree->GetBranch("Reco_mu_dxy")) tree->SetBranchAddress("Reco_mu_dxy", Reco_mu_dxy);
        if (tree->GetBranch("Reco_mu_dz")) tree->SetBranchAddress("Reco_mu_dz", Reco_mu_dz);
        if (tree->GetBranch("Reco_mu_isGlobal")) tree->SetBranchAddress("Reco_mu_isGlobal", Reco_mu_isGlobal);
        if (tree->GetBranch("Reco_mu_isTracker")) tree->SetBranchAddress("Reco_mu_isTracker", Reco_mu_isTracker);
        if (tree->GetBranch("Reco_mu_isSoft")) tree->SetBranchAddress("Reco_mu_isSoft", Reco_mu_isSoft);
        if (tree->GetBranch("Reco_mu_isPF")) tree->SetBranchAddress("Reco_mu_isPF", Reco_mu_isPF);
    }
    
    // Dielectron branches
    bool hasElectrons = (tree->GetBranch("Reco_ee_size") != nullptr);
    
    if (hasElectrons) {
        tree->SetBranchAddress("Reco_ee_size", &Reco_ee_size);
        tree->SetBranchAddress("Reco_ele_size", &Reco_ele_size);
        tree->SetBranchAddress("Reco_ee_pt", &Reco_ee_pt);
        tree->SetBranchAddress("Reco_ee_eta", &Reco_ee_eta);
        tree->SetBranchAddress("Reco_ee_phi", &Reco_ee_phi);
        tree->SetBranchAddress("Reco_ee_y", &Reco_ee_y);
        tree->SetBranchAddress("Reco_ee_mass", &Reco_ee_mass);
        tree->SetBranchAddress("Reco_ee_ele1Idx", Reco_ee_ele1Idx);
        tree->SetBranchAddress("Reco_ee_ele2Idx", Reco_ee_ele2Idx);
        tree->SetBranchAddress("Reco_ee_charge", Reco_ee_charge);
        if (tree->GetBranch("Reco_ee_vProb")) tree->SetBranchAddress("Reco_ee_vProb", Reco_ee_vProb);
        if (tree->GetBranch("Reco_ee_trigBits")) tree->SetBranchAddress("Reco_ee_trigBits", Reco_ee_trigBits);
        
        tree->SetBranchAddress("Reco_ele_pt", &Reco_ele_pt);
        tree->SetBranchAddress("Reco_ele_eta", &Reco_ele_eta);
        tree->SetBranchAddress("Reco_ele_phi", &Reco_ele_phi);
        tree->SetBranchAddress("Reco_ele_y", &Reco_ele_y);
        tree->SetBranchAddress("Reco_ele_charge", Reco_ele_charge);
        if (tree->GetBranch("Reco_ele_dxy")) tree->SetBranchAddress("Reco_ele_dxy", Reco_ele_dxy);
        if (tree->GetBranch("Reco_ele_dz")) tree->SetBranchAddress("Reco_ele_dz", Reco_ele_dz);
        if (tree->GetBranch("Reco_ele_sigmaIEtaIEta")) tree->SetBranchAddress("Reco_ele_sigmaIEtaIEta", Reco_ele_sigmaIEtaIEta);
        if (tree->GetBranch("Reco_ele_hOverE")) tree->SetBranchAddress("Reco_ele_hOverE", Reco_ele_hOverE);
        if (tree->GetBranch("Reco_ele_eOverP")) tree->SetBranchAddress("Reco_ele_eOverP", Reco_ele_eOverP);
        if (tree->GetBranch("Reco_ele_deltaEtaIn")) tree->SetBranchAddress("Reco_ele_deltaEtaIn", Reco_ele_deltaEtaIn);
        if (tree->GetBranch("Reco_ele_deltaPhiIn")) tree->SetBranchAddress("Reco_ele_deltaPhiIn", Reco_ele_deltaPhiIn);
        if (tree->GetBranch("Reco_ele_r9")) tree->SetBranchAddress("Reco_ele_r9", Reco_ele_r9);
        if (tree->GetBranch("Reco_ele_fbrem")) tree->SetBranchAddress("Reco_ele_fbrem", Reco_ele_fbrem);
        if (tree->GetBranch("Reco_ele_pfChIso")) tree->SetBranchAddress("Reco_ele_pfChIso", Reco_ele_pfChIso);
        if (tree->GetBranch("Reco_ele_pfNeuIso")) tree->SetBranchAddress("Reco_ele_pfNeuIso", Reco_ele_pfNeuIso);
        if (tree->GetBranch("Reco_ele_pfPhoIso")) tree->SetBranchAddress("Reco_ele_pfPhoIso", Reco_ele_pfPhoIso);
        if (tree->GetBranch("Reco_ele_convVeto")) tree->SetBranchAddress("Reco_ele_convVeto", Reco_ele_convVeto);
        if (tree->GetBranch("Reco_ele_nMissingHits")) tree->SetBranchAddress("Reco_ele_nMissingHits", Reco_ele_nMissingHits);
        if (tree->GetBranch("Reco_ele_scEta")) tree->SetBranchAddress("Reco_ele_scEta", Reco_ele_scEta);
        if (tree->GetBranch("Reco_ele_scPhi")) tree->SetBranchAddress("Reco_ele_scPhi", Reco_ele_scPhi);
        if (tree->GetBranch("Reco_ele_scEn")) tree->SetBranchAddress("Reco_ele_scEn", Reco_ele_scEn);
    }
    
    
    // Event loop
    Long64_t nEntries = tree->GetEntries();
    Long64_t nDimuons = 0, nDielectrons = 0;
    
    for (Long64_t iEntry = 0; iEntry < nEntries; iEntry++) {
        tree->GetEntry(iEntry);
        
        // Process dimuons
        if (hasMuons) {
            for (int iQQ = 0; iQQ < Reco_QQ_size; iQQ++) {
                int mupl_idx = Reco_QQ_mupl_idx[iQQ];
                int mumi_idx = Reco_QQ_mumi_idx[iQQ];
                
                if (mupl_idx < 0 || mumi_idx < 0) continue;
                if (mupl_idx >= Reco_mu_size || mumi_idx >= Reco_mu_size) continue;
                
                // Event info
                mu_runNb = in_runNb;
                mu_eventNb = in_eventNb;
                mu_LS = in_LS;
                mu_Centrality = in_Centrality;
                mu_zVtx = in_zVtx;
                mu_HLTriggers = in_HLTriggers;
                
                // Dimuon info
                if (useVectorMom) {
                    mu_pt = (*Reco_QQ_4mom_pt)[iQQ];
                    mu_eta = (*Reco_QQ_4mom_eta)[iQQ];
                    mu_phi = (*Reco_QQ_4mom_phi)[iQQ];
                    mu_mass = (*Reco_QQ_4mom_m)[iQQ];
                    TLorentzVector v; v.SetPtEtaPhiM(mu_pt, mu_eta, mu_phi, mu_mass);
                    mu_y = v.Rapidity();
                } else {
                    TLorentzVector* v = (TLorentzVector*)Reco_QQ_4mom->At(iQQ);
                    mu_pt = v->Pt();
                    mu_eta = v->Eta();
                    mu_phi = v->Phi();
                    mu_mass = v->M();
                    mu_y = v->Rapidity();
                }
                
                mu_charge = Reco_QQ_sign[iQQ];
                mu_VtxProb = Reco_QQ_VtxProb[iQQ];
                mu_ctau = Reco_QQ_ctau[iQQ];
                mu_ctauErr = Reco_QQ_ctauErr[iQQ];
                mu_cosAlpha = tree->GetBranch("Reco_QQ_cosAlpha") ? Reco_QQ_cosAlpha[iQQ] : -999;
                mu_trig = Reco_QQ_trig[iQQ];
                
                // mu+ info
                if (useVectorMom) {
                    mupl_pt = (*Reco_mu_4mom_pt)[mupl_idx];
                    mupl_eta = (*Reco_mu_4mom_eta)[mupl_idx];
                    mupl_phi = (*Reco_mu_4mom_phi)[mupl_idx];
                } else {
                    TLorentzVector* vpl = (TLorentzVector*)Reco_mu_4mom->At(mupl_idx);
                    mupl_pt = vpl->Pt();
                    mupl_eta = vpl->Eta();
                    mupl_phi = vpl->Phi();
                }
                mupl_charge = Reco_mu_charge[mupl_idx];
                mupl_nTrkHits = tree->GetBranch("Reco_mu_nTrkHits") ? Reco_mu_nTrkHits[mupl_idx] : -1;
                mupl_nPixWMea = tree->GetBranch("Reco_mu_nPixWMea") ? Reco_mu_nPixWMea[mupl_idx] : -1;
                mupl_nTrkWMea = tree->GetBranch("Reco_mu_nTrkWMea") ? Reco_mu_nTrkWMea[mupl_idx] : -1;
                mupl_dxy = tree->GetBranch("Reco_mu_dxy") ? Reco_mu_dxy[mupl_idx] : -999;
                mupl_dz = tree->GetBranch("Reco_mu_dz") ? Reco_mu_dz[mupl_idx] : -999;
                mupl_isGlobal = tree->GetBranch("Reco_mu_isGlobal") ? Reco_mu_isGlobal[mupl_idx] : false;
                mupl_isTracker = tree->GetBranch("Reco_mu_isTracker") ? Reco_mu_isTracker[mupl_idx] : false;
                mupl_isSoft = tree->GetBranch("Reco_mu_isSoft") ? Reco_mu_isSoft[mupl_idx] : false;
                mupl_isPF = tree->GetBranch("Reco_mu_isPF") ? Reco_mu_isPF[mupl_idx] : false;
                
                // mu- info
                if (useVectorMom) {
                    mumi_pt = (*Reco_mu_4mom_pt)[mumi_idx];
                    mumi_eta = (*Reco_mu_4mom_eta)[mumi_idx];
                    mumi_phi = (*Reco_mu_4mom_phi)[mumi_idx];
                } else {
                    TLorentzVector* vmi = (TLorentzVector*)Reco_mu_4mom->At(mumi_idx);
                    mumi_pt = vmi->Pt();
                    mumi_eta = vmi->Eta();
                    mumi_phi = vmi->Phi();
                }
                mumi_charge = Reco_mu_charge[mumi_idx];
                mumi_nTrkHits = tree->GetBranch("Reco_mu_nTrkHits") ? Reco_mu_nTrkHits[mumi_idx] : -1;
                mumi_nPixWMea = tree->GetBranch("Reco_mu_nPixWMea") ? Reco_mu_nPixWMea[mumi_idx] : -1;
                mumi_nTrkWMea = tree->GetBranch("Reco_mu_nTrkWMea") ? Reco_mu_nTrkWMea[mumi_idx] : -1;
                mumi_dxy = tree->GetBranch("Reco_mu_dxy") ? Reco_mu_dxy[mumi_idx] : -999;
                mumi_dz = tree->GetBranch("Reco_mu_dz") ? Reco_mu_dz[mumi_idx] : -999;
                mumi_isGlobal = tree->GetBranch("Reco_mu_isGlobal") ? Reco_mu_isGlobal[mumi_idx] : false;
                mumi_isTracker = tree->GetBranch("Reco_mu_isTracker") ? Reco_mu_isTracker[mumi_idx] : false;
                mumi_isSoft = tree->GetBranch("Reco_mu_isSoft") ? Reco_mu_isSoft[mumi_idx] : false;
                mumi_isPF = tree->GetBranch("Reco_mu_isPF") ? Reco_mu_isPF[mumi_idx] : false;
                
                muTree->Fill();
                nDimuons++;
            }
        }
        
        // Process dielectrons
        if (hasElectrons) {
            for (int iEE = 0; iEE < Reco_ee_size; iEE++) {
                int ele1_idx = Reco_ee_ele1Idx[iEE];
                int ele2_idx = Reco_ee_ele2Idx[iEE];
                
                if (ele1_idx < 0 || ele2_idx < 0) continue;
                if (ele1_idx >= Reco_ele_size || ele2_idx >= Reco_ele_size) continue;
                
                // Determine which is e+ and e-
                int ep_idx, em_idx;
                if (Reco_ele_charge[ele1_idx] > 0) {
                    ep_idx = ele1_idx;
                    em_idx = ele2_idx;
                } else {
                    ep_idx = ele2_idx;
                    em_idx = ele1_idx;
                }
                
                // Event info
                ele_runNb = in_runNb;
                ele_eventNb = in_eventNb;
                ele_LS = in_LS;
                ele_Centrality = in_Centrality;
                ele_zVtx = in_zVtx;
                ele_HLTriggers = in_HLTriggers;
                
                // Dielectron info
                ee_pt = (*Reco_ee_pt)[iEE];
                ee_eta = (*Reco_ee_eta)[iEE];
                ee_phi = (*Reco_ee_phi)[iEE];
                ee_y = (*Reco_ee_y)[iEE];
                ee_mass = (*Reco_ee_mass)[iEE];
                ee_charge = Reco_ee_charge[iEE];
                ee_VtxProb = tree->GetBranch("Reco_ee_vProb") ? Reco_ee_vProb[iEE] : -999;
                ee_trig = tree->GetBranch("Reco_ee_trigBits") ? Reco_ee_trigBits[iEE] : 0;
                
                // e+ (positron) info
                ep_pt = (*Reco_ele_pt)[ep_idx];
                ep_eta = (*Reco_ele_eta)[ep_idx];
                ep_phi = (*Reco_ele_phi)[ep_idx];
                ep_y = (*Reco_ele_y)[ep_idx];
                ep_charge = Reco_ele_charge[ep_idx];
                ep_dxy = tree->GetBranch("Reco_ele_dxy") ? Reco_ele_dxy[ep_idx] : -999;
                ep_dz = tree->GetBranch("Reco_ele_dz") ? Reco_ele_dz[ep_idx] : -999;
                ep_sigmaIEtaIEta = tree->GetBranch("Reco_ele_sigmaIEtaIEta") ? Reco_ele_sigmaIEtaIEta[ep_idx] : -999;
                ep_hOverE = tree->GetBranch("Reco_ele_hOverE") ? Reco_ele_hOverE[ep_idx] : -999;
                ep_eOverP = tree->GetBranch("Reco_ele_eOverP") ? Reco_ele_eOverP[ep_idx] : -999;
                ep_deltaEtaIn = tree->GetBranch("Reco_ele_deltaEtaIn") ? Reco_ele_deltaEtaIn[ep_idx] : -999;
                ep_deltaPhiIn = tree->GetBranch("Reco_ele_deltaPhiIn") ? Reco_ele_deltaPhiIn[ep_idx] : -999;
                ep_r9 = tree->GetBranch("Reco_ele_r9") ? Reco_ele_r9[ep_idx] : -999;
                ep_fbrem = tree->GetBranch("Reco_ele_fbrem") ? Reco_ele_fbrem[ep_idx] : -999;
                ep_pfChIso = tree->GetBranch("Reco_ele_pfChIso") ? Reco_ele_pfChIso[ep_idx] : -999;
                ep_pfNeuIso = tree->GetBranch("Reco_ele_pfNeuIso") ? Reco_ele_pfNeuIso[ep_idx] : -999;
                ep_pfPhoIso = tree->GetBranch("Reco_ele_pfPhoIso") ? Reco_ele_pfPhoIso[ep_idx] : -999;
                ep_convVeto = tree->GetBranch("Reco_ele_convVeto") ? Reco_ele_convVeto[ep_idx] : -1;
                ep_nMissingHits = tree->GetBranch("Reco_ele_nMissingHits") ? Reco_ele_nMissingHits[ep_idx] : -1;
                ep_scEta = tree->GetBranch("Reco_ele_scEta") ? Reco_ele_scEta[ep_idx] : -999;
                ep_scPhi = tree->GetBranch("Reco_ele_scPhi") ? Reco_ele_scPhi[ep_idx] : -999;
                ep_scEn = tree->GetBranch("Reco_ele_scEn") ? Reco_ele_scEn[ep_idx] : -999;
                
                // e- (electron) info
                em_pt = (*Reco_ele_pt)[em_idx];
                em_eta = (*Reco_ele_eta)[em_idx];
                em_phi = (*Reco_ele_phi)[em_idx];
                em_y = (*Reco_ele_y)[em_idx];
                em_charge = Reco_ele_charge[em_idx];
                em_dxy = tree->GetBranch("Reco_ele_dxy") ? Reco_ele_dxy[em_idx] : -999;
                em_dz = tree->GetBranch("Reco_ele_dz") ? Reco_ele_dz[em_idx] : -999;
                em_sigmaIEtaIEta = tree->GetBranch("Reco_ele_sigmaIEtaIEta") ? Reco_ele_sigmaIEtaIEta[em_idx] : -999;
                em_hOverE = tree->GetBranch("Reco_ele_hOverE") ? Reco_ele_hOverE[em_idx] : -999;
                em_eOverP = tree->GetBranch("Reco_ele_eOverP") ? Reco_ele_eOverP[em_idx] : -999;
                em_deltaEtaIn = tree->GetBranch("Reco_ele_deltaEtaIn") ? Reco_ele_deltaEtaIn[em_idx] : -999;
                em_deltaPhiIn = tree->GetBranch("Reco_ele_deltaPhiIn") ? Reco_ele_deltaPhiIn[em_idx] : -999;
                em_r9 = tree->GetBranch("Reco_ele_r9") ? Reco_ele_r9[em_idx] : -999;
                em_fbrem = tree->GetBranch("Reco_ele_fbrem") ? Reco_ele_fbrem[em_idx] : -999;
                em_pfChIso = tree->GetBranch("Reco_ele_pfChIso") ? Reco_ele_pfChIso[em_idx] : -999;
                em_pfNeuIso = tree->GetBranch("Reco_ele_pfNeuIso") ? Reco_ele_pfNeuIso[em_idx] : -999;
                em_pfPhoIso = tree->GetBranch("Reco_ele_pfPhoIso") ? Reco_ele_pfPhoIso[em_idx] : -999;
                em_convVeto = tree->GetBranch("Reco_ele_convVeto") ? Reco_ele_convVeto[em_idx] : -1;
                em_nMissingHits = tree->GetBranch("Reco_ele_nMissingHits") ? Reco_ele_nMissingHits[em_idx] : -1;
                em_scEta = tree->GetBranch("Reco_ele_scEta") ? Reco_ele_scEta[em_idx] : -999;
                em_scPhi = tree->GetBranch("Reco_ele_scPhi") ? Reco_ele_scPhi[em_idx] : -999;
                em_scEn = tree->GetBranch("Reco_ele_scEn") ? Reco_ele_scEn[em_idx] : -999;
                
                eleTree->Fill();
                nDielectrons++;
            }
        }
    }
    
    // Write and close
    fOut->cd();
    muTree->Write();
    eleTree->Write();
    fOut->Close();
    fIn->Close();
    
}
