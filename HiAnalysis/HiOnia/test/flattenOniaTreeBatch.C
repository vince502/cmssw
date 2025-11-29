// Flatten OniaTree: Batch version - processes multiple files from a file list
// Usage: root -l -b -q 'flattenOniaTreeBatch.C("filelist.txt", "output.root")'

#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TLorentzVector.h>
#include <TClonesArray.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

void flattenOniaTreeBatch(const char* fileListPath = "filelist.txt",
                          const char* outputFile = "FlatOniaTree.root") {
    
    std::cout << "=== Flatten OniaTree (Batch) ===" << std::endl;
    std::cout << "File list: " << fileListPath << std::endl;
    std::cout << "Output:    " << outputFile << std::endl;
    
    // Read file list
    std::vector<std::string> inputFiles;
    std::ifstream fileList(fileListPath);
    std::string line;
    while (std::getline(fileList, line)) {
        if (!line.empty() && line[0] != '#') {
            inputFiles.push_back(line);
        }
    }
    fileList.close();
    
    std::cout << "Found " << inputFiles.size() << " input files" << std::endl;
    
    if (inputFiles.empty()) {
        std::cerr << "Error: No input files found!" << std::endl;
        return;
    }
    
    // Create TChain
    TChain* chain = new TChain("hioniaCombined/OniaTree");
    for (const auto& f : inputFiles) {
        chain->Add(f.c_str());
    }
    
    // Check if chain has entries
    Long64_t nEntries = chain->GetEntries();
    if (nEntries == 0) {
        // Try legacy tree
        delete chain;
        chain = new TChain("hionia/myTree");
        for (const auto& f : inputFiles) {
            chain->Add(f.c_str());
        }
        nEntries = chain->GetEntries();
    }
    
    std::cout << "Total entries in chain: " << nEntries << std::endl;
    
    if (nEntries == 0) {
        std::cerr << "Error: No entries found in chain!" << std::endl;
        return;
    }
    
    // Create output file
    TFile* fOut = new TFile(outputFile, "RECREATE");
    
    // ========================================
    // DIMUON FLAT TREE
    // ========================================
    TTree* muTree = new TTree("DimuonTree", "Flat dimuon tree");
    
    // Event info
    UInt_t mu_runNb, mu_LS;
    ULong64_t mu_eventNb;
    Int_t mu_Centrality;
    Float_t mu_zVtx;
    ULong64_t mu_HLTriggers;
    
    // Dimuon variables
    Float_t mu_mass, mu_pt, mu_eta, mu_phi, mu_y;
    Float_t mu_VtxProb, mu_ctau, mu_ctauErr, mu_cosAlpha;
    Int_t mu_charge;
    ULong64_t mu_trig;
    
    // mu+ variables
    Float_t mupl_pt, mupl_eta, mupl_phi;
    Int_t mupl_charge;
    Int_t mupl_nTrkHits, mupl_nPixWMea, mupl_nTrkWMea;
    Float_t mupl_dxy, mupl_dz;
    Bool_t mupl_isGlobal, mupl_isTracker, mupl_isSoft, mupl_isPF;
    
    // mu- variables
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
    
    // ========================================
    // DIELECTRON FLAT TREE
    // ========================================
    TTree* eleTree = new TTree("DielectronTree", "Flat dielectron tree");
    
    // Event info
    UInt_t ele_runNb, ele_LS;
    ULong64_t ele_eventNb;
    Int_t ele_Centrality;
    Float_t ele_zVtx;
    ULong64_t ele_HLTriggers;
    
    // Dielectron variables
    Float_t ee_mass, ee_pt, ee_eta, ee_phi, ee_y;
    Float_t ee_VtxProb;
    Int_t ee_charge;
    ULong64_t ee_trig;
    
    // e+ variables
    Float_t ep_pt, ep_eta, ep_phi, ep_y;
    Int_t ep_charge;
    Float_t ep_dxy, ep_dz;
    Float_t ep_sigmaIEtaIEta, ep_hOverE, ep_eOverP;
    Float_t ep_deltaEtaIn, ep_deltaPhiIn;
    Float_t ep_r9, ep_fbrem;
    Float_t ep_pfChIso, ep_pfNeuIso, ep_pfPhoIso;
    Short_t ep_convVeto, ep_nMissingHits;
    Float_t ep_scEta, ep_scPhi, ep_scEn;
    
    // e- variables
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
    Int_t Reco_ee_charge[100];
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
    chain->SetBranchAddress("runNb", &in_runNb);
    chain->SetBranchAddress("eventNb", &in_eventNb);
    chain->SetBranchAddress("LS", &in_LS);
    chain->SetBranchAddress("Centrality", &in_Centrality);
    chain->SetBranchAddress("zVtx", &in_zVtx);
    chain->SetBranchAddress("HLTriggers", &in_HLTriggers);
    
    // Dimuon branches
    bool hasMuons = (chain->GetBranch("Reco_QQ_size") != nullptr);
    bool useVectorMom = (chain->GetBranch("Reco_QQ_4mom_pt") != nullptr);
    
    if (hasMuons) {
        chain->SetBranchAddress("Reco_QQ_size", &Reco_QQ_size);
        chain->SetBranchAddress("Reco_mu_size", &Reco_mu_size);
        
        if (useVectorMom) {
            chain->SetBranchAddress("Reco_QQ_4mom_pt", &Reco_QQ_4mom_pt);
            chain->SetBranchAddress("Reco_QQ_4mom_eta", &Reco_QQ_4mom_eta);
            chain->SetBranchAddress("Reco_QQ_4mom_phi", &Reco_QQ_4mom_phi);
            chain->SetBranchAddress("Reco_QQ_4mom_m", &Reco_QQ_4mom_m);
            chain->SetBranchAddress("Reco_mu_4mom_pt", &Reco_mu_4mom_pt);
            chain->SetBranchAddress("Reco_mu_4mom_eta", &Reco_mu_4mom_eta);
            chain->SetBranchAddress("Reco_mu_4mom_phi", &Reco_mu_4mom_phi);
            chain->SetBranchAddress("Reco_mu_4mom_m", &Reco_mu_4mom_m);
        }
        
        chain->SetBranchAddress("Reco_QQ_mupl_idx", Reco_QQ_mupl_idx);
        chain->SetBranchAddress("Reco_QQ_mumi_idx", Reco_QQ_mumi_idx);
        chain->SetBranchAddress("Reco_QQ_sign", Reco_QQ_sign);
        chain->SetBranchAddress("Reco_QQ_VtxProb", Reco_QQ_VtxProb);
        chain->SetBranchAddress("Reco_QQ_ctau", Reco_QQ_ctau);
        chain->SetBranchAddress("Reco_QQ_ctauErr", Reco_QQ_ctauErr);
        if (chain->GetBranch("Reco_QQ_cosAlpha")) chain->SetBranchAddress("Reco_QQ_cosAlpha", Reco_QQ_cosAlpha);
        chain->SetBranchAddress("Reco_QQ_trig", Reco_QQ_trig);
        
        chain->SetBranchAddress("Reco_mu_charge", Reco_mu_charge);
        if (chain->GetBranch("Reco_mu_nTrkHits")) chain->SetBranchAddress("Reco_mu_nTrkHits", Reco_mu_nTrkHits);
        if (chain->GetBranch("Reco_mu_nPixWMea")) chain->SetBranchAddress("Reco_mu_nPixWMea", Reco_mu_nPixWMea);
        if (chain->GetBranch("Reco_mu_nTrkWMea")) chain->SetBranchAddress("Reco_mu_nTrkWMea", Reco_mu_nTrkWMea);
        if (chain->GetBranch("Reco_mu_dxy")) chain->SetBranchAddress("Reco_mu_dxy", Reco_mu_dxy);
        if (chain->GetBranch("Reco_mu_dz")) chain->SetBranchAddress("Reco_mu_dz", Reco_mu_dz);
        if (chain->GetBranch("Reco_mu_isGlobal")) chain->SetBranchAddress("Reco_mu_isGlobal", Reco_mu_isGlobal);
        if (chain->GetBranch("Reco_mu_isTracker")) chain->SetBranchAddress("Reco_mu_isTracker", Reco_mu_isTracker);
        if (chain->GetBranch("Reco_mu_isSoft")) chain->SetBranchAddress("Reco_mu_isSoft", Reco_mu_isSoft);
        if (chain->GetBranch("Reco_mu_isPF")) chain->SetBranchAddress("Reco_mu_isPF", Reco_mu_isPF);
    }
    
    // Dielectron branches
    bool hasElectrons = (chain->GetBranch("Reco_ee_size") != nullptr);
    
    if (hasElectrons) {
        chain->SetBranchAddress("Reco_ee_size", &Reco_ee_size);
        chain->SetBranchAddress("Reco_ele_size", &Reco_ele_size);
        chain->SetBranchAddress("Reco_ee_pt", &Reco_ee_pt);
        chain->SetBranchAddress("Reco_ee_eta", &Reco_ee_eta);
        chain->SetBranchAddress("Reco_ee_phi", &Reco_ee_phi);
        chain->SetBranchAddress("Reco_ee_y", &Reco_ee_y);
        chain->SetBranchAddress("Reco_ee_mass", &Reco_ee_mass);
        chain->SetBranchAddress("Reco_ee_ele1Idx", Reco_ee_ele1Idx);
        chain->SetBranchAddress("Reco_ee_ele2Idx", Reco_ee_ele2Idx);
        chain->SetBranchAddress("Reco_ee_charge", Reco_ee_charge);
        if (chain->GetBranch("Reco_ee_vProb")) chain->SetBranchAddress("Reco_ee_vProb", Reco_ee_vProb);
        if (chain->GetBranch("Reco_ee_trigBits")) chain->SetBranchAddress("Reco_ee_trigBits", Reco_ee_trigBits);
        
        chain->SetBranchAddress("Reco_ele_pt", &Reco_ele_pt);
        chain->SetBranchAddress("Reco_ele_eta", &Reco_ele_eta);
        chain->SetBranchAddress("Reco_ele_phi", &Reco_ele_phi);
        chain->SetBranchAddress("Reco_ele_y", &Reco_ele_y);
        chain->SetBranchAddress("Reco_ele_charge", Reco_ele_charge);
        if (chain->GetBranch("Reco_ele_dxy")) chain->SetBranchAddress("Reco_ele_dxy", Reco_ele_dxy);
        if (chain->GetBranch("Reco_ele_dz")) chain->SetBranchAddress("Reco_ele_dz", Reco_ele_dz);
        if (chain->GetBranch("Reco_ele_sigmaIEtaIEta")) chain->SetBranchAddress("Reco_ele_sigmaIEtaIEta", Reco_ele_sigmaIEtaIEta);
        if (chain->GetBranch("Reco_ele_hOverE")) chain->SetBranchAddress("Reco_ele_hOverE", Reco_ele_hOverE);
        if (chain->GetBranch("Reco_ele_eOverP")) chain->SetBranchAddress("Reco_ele_eOverP", Reco_ele_eOverP);
        if (chain->GetBranch("Reco_ele_deltaEtaIn")) chain->SetBranchAddress("Reco_ele_deltaEtaIn", Reco_ele_deltaEtaIn);
        if (chain->GetBranch("Reco_ele_deltaPhiIn")) chain->SetBranchAddress("Reco_ele_deltaPhiIn", Reco_ele_deltaPhiIn);
        if (chain->GetBranch("Reco_ele_r9")) chain->SetBranchAddress("Reco_ele_r9", Reco_ele_r9);
        if (chain->GetBranch("Reco_ele_fbrem")) chain->SetBranchAddress("Reco_ele_fbrem", Reco_ele_fbrem);
        if (chain->GetBranch("Reco_ele_pfChIso")) chain->SetBranchAddress("Reco_ele_pfChIso", Reco_ele_pfChIso);
        if (chain->GetBranch("Reco_ele_pfNeuIso")) chain->SetBranchAddress("Reco_ele_pfNeuIso", Reco_ele_pfNeuIso);
        if (chain->GetBranch("Reco_ele_pfPhoIso")) chain->SetBranchAddress("Reco_ele_pfPhoIso", Reco_ele_pfPhoIso);
        if (chain->GetBranch("Reco_ele_convVeto")) chain->SetBranchAddress("Reco_ele_convVeto", Reco_ele_convVeto);
        if (chain->GetBranch("Reco_ele_nMissingHits")) chain->SetBranchAddress("Reco_ele_nMissingHits", Reco_ele_nMissingHits);
        if (chain->GetBranch("Reco_ele_scEta")) chain->SetBranchAddress("Reco_ele_scEta", Reco_ele_scEta);
        if (chain->GetBranch("Reco_ele_scPhi")) chain->SetBranchAddress("Reco_ele_scPhi", Reco_ele_scPhi);
        if (chain->GetBranch("Reco_ele_scEn")) chain->SetBranchAddress("Reco_ele_scEn", Reco_ele_scEn);
    }
    
    std::cout << "Has muons: " << hasMuons << ", Has electrons: " << hasElectrons << std::endl;
    std::cout << "Using vector momentum format: " << useVectorMom << std::endl;
    
    // ========================================
    // Event loop
    // ========================================
    Long64_t nDimuons = 0, nDielectrons = 0;
    
    for (Long64_t iEntry = 0; iEntry < nEntries; iEntry++) {
        if (iEntry % 50000 == 0) {
            std::cout << "Processing entry " << iEntry << " / " << nEntries 
                      << " (" << 100.0*iEntry/nEntries << "%)" << std::endl;
        }
        
        chain->GetEntry(iEntry);
        
        // Process dimuons
        if (hasMuons && useVectorMom && Reco_QQ_4mom_pt) {
            for (int iQQ = 0; iQQ < Reco_QQ_size; iQQ++) {
                int mupl_idx = Reco_QQ_mupl_idx[iQQ];
                int mumi_idx = Reco_QQ_mumi_idx[iQQ];
                
                if (mupl_idx < 0 || mumi_idx < 0) continue;
                if (mupl_idx >= Reco_mu_size || mumi_idx >= Reco_mu_size) continue;
                
                mu_runNb = in_runNb;
                mu_eventNb = in_eventNb;
                mu_LS = in_LS;
                mu_Centrality = in_Centrality;
                mu_zVtx = in_zVtx;
                mu_HLTriggers = in_HLTriggers;
                
                mu_pt = (*Reco_QQ_4mom_pt)[iQQ];
                mu_eta = (*Reco_QQ_4mom_eta)[iQQ];
                mu_phi = (*Reco_QQ_4mom_phi)[iQQ];
                mu_mass = (*Reco_QQ_4mom_m)[iQQ];
                TLorentzVector v; v.SetPtEtaPhiM(mu_pt, mu_eta, mu_phi, mu_mass);
                mu_y = v.Rapidity();
                
                mu_charge = Reco_QQ_sign[iQQ];
                mu_VtxProb = Reco_QQ_VtxProb[iQQ];
                mu_ctau = Reco_QQ_ctau[iQQ];
                mu_ctauErr = Reco_QQ_ctauErr[iQQ];
                mu_cosAlpha = chain->GetBranch("Reco_QQ_cosAlpha") ? Reco_QQ_cosAlpha[iQQ] : -999;
                mu_trig = Reco_QQ_trig[iQQ];
                
                mupl_pt = (*Reco_mu_4mom_pt)[mupl_idx];
                mupl_eta = (*Reco_mu_4mom_eta)[mupl_idx];
                mupl_phi = (*Reco_mu_4mom_phi)[mupl_idx];
                mupl_charge = Reco_mu_charge[mupl_idx];
                mupl_nTrkHits = chain->GetBranch("Reco_mu_nTrkHits") ? Reco_mu_nTrkHits[mupl_idx] : -1;
                mupl_nPixWMea = chain->GetBranch("Reco_mu_nPixWMea") ? Reco_mu_nPixWMea[mupl_idx] : -1;
                mupl_nTrkWMea = chain->GetBranch("Reco_mu_nTrkWMea") ? Reco_mu_nTrkWMea[mupl_idx] : -1;
                mupl_dxy = chain->GetBranch("Reco_mu_dxy") ? Reco_mu_dxy[mupl_idx] : -999;
                mupl_dz = chain->GetBranch("Reco_mu_dz") ? Reco_mu_dz[mupl_idx] : -999;
                mupl_isGlobal = chain->GetBranch("Reco_mu_isGlobal") ? Reco_mu_isGlobal[mupl_idx] : false;
                mupl_isTracker = chain->GetBranch("Reco_mu_isTracker") ? Reco_mu_isTracker[mupl_idx] : false;
                mupl_isSoft = chain->GetBranch("Reco_mu_isSoft") ? Reco_mu_isSoft[mupl_idx] : false;
                mupl_isPF = chain->GetBranch("Reco_mu_isPF") ? Reco_mu_isPF[mupl_idx] : false;
                
                mumi_pt = (*Reco_mu_4mom_pt)[mumi_idx];
                mumi_eta = (*Reco_mu_4mom_eta)[mumi_idx];
                mumi_phi = (*Reco_mu_4mom_phi)[mumi_idx];
                mumi_charge = Reco_mu_charge[mumi_idx];
                mumi_nTrkHits = chain->GetBranch("Reco_mu_nTrkHits") ? Reco_mu_nTrkHits[mumi_idx] : -1;
                mumi_nPixWMea = chain->GetBranch("Reco_mu_nPixWMea") ? Reco_mu_nPixWMea[mumi_idx] : -1;
                mumi_nTrkWMea = chain->GetBranch("Reco_mu_nTrkWMea") ? Reco_mu_nTrkWMea[mumi_idx] : -1;
                mumi_dxy = chain->GetBranch("Reco_mu_dxy") ? Reco_mu_dxy[mumi_idx] : -999;
                mumi_dz = chain->GetBranch("Reco_mu_dz") ? Reco_mu_dz[mumi_idx] : -999;
                mumi_isGlobal = chain->GetBranch("Reco_mu_isGlobal") ? Reco_mu_isGlobal[mumi_idx] : false;
                mumi_isTracker = chain->GetBranch("Reco_mu_isTracker") ? Reco_mu_isTracker[mumi_idx] : false;
                mumi_isSoft = chain->GetBranch("Reco_mu_isSoft") ? Reco_mu_isSoft[mumi_idx] : false;
                mumi_isPF = chain->GetBranch("Reco_mu_isPF") ? Reco_mu_isPF[mumi_idx] : false;
                
                muTree->Fill();
                nDimuons++;
            }
        }
        
        // Process dielectrons
        if (hasElectrons && Reco_ee_pt) {
            for (int iEE = 0; iEE < Reco_ee_size; iEE++) {
                int ele1_idx = Reco_ee_ele1Idx[iEE];
                int ele2_idx = Reco_ee_ele2Idx[iEE];
                
                if (ele1_idx < 0 || ele2_idx < 0) continue;
                if (ele1_idx >= Reco_ele_size || ele2_idx >= Reco_ele_size) continue;
                
                int ep_idx, em_idx;
                if (Reco_ele_charge[ele1_idx] > 0) {
                    ep_idx = ele1_idx;
                    em_idx = ele2_idx;
                } else {
                    ep_idx = ele2_idx;
                    em_idx = ele1_idx;
                }
                
                ele_runNb = in_runNb;
                ele_eventNb = in_eventNb;
                ele_LS = in_LS;
                ele_Centrality = in_Centrality;
                ele_zVtx = in_zVtx;
                ele_HLTriggers = in_HLTriggers;
                
                ee_pt = (*Reco_ee_pt)[iEE];
                ee_eta = (*Reco_ee_eta)[iEE];
                ee_phi = (*Reco_ee_phi)[iEE];
                ee_y = (*Reco_ee_y)[iEE];
                ee_mass = (*Reco_ee_mass)[iEE];
                ee_charge = Reco_ee_charge[iEE];
                ee_VtxProb = chain->GetBranch("Reco_ee_vProb") ? Reco_ee_vProb[iEE] : -999;
                ee_trig = chain->GetBranch("Reco_ee_trigBits") ? Reco_ee_trigBits[iEE] : 0;
                
                ep_pt = (*Reco_ele_pt)[ep_idx];
                ep_eta = (*Reco_ele_eta)[ep_idx];
                ep_phi = (*Reco_ele_phi)[ep_idx];
                ep_y = (*Reco_ele_y)[ep_idx];
                ep_charge = Reco_ele_charge[ep_idx];
                ep_dxy = chain->GetBranch("Reco_ele_dxy") ? Reco_ele_dxy[ep_idx] : -999;
                ep_dz = chain->GetBranch("Reco_ele_dz") ? Reco_ele_dz[ep_idx] : -999;
                ep_sigmaIEtaIEta = chain->GetBranch("Reco_ele_sigmaIEtaIEta") ? Reco_ele_sigmaIEtaIEta[ep_idx] : -999;
                ep_hOverE = chain->GetBranch("Reco_ele_hOverE") ? Reco_ele_hOverE[ep_idx] : -999;
                ep_eOverP = chain->GetBranch("Reco_ele_eOverP") ? Reco_ele_eOverP[ep_idx] : -999;
                ep_deltaEtaIn = chain->GetBranch("Reco_ele_deltaEtaIn") ? Reco_ele_deltaEtaIn[ep_idx] : -999;
                ep_deltaPhiIn = chain->GetBranch("Reco_ele_deltaPhiIn") ? Reco_ele_deltaPhiIn[ep_idx] : -999;
                ep_r9 = chain->GetBranch("Reco_ele_r9") ? Reco_ele_r9[ep_idx] : -999;
                ep_fbrem = chain->GetBranch("Reco_ele_fbrem") ? Reco_ele_fbrem[ep_idx] : -999;
                ep_pfChIso = chain->GetBranch("Reco_ele_pfChIso") ? Reco_ele_pfChIso[ep_idx] : -999;
                ep_pfNeuIso = chain->GetBranch("Reco_ele_pfNeuIso") ? Reco_ele_pfNeuIso[ep_idx] : -999;
                ep_pfPhoIso = chain->GetBranch("Reco_ele_pfPhoIso") ? Reco_ele_pfPhoIso[ep_idx] : -999;
                ep_convVeto = chain->GetBranch("Reco_ele_convVeto") ? Reco_ele_convVeto[ep_idx] : -1;
                ep_nMissingHits = chain->GetBranch("Reco_ele_nMissingHits") ? Reco_ele_nMissingHits[ep_idx] : -1;
                ep_scEta = chain->GetBranch("Reco_ele_scEta") ? Reco_ele_scEta[ep_idx] : -999;
                ep_scPhi = chain->GetBranch("Reco_ele_scPhi") ? Reco_ele_scPhi[ep_idx] : -999;
                ep_scEn = chain->GetBranch("Reco_ele_scEn") ? Reco_ele_scEn[ep_idx] : -999;
                
                em_pt = (*Reco_ele_pt)[em_idx];
                em_eta = (*Reco_ele_eta)[em_idx];
                em_phi = (*Reco_ele_phi)[em_idx];
                em_y = (*Reco_ele_y)[em_idx];
                em_charge = Reco_ele_charge[em_idx];
                em_dxy = chain->GetBranch("Reco_ele_dxy") ? Reco_ele_dxy[em_idx] : -999;
                em_dz = chain->GetBranch("Reco_ele_dz") ? Reco_ele_dz[em_idx] : -999;
                em_sigmaIEtaIEta = chain->GetBranch("Reco_ele_sigmaIEtaIEta") ? Reco_ele_sigmaIEtaIEta[em_idx] : -999;
                em_hOverE = chain->GetBranch("Reco_ele_hOverE") ? Reco_ele_hOverE[em_idx] : -999;
                em_eOverP = chain->GetBranch("Reco_ele_eOverP") ? Reco_ele_eOverP[em_idx] : -999;
                em_deltaEtaIn = chain->GetBranch("Reco_ele_deltaEtaIn") ? Reco_ele_deltaEtaIn[em_idx] : -999;
                em_deltaPhiIn = chain->GetBranch("Reco_ele_deltaPhiIn") ? Reco_ele_deltaPhiIn[em_idx] : -999;
                em_r9 = chain->GetBranch("Reco_ele_r9") ? Reco_ele_r9[em_idx] : -999;
                em_fbrem = chain->GetBranch("Reco_ele_fbrem") ? Reco_ele_fbrem[em_idx] : -999;
                em_pfChIso = chain->GetBranch("Reco_ele_pfChIso") ? Reco_ele_pfChIso[em_idx] : -999;
                em_pfNeuIso = chain->GetBranch("Reco_ele_pfNeuIso") ? Reco_ele_pfNeuIso[em_idx] : -999;
                em_pfPhoIso = chain->GetBranch("Reco_ele_pfPhoIso") ? Reco_ele_pfPhoIso[em_idx] : -999;
                em_convVeto = chain->GetBranch("Reco_ele_convVeto") ? Reco_ele_convVeto[em_idx] : -1;
                em_nMissingHits = chain->GetBranch("Reco_ele_nMissingHits") ? Reco_ele_nMissingHits[em_idx] : -1;
                em_scEta = chain->GetBranch("Reco_ele_scEta") ? Reco_ele_scEta[em_idx] : -999;
                em_scPhi = chain->GetBranch("Reco_ele_scPhi") ? Reco_ele_scPhi[em_idx] : -999;
                em_scEn = chain->GetBranch("Reco_ele_scEn") ? Reco_ele_scEn[em_idx] : -999;
                
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
    
    delete chain;
    
    std::cout << "=== Done ===" << std::endl;
    std::cout << "Total dimuons:     " << nDimuons << std::endl;
    std::cout << "Total dielectrons: " << nDielectrons << std::endl;
    std::cout << "Output: " << outputFile << std::endl;
}
