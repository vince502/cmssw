// Flatten the dielectron tree structure
// Usage: root -l -b -q 'flattenDielectronTree.C("input.root", "output_flat.root")'
//
// Gen Matching (MC only):
//   - Matching is computed in the analyzer (daughter-by-daughter, dR < 0.1, charge-correct)
//   - Reco_ee_isGenMatched: 1 if both daughters matched, 0 otherwise
//   - Reco_ee_gen_ele1_dR, Reco_ee_gen_ele2_dR: matching quality for each daughter
//   - Example custom cut: "Reco_ee_isGenMatched==1 && Reco_ee_gen_ele1_dR<0.05"

#include "TFile.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include <iostream>
#include <vector>
#include <cmath>

void flattenDielectronTree(const char* inputFile = "OniaTree_MuonElectron.root", 
                            const char* outputFile = "FlatDielectronTree.root") {
    
    std::cout << "Opening input file: " << inputFile << std::endl;
    TFile* fin = TFile::Open(inputFile, "READ");
    if (!fin || fin->IsZombie()) {
        std::cerr << "ERROR: Cannot open input file!" << std::endl;
        return;
    }
    
    TTree* tree = (TTree*)fin->Get("hioniaElectrons/eleTree");
    if (!tree) {
        std::cerr << "ERROR: Cannot find hioniaElectrons/eleTree!" << std::endl;
        fin->Close();
        return;
    }
    
    std::cout << "Input tree has " << tree->GetEntries() << " events" << std::endl;
    
    // Input branches - Event info
    UInt_t run, lumi;
    ULong64_t event;
    Int_t nPV;
    Float_t pvX, pvY, pvZ;
    Float_t centrality;
    Int_t centralityBin;
    
    // Input branches - Dielectron (vectors for kinematics, arrays for others)
    Int_t Reco_ee_size;
    std::vector<float> *Reco_ee_mass = 0;
    std::vector<float> *Reco_ee_pt = 0;
    std::vector<float> *Reco_ee_eta = 0;
    std::vector<float> *Reco_ee_phi = 0;
    std::vector<float> *Reco_ee_y = 0;
    Float_t Reco_ee_vProb[1000];
    Short_t Reco_ee_charge[1000];
    Short_t Reco_ee_ele1Idx[1000];
    Short_t Reco_ee_ele2Idx[1000];
    ULong64_t Reco_ee_trigBits[1000];
    
    // Gen-matching info (MC only, already computed in analyzer)
    Short_t Reco_ee_isGenMatched[1000];
    Short_t Reco_ee_matchedGenIdx[1000];
    Float_t Reco_ee_gen_ele1_dR[1000];
    Float_t Reco_ee_gen_ele2_dR[1000];
    
    // Input branches - Electrons (vectors for kinematics, arrays for others)
    Int_t Reco_ele_size;
    std::vector<float> *Reco_ele_pt = 0;
    std::vector<float> *Reco_ele_eta = 0;
    std::vector<float> *Reco_ele_phi = 0;
    std::vector<float> *Reco_ele_y = 0;
    std::vector<float> *Reco_ele_mass = 0;
    Short_t Reco_ele_charge[1000];
    
    // Track info
    Float_t Reco_ele_dxy[1000];
    Float_t Reco_ele_dz[1000];
    Float_t Reco_ele_dxyErr[1000];
    Float_t Reco_ele_dzErr[1000];
    Short_t Reco_ele_nValidHits[1000];
    Short_t Reco_ele_nMissingHits[1000];
    
    // SuperCluster
    Float_t Reco_ele_scEta[1000];
    Float_t Reco_ele_scPhi[1000];
    Float_t Reco_ele_scEn[1000];
    
    // Shower shape & ID
    Float_t Reco_ele_sigmaIEtaIEta[1000];
    Float_t Reco_ele_hOverE[1000];
    Float_t Reco_ele_r9[1000];
    Float_t Reco_ele_deltaEtaIn[1000];
    Float_t Reco_ele_deltaPhiIn[1000];
    Float_t Reco_ele_eOverP[1000];
    
    // Isolation
    Float_t Reco_ele_pfChIso[1000];
    Float_t Reco_ele_pfNeuIso[1000];
    Float_t Reco_ele_pfPhoIso[1000];
    Float_t Reco_ele_pfPUIso[1000];
    
    // Conversion veto
    Short_t Reco_ele_convVeto[1000];
    
    // Generator info (MC only)
    Int_t Gen_ele_size = 0;
    std::vector<float> *Gen_ele_pt = 0;
    std::vector<float> *Gen_ele_eta = 0;
    std::vector<float> *Gen_ele_phi = 0;
    std::vector<float> *Gen_ele_mass = 0;
    Int_t Gen_ele_pdgId[1000];
    Int_t Gen_ele_motherId[1000];
    
    Int_t Gen_ee_size = 0;
    std::vector<float> *Gen_ee_pt = 0;
    std::vector<float> *Gen_ee_eta = 0;
    std::vector<float> *Gen_ee_phi = 0;
    std::vector<float> *Gen_ee_mass = 0;
    Short_t Gen_ee_charge[1000];
    Short_t Gen_ee_ele1Idx[1000];
    Short_t Gen_ee_ele2Idx[1000];
    
    // Set branch addresses - Event
    tree->SetBranchAddress("run", &run);
    tree->SetBranchAddress("lumi", &lumi);
    tree->SetBranchAddress("event", &event);
    tree->SetBranchAddress("nPV", &nPV);
    tree->SetBranchAddress("pvX", &pvX);
    tree->SetBranchAddress("pvY", &pvY);
    tree->SetBranchAddress("pvZ", &pvZ);
    tree->SetBranchAddress("centrality", &centrality);
    tree->SetBranchAddress("centralityBin", &centralityBin);
    
    // Set branch addresses - Dielectron
    tree->SetBranchAddress("Reco_ee_size", &Reco_ee_size);
    tree->SetBranchAddress("Reco_ee_mass", &Reco_ee_mass);
    tree->SetBranchAddress("Reco_ee_pt", &Reco_ee_pt);
    tree->SetBranchAddress("Reco_ee_eta", &Reco_ee_eta);
    tree->SetBranchAddress("Reco_ee_phi", &Reco_ee_phi);
    tree->SetBranchAddress("Reco_ee_y", &Reco_ee_y);
    tree->SetBranchAddress("Reco_ee_vProb", Reco_ee_vProb);
    tree->SetBranchAddress("Reco_ee_charge", Reco_ee_charge);
    tree->SetBranchAddress("Reco_ee_ele1Idx", Reco_ee_ele1Idx);
    tree->SetBranchAddress("Reco_ee_ele2Idx", Reco_ee_ele2Idx);
    tree->SetBranchAddress("Reco_ee_trigBits", Reco_ee_trigBits);
    
    // Set branch addresses for gen-matching info (check if branches exist)
    bool hasGenMatching = (tree->GetBranch("Reco_ee_isGenMatched") != nullptr);
    if (hasGenMatching) {
        tree->SetBranchAddress("Reco_ee_isGenMatched", Reco_ee_isGenMatched);
        tree->SetBranchAddress("Reco_ee_matchedGenIdx", Reco_ee_matchedGenIdx);
        tree->SetBranchAddress("Reco_ee_gen_ele1_dR", Reco_ee_gen_ele1_dR);
        tree->SetBranchAddress("Reco_ee_gen_ele2_dR", Reco_ee_gen_ele2_dR);
        std::cout << "Gen-matching branches found (computed in analyzer)" << std::endl;
    }
    
    // Set branch addresses - Electrons
    tree->SetBranchAddress("Reco_ele_size", &Reco_ele_size);
    tree->SetBranchAddress("Reco_ele_pt", &Reco_ele_pt);
    tree->SetBranchAddress("Reco_ele_eta", &Reco_ele_eta);
    tree->SetBranchAddress("Reco_ele_phi", &Reco_ele_phi);
    tree->SetBranchAddress("Reco_ele_y", &Reco_ele_y);
    tree->SetBranchAddress("Reco_ele_mass", &Reco_ele_mass);
    tree->SetBranchAddress("Reco_ele_charge", Reco_ele_charge);
    tree->SetBranchAddress("Reco_ele_dxy", Reco_ele_dxy);
    tree->SetBranchAddress("Reco_ele_dz", Reco_ele_dz);
    tree->SetBranchAddress("Reco_ele_dxyErr", Reco_ele_dxyErr);
    tree->SetBranchAddress("Reco_ele_dzErr", Reco_ele_dzErr);
    tree->SetBranchAddress("Reco_ele_nValidHits", Reco_ele_nValidHits);
    tree->SetBranchAddress("Reco_ele_nMissingHits", Reco_ele_nMissingHits);
    tree->SetBranchAddress("Reco_ele_scEta", Reco_ele_scEta);
    tree->SetBranchAddress("Reco_ele_scPhi", Reco_ele_scPhi);
    tree->SetBranchAddress("Reco_ele_scEn", Reco_ele_scEn);
    tree->SetBranchAddress("Reco_ele_sigmaIEtaIEta", Reco_ele_sigmaIEtaIEta);
    tree->SetBranchAddress("Reco_ele_hOverE", Reco_ele_hOverE);
    tree->SetBranchAddress("Reco_ele_r9", Reco_ele_r9);
    tree->SetBranchAddress("Reco_ele_deltaEtaIn", Reco_ele_deltaEtaIn);
    tree->SetBranchAddress("Reco_ele_deltaPhiIn", Reco_ele_deltaPhiIn);
    tree->SetBranchAddress("Reco_ele_eOverP", Reco_ele_eOverP);
    tree->SetBranchAddress("Reco_ele_pfChIso", Reco_ele_pfChIso);
    tree->SetBranchAddress("Reco_ele_pfNeuIso", Reco_ele_pfNeuIso);
    tree->SetBranchAddress("Reco_ele_pfPhoIso", Reco_ele_pfPhoIso);
    tree->SetBranchAddress("Reco_ele_pfPUIso", Reco_ele_pfPUIso);
    tree->SetBranchAddress("Reco_ele_convVeto", Reco_ele_convVeto);
    
    // Set branch addresses for Gen info (check if branches exist)
    bool isMC = (tree->GetBranch("Gen_ele_size") != nullptr);
    if (isMC) {
        tree->SetBranchAddress("Gen_ele_size", &Gen_ele_size);
        tree->SetBranchAddress("Gen_ele_pt", &Gen_ele_pt);
        tree->SetBranchAddress("Gen_ele_eta", &Gen_ele_eta);
        tree->SetBranchAddress("Gen_ele_phi", &Gen_ele_phi);
        tree->SetBranchAddress("Gen_ele_mass", &Gen_ele_mass);
        tree->SetBranchAddress("Gen_ele_pdgId", Gen_ele_pdgId);
        tree->SetBranchAddress("Gen_ele_motherId", Gen_ele_motherId);
        
        tree->SetBranchAddress("Gen_ee_size", &Gen_ee_size);
        tree->SetBranchAddress("Gen_ee_pt", &Gen_ee_pt);
        tree->SetBranchAddress("Gen_ee_eta", &Gen_ee_eta);
        tree->SetBranchAddress("Gen_ee_phi", &Gen_ee_phi);
        tree->SetBranchAddress("Gen_ee_mass", &Gen_ee_mass);
        tree->SetBranchAddress("Gen_ee_charge", Gen_ee_charge);
        tree->SetBranchAddress("Gen_ee_ele1Idx", Gen_ee_ele1Idx);
        tree->SetBranchAddress("Gen_ee_ele2Idx", Gen_ee_ele2Idx);
        std::cout << "MC mode: Gen branches found and loaded" << std::endl;
    } else {
        std::cout << "Data mode: No Gen branches found" << std::endl;
    }
    
    // Create output file and tree
    TFile* fout = new TFile(outputFile, "RECREATE");
    TTree* outTree = new TTree("dielectronTree", "Flattened dielectron tree");
    
    // Output branches - Event info
    UInt_t o_runNb, o_lumiSection;
    ULong64_t o_eventNb;
    Int_t o_nPV;
    Float_t o_pvX, o_pvY, o_pvZ;
    Float_t o_centrality;
    Int_t o_centralityBin;
    
    outTree->Branch("runNb", &o_runNb);
    outTree->Branch("lumiSection", &o_lumiSection);
    outTree->Branch("eventNb", &o_eventNb);
    outTree->Branch("nPV", &o_nPV);
    outTree->Branch("pvX", &o_pvX);
    outTree->Branch("pvY", &o_pvY);
    outTree->Branch("pvZ", &o_pvZ);
    outTree->Branch("centrality", &o_centrality);
    outTree->Branch("centralityBin", &o_centralityBin);
    
    // Output branches - Dielectron
    Float_t ee_mass, ee_pt, ee_eta, ee_phi, ee_y;
    Float_t ee_vProb, ee_vChi2;
    Int_t ee_charge;
    ULong64_t ee_trigBits;
    
    outTree->Branch("ee_mass", &ee_mass);
    outTree->Branch("ee_pt", &ee_pt);
    outTree->Branch("ee_eta", &ee_eta);
    outTree->Branch("ee_phi", &ee_phi);
    outTree->Branch("ee_y", &ee_y);
    outTree->Branch("ee_vProb", &ee_vProb);
    outTree->Branch("ee_vChi2", &ee_vChi2);
    outTree->Branch("ee_charge", &ee_charge);
    outTree->Branch("ee_trigBits", &ee_trigBits);
    
    // Output branches - Electron 1
    Float_t ele1_pt, ele1_eta, ele1_phi, ele1_mass;
    Int_t ele1_charge;
    Float_t ele1_dxy, ele1_dz, ele1_dxyErr, ele1_dzErr;
    Int_t ele1_nValidHits, ele1_nMissingHits;
    Float_t ele1_scEta, ele1_scPhi, ele1_scEn;
    Float_t ele1_sigmaIEtaIEta, ele1_hOverE, ele1_r9;
    Float_t ele1_deltaEtaIn, ele1_deltaPhiIn, ele1_eOverP;
    Float_t ele1_pfChIso, ele1_pfNeuIso, ele1_pfPhoIso, ele1_pfPUIso;
    Short_t ele1_convVeto;
    
    outTree->Branch("ele1_pt", &ele1_pt);
    outTree->Branch("ele1_eta", &ele1_eta);
    outTree->Branch("ele1_phi", &ele1_phi);
    outTree->Branch("ele1_mass", &ele1_mass);
    outTree->Branch("ele1_charge", &ele1_charge);
    outTree->Branch("ele1_dxy", &ele1_dxy);
    outTree->Branch("ele1_dz", &ele1_dz);
    outTree->Branch("ele1_dxyErr", &ele1_dxyErr);
    outTree->Branch("ele1_dzErr", &ele1_dzErr);
    outTree->Branch("ele1_nValidHits", &ele1_nValidHits);
    outTree->Branch("ele1_nMissingHits", &ele1_nMissingHits);
    outTree->Branch("ele1_scEta", &ele1_scEta);
    outTree->Branch("ele1_scPhi", &ele1_scPhi);
    outTree->Branch("ele1_scEn", &ele1_scEn);
    outTree->Branch("ele1_sigmaIEtaIEta", &ele1_sigmaIEtaIEta);
    outTree->Branch("ele1_hOverE", &ele1_hOverE);
    outTree->Branch("ele1_r9", &ele1_r9);
    outTree->Branch("ele1_deltaEtaIn", &ele1_deltaEtaIn);
    outTree->Branch("ele1_deltaPhiIn", &ele1_deltaPhiIn);
    outTree->Branch("ele1_eOverP", &ele1_eOverP);
    outTree->Branch("ele1_pfChIso", &ele1_pfChIso);
    outTree->Branch("ele1_pfNeuIso", &ele1_pfNeuIso);
    outTree->Branch("ele1_pfPhoIso", &ele1_pfPhoIso);
    outTree->Branch("ele1_pfPUIso", &ele1_pfPUIso);
    outTree->Branch("ele1_convVeto", &ele1_convVeto);
    
    // Output branches - Electron 2
    Float_t ele2_pt, ele2_eta, ele2_phi, ele2_mass;
    Int_t ele2_charge;
    Float_t ele2_dxy, ele2_dz, ele2_dxyErr, ele2_dzErr;
    Int_t ele2_nValidHits, ele2_nMissingHits;
    Float_t ele2_scEta, ele2_scPhi, ele2_scEn;
    Float_t ele2_sigmaIEtaIEta, ele2_hOverE, ele2_r9;
    Float_t ele2_deltaEtaIn, ele2_deltaPhiIn, ele2_eOverP;
    Float_t ele2_pfChIso, ele2_pfNeuIso, ele2_pfPhoIso, ele2_pfPUIso;
    Short_t ele2_convVeto;
    
    outTree->Branch("ele2_pt", &ele2_pt);
    outTree->Branch("ele2_eta", &ele2_eta);
    outTree->Branch("ele2_phi", &ele2_phi);
    outTree->Branch("ele2_mass", &ele2_mass);
    outTree->Branch("ele2_charge", &ele2_charge);
    outTree->Branch("ele2_dxy", &ele2_dxy);
    outTree->Branch("ele2_dz", &ele2_dz);
    outTree->Branch("ele2_dxyErr", &ele2_dxyErr);
    outTree->Branch("ele2_dzErr", &ele2_dzErr);
    outTree->Branch("ele2_nValidHits", &ele2_nValidHits);
    outTree->Branch("ele2_nMissingHits", &ele2_nMissingHits);
    outTree->Branch("ele2_scEta", &ele2_scEta);
    outTree->Branch("ele2_scPhi", &ele2_scPhi);
    outTree->Branch("ele2_scEn", &ele2_scEn);
    outTree->Branch("ele2_sigmaIEtaIEta", &ele2_sigmaIEtaIEta);
    outTree->Branch("ele2_hOverE", &ele2_hOverE);
    outTree->Branch("ele2_r9", &ele2_r9);
    outTree->Branch("ele2_deltaEtaIn", &ele2_deltaEtaIn);
    outTree->Branch("ele2_deltaPhiIn", &ele2_deltaPhiIn);
    outTree->Branch("ele2_eOverP", &ele2_eOverP);
    outTree->Branch("ele2_pfChIso", &ele2_pfChIso);
    outTree->Branch("ele2_pfNeuIso", &ele2_pfNeuIso);
    outTree->Branch("ele2_pfPhoIso", &ele2_pfPhoIso);
    outTree->Branch("ele2_pfPUIso", &ele2_pfPUIso);
    outTree->Branch("ele2_convVeto", &ele2_convVeto);
    
    // Output branches - Gen-matching info (just copy from input)
    Short_t o_Reco_ee_isGenMatched;
    Short_t o_Reco_ee_matchedGenIdx;
    Float_t o_Reco_ee_gen_ele1_dR;
    Float_t o_Reco_ee_gen_ele2_dR;
    
    // Output branches - Matched Gen info
    Float_t gen_ee_mass, gen_ee_pt, gen_ee_eta, gen_ee_phi;
    Int_t gen_ee_charge;
    Float_t gen_ele1_pt, gen_ele1_eta, gen_ele1_phi;
    Int_t gen_ele1_pdgId, gen_ele1_motherId;
    Float_t gen_ele2_pt, gen_ele2_eta, gen_ele2_phi;
    Int_t gen_ele2_pdgId, gen_ele2_motherId;
    
    if (hasGenMatching) {
        outTree->Branch("Reco_ee_isGenMatched", &o_Reco_ee_isGenMatched);
        outTree->Branch("Reco_ee_matchedGenIdx", &o_Reco_ee_matchedGenIdx);
        outTree->Branch("Reco_ee_gen_ele1_dR", &o_Reco_ee_gen_ele1_dR);
        outTree->Branch("Reco_ee_gen_ele2_dR", &o_Reco_ee_gen_ele2_dR);
        outTree->Branch("gen_ee_mass", &gen_ee_mass);
        outTree->Branch("gen_ee_pt", &gen_ee_pt);
        outTree->Branch("gen_ee_eta", &gen_ee_eta);
        outTree->Branch("gen_ee_phi", &gen_ee_phi);
        outTree->Branch("gen_ee_charge", &gen_ee_charge);
        outTree->Branch("gen_ele1_pt", &gen_ele1_pt);
        outTree->Branch("gen_ele1_eta", &gen_ele1_eta);
        outTree->Branch("gen_ele1_phi", &gen_ele1_phi);
        outTree->Branch("gen_ele1_pdgId", &gen_ele1_pdgId);
        outTree->Branch("gen_ele1_motherId", &gen_ele1_motherId);
        outTree->Branch("gen_ele2_pt", &gen_ele2_pt);
        outTree->Branch("gen_ele2_eta", &gen_ele2_eta);
        outTree->Branch("gen_ele2_phi", &gen_ele2_phi);
        outTree->Branch("gen_ele2_pdgId", &gen_ele2_pdgId);
        outTree->Branch("gen_ele2_motherId", &gen_ele2_motherId);
    }
    
    // Loop over events
    Long64_t nEntries = tree->GetEntries();
    Long64_t nDielectrons = 0;
    
    for (Long64_t i = 0; i < nEntries; i++) {
        if (i % 10000 == 0) {
            std::cout << "Processing event " << i << " / " << nEntries << std::endl;
        }
        
        tree->GetEntry(i);
        
        // Copy event info
        o_runNb = run;
        o_lumiSection = lumi;
        o_eventNb = event;
        o_nPV = nPV;
        o_pvX = pvX;
        o_pvY = pvY;
        o_pvZ = pvZ;
        o_centrality = centrality;
        o_centralityBin = centralityBin;
        
        // Loop over dielectron candidates
        for (Int_t iee = 0; iee < Reco_ee_size; iee++) {
            Short_t idx1 = Reco_ee_ele1Idx[iee];
            Short_t idx2 = Reco_ee_ele2Idx[iee];
            
            // Sanity check
            if (idx1 < 0 || idx1 >= Reco_ele_size || idx2 < 0 || idx2 >= Reco_ele_size) {
                std::cerr << "WARNING: Invalid electron index! Event " << i 
                          << " ee " << iee << " idx1=" << idx1 << " idx2=" << idx2 
                          << " ele_size=" << Reco_ele_size << std::endl;
                continue;
            }
            
            // Fill dielectron info
            ee_mass = (*Reco_ee_mass)[iee];
            ee_pt = (*Reco_ee_pt)[iee];
            ee_eta = (*Reco_ee_eta)[iee];
            ee_phi = (*Reco_ee_phi)[iee];
            ee_y = (*Reco_ee_y)[iee];
            ee_vProb = Reco_ee_vProb[iee];
            ee_vChi2 = 0;  // Not stored
            ee_charge = Reco_ee_charge[iee];
            ee_trigBits = Reco_ee_trigBits[iee];
            
            // Fill electron 1 info
            ele1_pt = (*Reco_ele_pt)[idx1];
            ele1_eta = (*Reco_ele_eta)[idx1];
            ele1_phi = (*Reco_ele_phi)[idx1];
            ele1_mass = (*Reco_ele_mass)[idx1];
            ele1_charge = Reco_ele_charge[idx1];
            ele1_dxy = Reco_ele_dxy[idx1];
            ele1_dz = Reco_ele_dz[idx1];
            ele1_dxyErr = Reco_ele_dxyErr[idx1];
            ele1_dzErr = Reco_ele_dzErr[idx1];
            ele1_nValidHits = Reco_ele_nValidHits[idx1];
            ele1_nMissingHits = Reco_ele_nMissingHits[idx1];
            ele1_scEta = Reco_ele_scEta[idx1];
            ele1_scPhi = Reco_ele_scPhi[idx1];
            ele1_scEn = Reco_ele_scEn[idx1];
            ele1_sigmaIEtaIEta = Reco_ele_sigmaIEtaIEta[idx1];
            ele1_hOverE = Reco_ele_hOverE[idx1];
            ele1_r9 = Reco_ele_r9[idx1];
            ele1_deltaEtaIn = Reco_ele_deltaEtaIn[idx1];
            ele1_deltaPhiIn = Reco_ele_deltaPhiIn[idx1];
            ele1_eOverP = Reco_ele_eOverP[idx1];
            ele1_pfChIso = Reco_ele_pfChIso[idx1];
            ele1_pfNeuIso = Reco_ele_pfNeuIso[idx1];
            ele1_pfPhoIso = Reco_ele_pfPhoIso[idx1];
            ele1_pfPUIso = Reco_ele_pfPUIso[idx1];
            ele1_convVeto = Reco_ele_convVeto[idx1];
            
            // Fill electron 2 info
            ele2_pt = (*Reco_ele_pt)[idx2];
            ele2_eta = (*Reco_ele_eta)[idx2];
            ele2_phi = (*Reco_ele_phi)[idx2];
            ele2_mass = (*Reco_ele_mass)[idx2];
            ele2_charge = Reco_ele_charge[idx2];
            ele2_dxy = Reco_ele_dxy[idx2];
            ele2_dz = Reco_ele_dz[idx2];
            ele2_dxyErr = Reco_ele_dxyErr[idx2];
            ele2_dzErr = Reco_ele_dzErr[idx2];
            ele2_nValidHits = Reco_ele_nValidHits[idx2];
            ele2_nMissingHits = Reco_ele_nMissingHits[idx2];
            ele2_scEta = Reco_ele_scEta[idx2];
            ele2_scPhi = Reco_ele_scPhi[idx2];
            ele2_scEn = Reco_ele_scEn[idx2];
            ele2_sigmaIEtaIEta = Reco_ele_sigmaIEtaIEta[idx2];
            ele2_hOverE = Reco_ele_hOverE[idx2];
            ele2_r9 = Reco_ele_r9[idx2];
            ele2_deltaEtaIn = Reco_ele_deltaEtaIn[idx2];
            ele2_deltaPhiIn = Reco_ele_deltaPhiIn[idx2];
            ele2_eOverP = Reco_ele_eOverP[idx2];
            ele2_pfChIso = Reco_ele_pfChIso[idx2];
            ele2_pfNeuIso = Reco_ele_pfNeuIso[idx2];
            ele2_pfPhoIso = Reco_ele_pfPhoIso[idx2];
            ele2_pfPUIso = Reco_ele_pfPUIso[idx2];
            ele2_convVeto = Reco_ele_convVeto[idx2];
            
            // Copy gen-matching info (already computed in analyzer)
            o_Reco_ee_isGenMatched = 0;
            o_Reco_ee_matchedGenIdx = -1;
            o_Reco_ee_gen_ele1_dR = 999.9;
            o_Reco_ee_gen_ele2_dR = 999.9;
            gen_ee_mass = -999.9;
            gen_ee_pt = -999.9;
            gen_ee_eta = -999.9;
            gen_ee_phi = -999.9;
            gen_ee_charge = -999;
            gen_ele1_pt = -999.9;
            gen_ele1_eta = -999.9;
            gen_ele1_phi = -999.9;
            gen_ele1_pdgId = 0;
            gen_ele1_motherId = 0;
            gen_ele2_pt = -999.9;
            gen_ele2_eta = -999.9;
            gen_ele2_phi = -999.9;
            gen_ele2_pdgId = 0;
            gen_ele2_motherId = 0;
            
            if (hasGenMatching) {
                // Copy matching results from input tree
                o_Reco_ee_isGenMatched = Reco_ee_isGenMatched[iee];
                o_Reco_ee_matchedGenIdx = Reco_ee_matchedGenIdx[iee];
                o_Reco_ee_gen_ele1_dR = Reco_ee_gen_ele1_dR[iee];
                o_Reco_ee_gen_ele2_dR = Reco_ee_gen_ele2_dR[iee];
                
                // If matched, retrieve the Gen info
                if (o_Reco_ee_isGenMatched == 1 && o_Reco_ee_matchedGenIdx >= 0 && 
                    o_Reco_ee_matchedGenIdx < Gen_ee_size) {
                    Int_t igen = o_Reco_ee_matchedGenIdx;
                    
                    gen_ee_mass = (*Gen_ee_mass)[igen];
                    gen_ee_pt = (*Gen_ee_pt)[igen];
                    gen_ee_eta = (*Gen_ee_eta)[igen];
                    gen_ee_phi = (*Gen_ee_phi)[igen];
                    gen_ee_charge = Gen_ee_charge[igen];
                    
                    Short_t genEle1Idx = Gen_ee_ele1Idx[igen];
                    Short_t genEle2Idx = Gen_ee_ele2Idx[igen];
                    
                    if (genEle1Idx >= 0 && genEle1Idx < Gen_ele_size) {
                        gen_ele1_pt = (*Gen_ele_pt)[genEle1Idx];
                        gen_ele1_eta = (*Gen_ele_eta)[genEle1Idx];
                        gen_ele1_phi = (*Gen_ele_phi)[genEle1Idx];
                        gen_ele1_pdgId = Gen_ele_pdgId[genEle1Idx];
                        gen_ele1_motherId = Gen_ele_motherId[genEle1Idx];
                    }
                    
                    if (genEle2Idx >= 0 && genEle2Idx < Gen_ele_size) {
                        gen_ele2_pt = (*Gen_ele_pt)[genEle2Idx];
                        gen_ele2_eta = (*Gen_ele_eta)[genEle2Idx];
                        gen_ele2_phi = (*Gen_ele_phi)[genEle2Idx];
                        gen_ele2_pdgId = Gen_ele_pdgId[genEle2Idx];
                        gen_ele2_motherId = Gen_ele_motherId[genEle2Idx];
                    }
                }
            }
            
            outTree->Fill();
            nDielectrons++;
        }
    }
    
    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "Input events: " << nEntries << std::endl;
    std::cout << "Output dielectrons: " << nDielectrons << std::endl;
    std::cout << "Average dielectrons/event: " << (float)nDielectrons/nEntries << std::endl;
    
    fout->cd();
    outTree->Write();
    fout->Close();
    fin->Close();
    
    std::cout << "\nFlattened tree written to: " << outputFile << std::endl;
    std::cout << "Usage example:" << std::endl;
    std::cout << "  root -l " << outputFile << std::endl;
    std::cout << "  dielectronTree->Draw(\"ee_mass\", \"ee_charge==0 && ee_vProb>0.01\")" << std::endl;
}
