// HiForestBranches.h - Branch definitions for HiForest trees
// For unified B meson analysis (bTreeEE, bTreeMuMu, hionia, jets, events)
//
// Usage:
//   #include "HiForestBranches.h"
//   EventBranches evt;
//   evt.SetBranchAddresses(tree);
//
// Author: Auto-generated from HiForestOO_UnifiedBMeson structure

#ifndef HIFORESTBRANCHES_H
#define HIFORESTBRANCHES_H

#include <TTree.h>
#include <TClonesArray.h>
#include <vector>

// Maximum array sizes
const int kMaxQQ = 100;
const int kMaxMu = 100;
const int kMaxEle = 100;
const int kMaxEE = 100;
const int kMaxJet = 500;

// ============================================================================
// Event Tree (hiEvtAnalyzer/HiTree)
// ============================================================================
struct EventBranches {
    // Event ID
    UInt_t    run;
    ULong64_t evt;
    UInt_t    lumi;
    
    // Primary vertex
    Float_t   vx, vy, vz;
    
    // Centrality
    Int_t     hiBin;
    Float_t   hiHF, hiHFplus, hiHFminus;
    Float_t   hiHFECut, hiHFECutPlus, hiHFECutMinus;
    Float_t   hiHFplusEta4, hiHFminusEta4;
    
    // ZDC
    Float_t   hiZDC, hiZDCplus, hiZDCminus;
    
    // HF hit
    Float_t   hiHFhit, hiHFhitPlus, hiHFhitMinus;
    
    // Calorimeter
    Float_t   hiET, hiEE, hiEB, hiEEplus, hiEEminus;
    
    // Pixel
    Int_t     hiNpix, hiNpixPlus, hiNpixMinus;
    Int_t     hiNpixelTracks, hiNpixelTracksPlus, hiNpixelTracksMinus;
    
    // Tracks
    Int_t     hiNtracks, hiNtracksPtCut, hiNtracksEtaCut, hiNtracksEtaPtCut;
    
    // PF HF
    Float_t   hiHF_pf, hiHFE_pf, hiHF_pfha, hiHF_pfem;
    Float_t   hiHFPlus_pf, hiHFEPlus_pf, hiHFPlus_pfha, hiHFPlus_pfem;
    Float_t   hiHFMinus_pf, hiHFEMinus_pf, hiHFMinus_pfha, hiHFMinus_pfem;
    Float_t   hiHF_pfle1, hiHFPlus_pfle1, hiHFMinus_pfle1;
    Float_t   hiHF_pfle2, hiHFPlus_pfle2, hiHFMinus_pfle2;
    Float_t   hiHF_pfle3, hiHFPlus_pfle3, hiHFMinus_pfle3;
    Int_t     nCountsHF_pf, nCountsHFPlus_pf, nCountsHFMinus_pf;
    Int_t     numMinHFTower2, numMinHFTower3, numMinHFTower4, numMinHFTower5;
    
    void SetBranchAddresses(TTree* t) {
        t->SetBranchAddress("run", &run);
        t->SetBranchAddress("evt", &evt);
        t->SetBranchAddress("lumi", &lumi);
        t->SetBranchAddress("vx", &vx);
        t->SetBranchAddress("vy", &vy);
        t->SetBranchAddress("vz", &vz);
        t->SetBranchAddress("hiBin", &hiBin);
        t->SetBranchAddress("hiHF", &hiHF);
        t->SetBranchAddress("hiHFplus", &hiHFplus);
        t->SetBranchAddress("hiHFminus", &hiHFminus);
        t->SetBranchAddress("hiHFECut", &hiHFECut);
        t->SetBranchAddress("hiHFECutPlus", &hiHFECutPlus);
        t->SetBranchAddress("hiHFECutMinus", &hiHFECutMinus);
        t->SetBranchAddress("hiHFplusEta4", &hiHFplusEta4);
        t->SetBranchAddress("hiHFminusEta4", &hiHFminusEta4);
        t->SetBranchAddress("hiZDC", &hiZDC);
        t->SetBranchAddress("hiZDCplus", &hiZDCplus);
        t->SetBranchAddress("hiZDCminus", &hiZDCminus);
        t->SetBranchAddress("hiHFhit", &hiHFhit);
        t->SetBranchAddress("hiHFhitPlus", &hiHFhitPlus);
        t->SetBranchAddress("hiHFhitMinus", &hiHFhitMinus);
        t->SetBranchAddress("hiET", &hiET);
        t->SetBranchAddress("hiEE", &hiEE);
        t->SetBranchAddress("hiEB", &hiEB);
        t->SetBranchAddress("hiEEplus", &hiEEplus);
        t->SetBranchAddress("hiEEminus", &hiEEminus);
        t->SetBranchAddress("hiNpix", &hiNpix);
        t->SetBranchAddress("hiNpixPlus", &hiNpixPlus);
        t->SetBranchAddress("hiNpixMinus", &hiNpixMinus);
        t->SetBranchAddress("hiNpixelTracks", &hiNpixelTracks);
        t->SetBranchAddress("hiNpixelTracksPlus", &hiNpixelTracksPlus);
        t->SetBranchAddress("hiNpixelTracksMinus", &hiNpixelTracksMinus);
        t->SetBranchAddress("hiNtracks", &hiNtracks);
        t->SetBranchAddress("hiNtracksPtCut", &hiNtracksPtCut);
        t->SetBranchAddress("hiNtracksEtaCut", &hiNtracksEtaCut);
        t->SetBranchAddress("hiNtracksEtaPtCut", &hiNtracksEtaPtCut);
        t->SetBranchAddress("hiHF_pf", &hiHF_pf);
        t->SetBranchAddress("hiHFE_pf", &hiHFE_pf);
        t->SetBranchAddress("hiHF_pfha", &hiHF_pfha);
        t->SetBranchAddress("hiHF_pfem", &hiHF_pfem);
        t->SetBranchAddress("hiHFPlus_pf", &hiHFPlus_pf);
        t->SetBranchAddress("hiHFEPlus_pf", &hiHFEPlus_pf);
        t->SetBranchAddress("hiHFPlus_pfha", &hiHFPlus_pfha);
        t->SetBranchAddress("hiHFPlus_pfem", &hiHFPlus_pfem);
        t->SetBranchAddress("hiHFMinus_pf", &hiHFMinus_pf);
        t->SetBranchAddress("hiHFEMinus_pf", &hiHFEMinus_pf);
        t->SetBranchAddress("hiHFMinus_pfha", &hiHFMinus_pfha);
        t->SetBranchAddress("hiHFMinus_pfem", &hiHFMinus_pfem);
        t->SetBranchAddress("hiHF_pfle1", &hiHF_pfle1);
        t->SetBranchAddress("hiHFPlus_pfle1", &hiHFPlus_pfle1);
        t->SetBranchAddress("hiHFMinus_pfle1", &hiHFMinus_pfle1);
        t->SetBranchAddress("hiHF_pfle2", &hiHF_pfle2);
        t->SetBranchAddress("hiHFPlus_pfle2", &hiHFPlus_pfle2);
        t->SetBranchAddress("hiHFMinus_pfle2", &hiHFMinus_pfle2);
        t->SetBranchAddress("hiHF_pfle3", &hiHF_pfle3);
        t->SetBranchAddress("hiHFPlus_pfle3", &hiHFPlus_pfle3);
        t->SetBranchAddress("hiHFMinus_pfle3", &hiHFMinus_pfle3);
        t->SetBranchAddress("nCountsHF_pf", &nCountsHF_pf);
        t->SetBranchAddress("nCountsHFPlus_pf", &nCountsHFPlus_pf);
        t->SetBranchAddress("nCountsHFMinus_pf", &nCountsHFMinus_pf);
        t->SetBranchAddress("numMinHFTower2", &numMinHFTower2);
        t->SetBranchAddress("numMinHFTower3", &numMinHFTower3);
        t->SetBranchAddress("numMinHFTower4", &numMinHFTower4);
        t->SetBranchAddress("numMinHFTower5", &numMinHFTower5);
    }
};

// ============================================================================
// B Meson Tree (bTreeEE/bTree or bTreeMuMu/bTree)
// ============================================================================
struct BTreeBranches {
    // Event ID
    UInt_t    run;
    UInt_t    lumi;
    UInt_t    event;
    
    // Primary vertex
    Short_t   nPV;
    Float_t   pvX, pvY, pvZ;
    
    // Event info
    UInt_t    nCand;
    Short_t   centrality;
    Int_t     Ntrkoffline;
    
    // B meson kinematics (vectors)
    std::vector<float>* B_mass = nullptr;
    std::vector<float>* B_pt = nullptr;
    std::vector<float>* B_eta = nullptr;
    std::vector<float>* B_phi = nullptr;
    std::vector<float>* B_y = nullptr;
    std::vector<int>*   B_charge = nullptr;
    std::vector<int>*   B_decayId = nullptr;
    std::vector<int>*   B_nTracks = nullptr;
    std::vector<int>*   B_hasDau2 = nullptr;
    
    // B meson vertex
    std::vector<float>* B_vtxX = nullptr;
    std::vector<float>* B_vtxY = nullptr;
    std::vector<float>* B_vtxZ = nullptr;
    std::vector<float>* B_vtxChi2 = nullptr;
    std::vector<float>* B_vtxNdof = nullptr;
    std::vector<float>* B_vtxProb = nullptr;
    
    // B meson decay length
    std::vector<float>* B_lxy = nullptr;
    std::vector<float>* B_lxySig = nullptr;
    std::vector<float>* B_l3D = nullptr;
    std::vector<float>* B_l3DSig = nullptr;
    std::vector<float>* B_cosAlpha = nullptr;
    
    // J/psi
    std::vector<int>*   jpsiIdx = nullptr;
    std::vector<float>* Jpsi_mass = nullptr;
    std::vector<float>* Jpsi_pt = nullptr;
    std::vector<float>* Jpsi_eta = nullptr;
    std::vector<float>* Jpsi_phi = nullptr;
    std::vector<float>* Jpsi_y = nullptr;
    std::vector<float>* resMass = nullptr;
    
    // Track 1 (kaon or first track)
    std::vector<float>* trk1_pt = nullptr;
    std::vector<float>* trk1_eta = nullptr;
    std::vector<float>* trk1_phi = nullptr;
    std::vector<int>*   trk1_charge = nullptr;
    std::vector<float>* trk1_massHypo = nullptr;
    std::vector<int>*   trk1_idx = nullptr;
    
    // Track 2 (second track, if present)
    std::vector<float>* trk2_pt = nullptr;
    std::vector<float>* trk2_eta = nullptr;
    std::vector<float>* trk2_phi = nullptr;
    std::vector<int>*   trk2_charge = nullptr;
    std::vector<float>* trk2_massHypo = nullptr;
    std::vector<int>*   trk2_idx = nullptr;
    
    void SetBranchAddresses(TTree* t) {
        t->SetBranchAddress("run", &run);
        t->SetBranchAddress("lumi", &lumi);
        t->SetBranchAddress("event", &event);
        t->SetBranchAddress("nPV", &nPV);
        t->SetBranchAddress("pvX", &pvX);
        t->SetBranchAddress("pvY", &pvY);
        t->SetBranchAddress("pvZ", &pvZ);
        t->SetBranchAddress("nCand", &nCand);
        t->SetBranchAddress("centrality", &centrality);
        t->SetBranchAddress("Ntrkoffline", &Ntrkoffline);
        t->SetBranchAddress("B_mass", &B_mass);
        t->SetBranchAddress("B_pt", &B_pt);
        t->SetBranchAddress("B_eta", &B_eta);
        t->SetBranchAddress("B_phi", &B_phi);
        t->SetBranchAddress("B_y", &B_y);
        t->SetBranchAddress("B_charge", &B_charge);
        t->SetBranchAddress("B_decayId", &B_decayId);
        t->SetBranchAddress("B_nTracks", &B_nTracks);
        t->SetBranchAddress("B_hasDau2", &B_hasDau2);
        t->SetBranchAddress("B_vtxX", &B_vtxX);
        t->SetBranchAddress("B_vtxY", &B_vtxY);
        t->SetBranchAddress("B_vtxZ", &B_vtxZ);
        t->SetBranchAddress("B_vtxChi2", &B_vtxChi2);
        t->SetBranchAddress("B_vtxNdof", &B_vtxNdof);
        t->SetBranchAddress("B_vtxProb", &B_vtxProb);
        t->SetBranchAddress("B_lxy", &B_lxy);
        t->SetBranchAddress("B_lxySig", &B_lxySig);
        t->SetBranchAddress("B_l3D", &B_l3D);
        t->SetBranchAddress("B_l3DSig", &B_l3DSig);
        t->SetBranchAddress("B_cosAlpha", &B_cosAlpha);
        t->SetBranchAddress("jpsiIdx", &jpsiIdx);
        t->SetBranchAddress("Jpsi_mass", &Jpsi_mass);
        t->SetBranchAddress("Jpsi_pt", &Jpsi_pt);
        t->SetBranchAddress("Jpsi_eta", &Jpsi_eta);
        t->SetBranchAddress("Jpsi_phi", &Jpsi_phi);
        t->SetBranchAddress("Jpsi_y", &Jpsi_y);
        t->SetBranchAddress("resMass", &resMass);
        t->SetBranchAddress("trk1_pt", &trk1_pt);
        t->SetBranchAddress("trk1_eta", &trk1_eta);
        t->SetBranchAddress("trk1_phi", &trk1_phi);
        t->SetBranchAddress("trk1_charge", &trk1_charge);
        t->SetBranchAddress("trk1_massHypo", &trk1_massHypo);
        t->SetBranchAddress("trk1_idx", &trk1_idx);
        t->SetBranchAddress("trk2_pt", &trk2_pt);
        t->SetBranchAddress("trk2_eta", &trk2_eta);
        t->SetBranchAddress("trk2_phi", &trk2_phi);
        t->SetBranchAddress("trk2_charge", &trk2_charge);
        t->SetBranchAddress("trk2_massHypo", &trk2_massHypo);
        t->SetBranchAddress("trk2_idx", &trk2_idx);
    }
};

// ============================================================================
// Dimuon Tree (hionia/myTree)
// ============================================================================
struct DimuonBranches {
    // Event ID
    UInt_t    eventNb;
    UInt_t    runNb;
    UInt_t    LS;
    
    // Event info
    Float_t   zVtx;
    Short_t   nPV;
    Short_t   Ntracks;
    Int_t     trigPrescale[9];
    ULong64_t HLTriggers;
    
    // Dimuon (QQ)
    Short_t   Reco_QQ_size;
    Short_t   Reco_QQ_type[kMaxQQ];
    Short_t   Reco_QQ_sign[kMaxQQ];
    Short_t   Reco_QQ_collIdx[kMaxQQ];
    std::vector<float>* Reco_QQ_4mom_pt = nullptr;
    std::vector<float>* Reco_QQ_4mom_eta = nullptr;
    std::vector<float>* Reco_QQ_4mom_phi = nullptr;
    std::vector<float>* Reco_QQ_4mom_m = nullptr;
    Short_t   Reco_QQ_mupl_idx[kMaxQQ];
    Short_t   Reco_QQ_mumi_idx[kMaxQQ];
    ULong64_t Reco_QQ_trig[kMaxQQ];
    Float_t   Reco_QQ_ctau[kMaxQQ];
    Float_t   Reco_QQ_ctauErr[kMaxQQ];
    Float_t   Reco_QQ_cosAlpha[kMaxQQ];
    Float_t   Reco_QQ_ctau3D[kMaxQQ];
    Float_t   Reco_QQ_ctauErr3D[kMaxQQ];
    Float_t   Reco_QQ_cosAlpha3D[kMaxQQ];
    Float_t   Reco_QQ_VtxProb[kMaxQQ];
    Float_t   Reco_QQ_dca[kMaxQQ];
    TClonesArray* Reco_QQ_vtx = nullptr;
    
    // Single muon
    Short_t   Reco_mu_size;
    Short_t   Reco_mu_type[kMaxMu];
    Short_t   Reco_mu_charge[kMaxMu];
    std::vector<float>* Reco_mu_4mom_pt = nullptr;
    std::vector<float>* Reco_mu_4mom_eta = nullptr;
    std::vector<float>* Reco_mu_4mom_phi = nullptr;
    std::vector<float>* Reco_mu_4mom_m = nullptr;
    std::vector<float>* Reco_mu_L1_4mom_pt = nullptr;
    std::vector<float>* Reco_mu_L1_4mom_eta = nullptr;
    std::vector<float>* Reco_mu_L1_4mom_phi = nullptr;
    std::vector<float>* Reco_mu_L1_4mom_m = nullptr;
    ULong64_t Reco_mu_trig[kMaxMu];
    Bool_t    Reco_mu_InTightAcc[kMaxMu];
    Bool_t    Reco_mu_InLooseAcc[kMaxMu];
    Bool_t    Reco_mu_highPurity[kMaxMu];
    Bool_t    Reco_mu_TMOneStaTight[kMaxMu];
    Bool_t    Reco_mu_isPF[kMaxMu];
    Bool_t    Reco_mu_isTracker[kMaxMu];
    Bool_t    Reco_mu_isGlobal[kMaxMu];
    Bool_t    Reco_mu_isSoftCutBased[kMaxMu];
    Float_t   Reco_mu_softMvaRun3Value[kMaxMu];
    Bool_t    Reco_mu_isHybridSoft[kMaxMu];
    Bool_t    Reco_mu_isMediumCutBased[kMaxMu];
    Bool_t    Reco_mu_isTightCutBased[kMaxMu];
    Int_t     Reco_mu_nPixValHits[kMaxMu];
    Int_t     Reco_mu_nMuValHits[kMaxMu];
    Int_t     Reco_mu_nTrkHits[kMaxMu];
    Float_t   Reco_mu_normChi2_inner[kMaxMu];
    Int_t     Reco_mu_nPixWMea[kMaxMu];
    Int_t     Reco_mu_nTrkWMea[kMaxMu];
    
    void SetBranchAddresses(TTree* t) {
        t->SetBranchAddress("eventNb", &eventNb);
        t->SetBranchAddress("runNb", &runNb);
        t->SetBranchAddress("LS", &LS);
        t->SetBranchAddress("zVtx", &zVtx);
        t->SetBranchAddress("nPV", &nPV);
        t->SetBranchAddress("Ntracks", &Ntracks);
        t->SetBranchAddress("trigPrescale", trigPrescale);
        t->SetBranchAddress("HLTriggers", &HLTriggers);
        t->SetBranchAddress("Reco_QQ_size", &Reco_QQ_size);
        t->SetBranchAddress("Reco_QQ_type", Reco_QQ_type);
        t->SetBranchAddress("Reco_QQ_sign", Reco_QQ_sign);
        t->SetBranchAddress("Reco_QQ_collIdx", Reco_QQ_collIdx);
        t->SetBranchAddress("Reco_QQ_4mom_pt", &Reco_QQ_4mom_pt);
        t->SetBranchAddress("Reco_QQ_4mom_eta", &Reco_QQ_4mom_eta);
        t->SetBranchAddress("Reco_QQ_4mom_phi", &Reco_QQ_4mom_phi);
        t->SetBranchAddress("Reco_QQ_4mom_m", &Reco_QQ_4mom_m);
        t->SetBranchAddress("Reco_QQ_mupl_idx", Reco_QQ_mupl_idx);
        t->SetBranchAddress("Reco_QQ_mumi_idx", Reco_QQ_mumi_idx);
        t->SetBranchAddress("Reco_QQ_trig", Reco_QQ_trig);
        t->SetBranchAddress("Reco_QQ_ctau", Reco_QQ_ctau);
        t->SetBranchAddress("Reco_QQ_ctauErr", Reco_QQ_ctauErr);
        t->SetBranchAddress("Reco_QQ_cosAlpha", Reco_QQ_cosAlpha);
        t->SetBranchAddress("Reco_QQ_ctau3D", Reco_QQ_ctau3D);
        t->SetBranchAddress("Reco_QQ_ctauErr3D", Reco_QQ_ctauErr3D);
        t->SetBranchAddress("Reco_QQ_cosAlpha3D", Reco_QQ_cosAlpha3D);
        t->SetBranchAddress("Reco_QQ_VtxProb", Reco_QQ_VtxProb);
        t->SetBranchAddress("Reco_QQ_dca", Reco_QQ_dca);
        t->SetBranchAddress("Reco_QQ_vtx", &Reco_QQ_vtx);
        t->SetBranchAddress("Reco_mu_size", &Reco_mu_size);
        t->SetBranchAddress("Reco_mu_type", Reco_mu_type);
        t->SetBranchAddress("Reco_mu_charge", Reco_mu_charge);
        t->SetBranchAddress("Reco_mu_4mom_pt", &Reco_mu_4mom_pt);
        t->SetBranchAddress("Reco_mu_4mom_eta", &Reco_mu_4mom_eta);
        t->SetBranchAddress("Reco_mu_4mom_phi", &Reco_mu_4mom_phi);
        t->SetBranchAddress("Reco_mu_4mom_m", &Reco_mu_4mom_m);
        t->SetBranchAddress("Reco_mu_L1_4mom_pt", &Reco_mu_L1_4mom_pt);
        t->SetBranchAddress("Reco_mu_L1_4mom_eta", &Reco_mu_L1_4mom_eta);
        t->SetBranchAddress("Reco_mu_L1_4mom_phi", &Reco_mu_L1_4mom_phi);
        t->SetBranchAddress("Reco_mu_L1_4mom_m", &Reco_mu_L1_4mom_m);
        t->SetBranchAddress("Reco_mu_trig", Reco_mu_trig);
        t->SetBranchAddress("Reco_mu_InTightAcc", Reco_mu_InTightAcc);
        t->SetBranchAddress("Reco_mu_InLooseAcc", Reco_mu_InLooseAcc);
        t->SetBranchAddress("Reco_mu_highPurity", Reco_mu_highPurity);
        t->SetBranchAddress("Reco_mu_TMOneStaTight", Reco_mu_TMOneStaTight);
        t->SetBranchAddress("Reco_mu_isPF", Reco_mu_isPF);
        t->SetBranchAddress("Reco_mu_isTracker", Reco_mu_isTracker);
        t->SetBranchAddress("Reco_mu_isGlobal", Reco_mu_isGlobal);
        t->SetBranchAddress("Reco_mu_isSoftCutBased", Reco_mu_isSoftCutBased);
        t->SetBranchAddress("Reco_mu_softMvaRun3Value", Reco_mu_softMvaRun3Value);
        t->SetBranchAddress("Reco_mu_isHybridSoft", Reco_mu_isHybridSoft);
        t->SetBranchAddress("Reco_mu_isMediumCutBased", Reco_mu_isMediumCutBased);
        t->SetBranchAddress("Reco_mu_isTightCutBased", Reco_mu_isTightCutBased);
        t->SetBranchAddress("Reco_mu_nPixValHits", Reco_mu_nPixValHits);
        t->SetBranchAddress("Reco_mu_nMuValHits", Reco_mu_nMuValHits);
        t->SetBranchAddress("Reco_mu_nTrkHits", Reco_mu_nTrkHits);
        t->SetBranchAddress("Reco_mu_normChi2_inner", Reco_mu_normChi2_inner);
        t->SetBranchAddress("Reco_mu_nPixWMea", Reco_mu_nPixWMea);
        t->SetBranchAddress("Reco_mu_nTrkWMea", Reco_mu_nTrkWMea);
    }
};

// ============================================================================
// Dielectron Tree (hioniaElectrons/eleTree)
// ============================================================================
struct DielectronBranches {
    // Event ID
    UInt_t    run;
    UInt_t    lumi;
    ULong64_t event;
    
    // Primary vertex
    Int_t     nPV;
    Float_t   pvX, pvY, pvZ;
    
    // Event info
    Float_t   centrality;
    Int_t     centralityBin;
    Float_t   evtPlane;
    ULong64_t triggerBits;
    
    // Single electrons
    Int_t     Reco_ele_size;
    std::vector<float>* Reco_ele_pt = nullptr;
    std::vector<float>* Reco_ele_eta = nullptr;
    std::vector<float>* Reco_ele_phi = nullptr;
    std::vector<float>* Reco_ele_y = nullptr;
    std::vector<float>* Reco_ele_mass = nullptr;
    Short_t   Reco_ele_charge[kMaxEle];
    Short_t   Reco_ele_isEB[kMaxEle];
    Float_t   Reco_ele_ptErr[kMaxEle];
    Float_t   Reco_ele_energy[kMaxEle];
    Float_t   Reco_ele_corrEnergy[kMaxEle];
    Float_t   Reco_ele_corrEnergyErr[kMaxEle];
    Float_t   Reco_ele_EcalEnergy[kMaxEle];
    Float_t   Reco_ele_trackPt[kMaxEle];
    Float_t   Reco_ele_trackEta[kMaxEle];
    Float_t   Reco_ele_trackPhi[kMaxEle];
    Float_t   Reco_ele_trackChi2[kMaxEle];
    Float_t   Reco_ele_trackNdof[kMaxEle];
    Float_t   Reco_ele_trackNormalizedChi2[kMaxEle];
    Short_t   Reco_ele_nValidHits[kMaxEle];
    Short_t   Reco_ele_nMissingHits[kMaxEle];
    Short_t   Reco_ele_trackerLayers[kMaxEle];
    Short_t   Reco_ele_pixelLayers[kMaxEle];
    Float_t   Reco_ele_dxy[kMaxEle];
    Float_t   Reco_ele_dxyErr[kMaxEle];
    Float_t   Reco_ele_dz[kMaxEle];
    Float_t   Reco_ele_dzErr[kMaxEle];
    Float_t   Reco_ele_scEta[kMaxEle];
    Float_t   Reco_ele_scPhi[kMaxEle];
    Float_t   Reco_ele_scEn[kMaxEle];
    Float_t   Reco_ele_scRawEn[kMaxEle];
    Float_t   Reco_ele_r9[kMaxEle];
    Float_t   Reco_ele_sigmaIEtaIEta[kMaxEle];
    Float_t   Reco_ele_sigmaIPhiIPhi[kMaxEle];
    Float_t   Reco_ele_hOverE[kMaxEle];
    Float_t   Reco_ele_fbrem[kMaxEle];
    Float_t   Reco_ele_eOverP[kMaxEle];
    Float_t   Reco_ele_eOverPInv[kMaxEle];
    Float_t   Reco_ele_deltaEtaIn[kMaxEle];
    Float_t   Reco_ele_deltaPhiIn[kMaxEle];
    Float_t   Reco_ele_sigmaEtaEta[kMaxEle];
    Float_t   Reco_ele_sigmaPhiPhi[kMaxEle];
    Float_t   Reco_ele_pfChIso[kMaxEle];
    Float_t   Reco_ele_pfNeuIso[kMaxEle];
    Float_t   Reco_ele_pfPhoIso[kMaxEle];
    Float_t   Reco_ele_pfPUIso[kMaxEle];
    Short_t   Reco_ele_convVeto[kMaxEle];
    ULong64_t Reco_ele_trigBits[kMaxEle];
    Float_t   Reco_ele_MVAIso[kMaxEle];
    Float_t   Reco_ele_MVAId[kMaxEle];
    Short_t   Reco_ele_MVAIsoWP95[kMaxEle];
    Short_t   Reco_ele_MVAIsoWP90[kMaxEle];
    Short_t   Reco_ele_MVAIsoWP85[kMaxEle];
    Short_t   Reco_ele_MVAIsoWP80[kMaxEle];
    Short_t   Reco_ele_MVAIdWP95[kMaxEle];
    Short_t   Reco_ele_MVAIdWP90[kMaxEle];
    Short_t   Reco_ele_MVAIdWP85[kMaxEle];
    Short_t   Reco_ele_MVAIdWP80[kMaxEle];
    Short_t   Reco_ele_CutIdWP95[kMaxEle];
    Short_t   Reco_ele_CutIdWP90[kMaxEle];
    Short_t   Reco_ele_CutIdWP80[kMaxEle];
    Short_t   Reco_ele_CutIdWP70[kMaxEle];
    Float_t   Reco_ele_rawPt[kMaxEle];
    Float_t   Reco_ele_rawEcalEnergy[kMaxEle];
    
    // Dielectrons
    Int_t     Reco_ee_size;
    std::vector<float>* Reco_ee_pt = nullptr;
    std::vector<float>* Reco_ee_eta = nullptr;
    std::vector<float>* Reco_ee_phi = nullptr;
    std::vector<float>* Reco_ee_y = nullptr;
    std::vector<float>* Reco_ee_mass = nullptr;
    Float_t   Reco_ee_vProb[kMaxEE];
    Float_t   Reco_ee_chi2[kMaxEE];
    Float_t   Reco_ee_ndf[kMaxEE];
    Short_t   Reco_ee_charge[kMaxEE];
    Short_t   Reco_ee_ele1Idx[kMaxEE];
    Short_t   Reco_ee_ele2Idx[kMaxEE];
    ULong64_t Reco_ee_trigBits[kMaxEE];
    Float_t   Reco_ee_ctau[kMaxEE];
    Float_t   Reco_ee_ctauErr[kMaxEE];
    Float_t   Reco_ee_ctau3D[kMaxEE];
    Float_t   Reco_ee_ctauErr3D[kMaxEE];
    Float_t   Reco_ee_cosAlpha[kMaxEE];
    Float_t   Reco_ee_cosAlpha3D[kMaxEE];
    Float_t   Reco_ee_dca[kMaxEE];
    Float_t   Reco_ee_vtxX[kMaxEE];
    Float_t   Reco_ee_vtxY[kMaxEE];
    Float_t   Reco_ee_vtxZ[kMaxEE];
    
    void SetBranchAddresses(TTree* t) {
        t->SetBranchAddress("run", &run);
        t->SetBranchAddress("lumi", &lumi);
        t->SetBranchAddress("event", &event);
        t->SetBranchAddress("nPV", &nPV);
        t->SetBranchAddress("pvX", &pvX);
        t->SetBranchAddress("pvY", &pvY);
        t->SetBranchAddress("pvZ", &pvZ);
        t->SetBranchAddress("centrality", &centrality);
        t->SetBranchAddress("centralityBin", &centralityBin);
        t->SetBranchAddress("evtPlane", &evtPlane);
        t->SetBranchAddress("triggerBits", &triggerBits);
        
        t->SetBranchAddress("Reco_ele_size", &Reco_ele_size);
        t->SetBranchAddress("Reco_ele_pt", &Reco_ele_pt);
        t->SetBranchAddress("Reco_ele_eta", &Reco_ele_eta);
        t->SetBranchAddress("Reco_ele_phi", &Reco_ele_phi);
        t->SetBranchAddress("Reco_ele_y", &Reco_ele_y);
        t->SetBranchAddress("Reco_ele_mass", &Reco_ele_mass);
        t->SetBranchAddress("Reco_ele_charge", Reco_ele_charge);
        t->SetBranchAddress("Reco_ele_isEB", Reco_ele_isEB);
        t->SetBranchAddress("Reco_ele_ptErr", Reco_ele_ptErr);
        t->SetBranchAddress("Reco_ele_energy", Reco_ele_energy);
        t->SetBranchAddress("Reco_ele_corrEnergy", Reco_ele_corrEnergy);
        t->SetBranchAddress("Reco_ele_corrEnergyErr", Reco_ele_corrEnergyErr);
        t->SetBranchAddress("Reco_ele_EcalEnergy", Reco_ele_EcalEnergy);
        t->SetBranchAddress("Reco_ele_trackPt", Reco_ele_trackPt);
        t->SetBranchAddress("Reco_ele_trackEta", Reco_ele_trackEta);
        t->SetBranchAddress("Reco_ele_trackPhi", Reco_ele_trackPhi);
        t->SetBranchAddress("Reco_ele_trackChi2", Reco_ele_trackChi2);
        t->SetBranchAddress("Reco_ele_trackNdof", Reco_ele_trackNdof);
        t->SetBranchAddress("Reco_ele_trackNormalizedChi2", Reco_ele_trackNormalizedChi2);
        t->SetBranchAddress("Reco_ele_nValidHits", Reco_ele_nValidHits);
        t->SetBranchAddress("Reco_ele_nMissingHits", Reco_ele_nMissingHits);
        t->SetBranchAddress("Reco_ele_trackerLayers", Reco_ele_trackerLayers);
        t->SetBranchAddress("Reco_ele_pixelLayers", Reco_ele_pixelLayers);
        t->SetBranchAddress("Reco_ele_dxy", Reco_ele_dxy);
        t->SetBranchAddress("Reco_ele_dxyErr", Reco_ele_dxyErr);
        t->SetBranchAddress("Reco_ele_dz", Reco_ele_dz);
        t->SetBranchAddress("Reco_ele_dzErr", Reco_ele_dzErr);
        t->SetBranchAddress("Reco_ele_scEta", Reco_ele_scEta);
        t->SetBranchAddress("Reco_ele_scPhi", Reco_ele_scPhi);
        t->SetBranchAddress("Reco_ele_scEn", Reco_ele_scEn);
        t->SetBranchAddress("Reco_ele_scRawEn", Reco_ele_scRawEn);
        t->SetBranchAddress("Reco_ele_r9", Reco_ele_r9);
        t->SetBranchAddress("Reco_ele_sigmaIEtaIEta", Reco_ele_sigmaIEtaIEta);
        t->SetBranchAddress("Reco_ele_sigmaIPhiIPhi", Reco_ele_sigmaIPhiIPhi);
        t->SetBranchAddress("Reco_ele_hOverE", Reco_ele_hOverE);
        t->SetBranchAddress("Reco_ele_fbrem", Reco_ele_fbrem);
        t->SetBranchAddress("Reco_ele_eOverP", Reco_ele_eOverP);
        t->SetBranchAddress("Reco_ele_eOverPInv", Reco_ele_eOverPInv);
        t->SetBranchAddress("Reco_ele_deltaEtaIn", Reco_ele_deltaEtaIn);
        t->SetBranchAddress("Reco_ele_deltaPhiIn", Reco_ele_deltaPhiIn);
        t->SetBranchAddress("Reco_ele_sigmaEtaEta", Reco_ele_sigmaEtaEta);
        t->SetBranchAddress("Reco_ele_sigmaPhiPhi", Reco_ele_sigmaPhiPhi);
        t->SetBranchAddress("Reco_ele_pfChIso", Reco_ele_pfChIso);
        t->SetBranchAddress("Reco_ele_pfNeuIso", Reco_ele_pfNeuIso);
        t->SetBranchAddress("Reco_ele_pfPhoIso", Reco_ele_pfPhoIso);
        t->SetBranchAddress("Reco_ele_pfPUIso", Reco_ele_pfPUIso);
        t->SetBranchAddress("Reco_ele_convVeto", Reco_ele_convVeto);
        t->SetBranchAddress("Reco_ele_trigBits", Reco_ele_trigBits);
        t->SetBranchAddress("Reco_ele_MVAIso", Reco_ele_MVAIso);
        t->SetBranchAddress("Reco_ele_MVAId", Reco_ele_MVAId);
        t->SetBranchAddress("Reco_ele_MVAIsoWP95", Reco_ele_MVAIsoWP95);
        t->SetBranchAddress("Reco_ele_MVAIsoWP90", Reco_ele_MVAIsoWP90);
        t->SetBranchAddress("Reco_ele_MVAIsoWP85", Reco_ele_MVAIsoWP85);
        t->SetBranchAddress("Reco_ele_MVAIsoWP80", Reco_ele_MVAIsoWP80);
        t->SetBranchAddress("Reco_ele_MVAIdWP95", Reco_ele_MVAIdWP95);
        t->SetBranchAddress("Reco_ele_MVAIdWP90", Reco_ele_MVAIdWP90);
        t->SetBranchAddress("Reco_ele_MVAIdWP85", Reco_ele_MVAIdWP85);
        t->SetBranchAddress("Reco_ele_MVAIdWP80", Reco_ele_MVAIdWP80);
        t->SetBranchAddress("Reco_ele_CutIdWP95", Reco_ele_CutIdWP95);
        t->SetBranchAddress("Reco_ele_CutIdWP90", Reco_ele_CutIdWP90);
        t->SetBranchAddress("Reco_ele_CutIdWP80", Reco_ele_CutIdWP80);
        t->SetBranchAddress("Reco_ele_CutIdWP70", Reco_ele_CutIdWP70);
        t->SetBranchAddress("Reco_ele_rawPt", Reco_ele_rawPt);
        t->SetBranchAddress("Reco_ele_rawEcalEnergy", Reco_ele_rawEcalEnergy);
        
        t->SetBranchAddress("Reco_ee_size", &Reco_ee_size);
        t->SetBranchAddress("Reco_ee_pt", &Reco_ee_pt);
        t->SetBranchAddress("Reco_ee_eta", &Reco_ee_eta);
        t->SetBranchAddress("Reco_ee_phi", &Reco_ee_phi);
        t->SetBranchAddress("Reco_ee_y", &Reco_ee_y);
        t->SetBranchAddress("Reco_ee_mass", &Reco_ee_mass);
        t->SetBranchAddress("Reco_ee_vProb", Reco_ee_vProb);
        t->SetBranchAddress("Reco_ee_chi2", Reco_ee_chi2);
        t->SetBranchAddress("Reco_ee_ndf", Reco_ee_ndf);
        t->SetBranchAddress("Reco_ee_charge", Reco_ee_charge);
        t->SetBranchAddress("Reco_ee_ele1Idx", Reco_ee_ele1Idx);
        t->SetBranchAddress("Reco_ee_ele2Idx", Reco_ee_ele2Idx);
        t->SetBranchAddress("Reco_ee_trigBits", Reco_ee_trigBits);
        t->SetBranchAddress("Reco_ee_ctau", Reco_ee_ctau);
        t->SetBranchAddress("Reco_ee_ctauErr", Reco_ee_ctauErr);
        t->SetBranchAddress("Reco_ee_ctau3D", Reco_ee_ctau3D);
        t->SetBranchAddress("Reco_ee_ctauErr3D", Reco_ee_ctauErr3D);
        t->SetBranchAddress("Reco_ee_cosAlpha", Reco_ee_cosAlpha);
        t->SetBranchAddress("Reco_ee_cosAlpha3D", Reco_ee_cosAlpha3D);
        t->SetBranchAddress("Reco_ee_dca", Reco_ee_dca);
        t->SetBranchAddress("Reco_ee_vtxX", Reco_ee_vtxX);
        t->SetBranchAddress("Reco_ee_vtxY", Reco_ee_vtxY);
        t->SetBranchAddress("Reco_ee_vtxZ", Reco_ee_vtxZ);
    }
};

// ============================================================================
// Jet Tree (akCs4PFJetAnalyzer/t)
// ============================================================================
struct JetBranches {
    // Event ID
    Int_t     run;
    Int_t     evt;
    Int_t     lumi;
    
    // Jet multiplicity
    Int_t     nref;
    Int_t     ncalo;
    
    // Jet kinematics (arrays)
    Float_t   rawpt[kMaxJet];
    Float_t   jtpt[kMaxJet];
    Float_t   jteta[kMaxJet];
    Float_t   jty[kMaxJet];
    Float_t   jtphi[kMaxJet];
    Float_t   jtpu[kMaxJet];
    Float_t   jtm[kMaxJet];
    Float_t   jtarea[kMaxJet];
    
    // Calo jets
    Float_t   calopt[kMaxJet];
    Float_t   caloeta[kMaxJet];
    Float_t   calophi[kMaxJet];
    
    // PF fractions
    Float_t   jtPfCHF[kMaxJet];
    Float_t   jtPfNHF[kMaxJet];
    Float_t   jtPfCEF[kMaxJet];
    Float_t   jtPfNEF[kMaxJet];
    Float_t   jtPfMUF[kMaxJet];
    Int_t     jtPfCHM[kMaxJet];
    Int_t     jtPfNHM[kMaxJet];
    Int_t     jtPfCEM[kMaxJet];
    Int_t     jtPfNEM[kMaxJet];
    Int_t     jtPfMUM[kMaxJet];
    
    // N-subjettiness
    Float_t   jttau1[kMaxJet];
    Float_t   jttau2[kMaxJet];
    Float_t   jttau3[kMaxJet];
    
    void SetBranchAddresses(TTree* t) {
        t->SetBranchAddress("run", &run);
        t->SetBranchAddress("evt", &evt);
        t->SetBranchAddress("lumi", &lumi);
        t->SetBranchAddress("nref", &nref);
        t->SetBranchAddress("ncalo", &ncalo);
        t->SetBranchAddress("rawpt", rawpt);
        t->SetBranchAddress("jtpt", jtpt);
        t->SetBranchAddress("jteta", jteta);
        t->SetBranchAddress("jty", jty);
        t->SetBranchAddress("jtphi", jtphi);
        t->SetBranchAddress("jtpu", jtpu);
        t->SetBranchAddress("jtm", jtm);
        t->SetBranchAddress("jtarea", jtarea);
        t->SetBranchAddress("calopt", calopt);
        t->SetBranchAddress("caloeta", caloeta);
        t->SetBranchAddress("calophi", calophi);
        t->SetBranchAddress("jtPfCHF", jtPfCHF);
        t->SetBranchAddress("jtPfNHF", jtPfNHF);
        t->SetBranchAddress("jtPfCEF", jtPfCEF);
        t->SetBranchAddress("jtPfNEF", jtPfNEF);
        t->SetBranchAddress("jtPfMUF", jtPfMUF);
        t->SetBranchAddress("jtPfCHM", jtPfCHM);
        t->SetBranchAddress("jtPfNHM", jtPfNHM);
        t->SetBranchAddress("jtPfCEM", jtPfCEM);
        t->SetBranchAddress("jtPfNEM", jtPfNEM);
        t->SetBranchAddress("jtPfMUM", jtPfMUM);
        t->SetBranchAddress("jttau1", jttau1);
        t->SetBranchAddress("jttau2", jttau2);
        t->SetBranchAddress("jttau3", jttau3);
    }
};

// ============================================================================
// FLATTENED STRUCTURES (one entry per candidate)
// ============================================================================

// Flattened B meson candidate
struct FlatBCandidate {
    // Event ID
    UInt_t    run;
    UInt_t    lumi;
    ULong64_t event;
    
    // Event info (from hiEvtAnalyzer)
    Int_t     hiBin;
    Float_t   hiHF;
    Float_t   vz;
    Short_t   centrality;
    Int_t     Ntrkoffline;
    
    // B meson kinematics
    Float_t   B_mass;
    Float_t   B_pt;
    Float_t   B_eta;
    Float_t   B_phi;
    Float_t   B_y;
    Int_t     B_charge;
    Int_t     B_decayId;
    Int_t     B_nTracks;
    Int_t     B_hasDau2;
    
    // B meson vertex
    Float_t   B_vtxX;
    Float_t   B_vtxY;
    Float_t   B_vtxZ;
    Float_t   B_vtxChi2;
    Float_t   B_vtxNdof;
    Float_t   B_vtxProb;
    
    // B meson decay length
    Float_t   B_lxy;
    Float_t   B_lxySig;
    Float_t   B_l3D;
    Float_t   B_l3DSig;
    Float_t   B_cosAlpha;
    
    // J/psi
    Int_t     jpsiIdx;
    Float_t   Jpsi_mass;
    Float_t   Jpsi_pt;
    Float_t   Jpsi_eta;
    Float_t   Jpsi_phi;
    Float_t   Jpsi_y;
    Float_t   resMass;
    
    // Track 1 (kaon)
    Float_t   trk1_pt;
    Float_t   trk1_eta;
    Float_t   trk1_phi;
    Int_t     trk1_charge;
    Float_t   trk1_massHypo;
    
    // Track 2 (if present)
    Float_t   trk2_pt;
    Float_t   trk2_eta;
    Float_t   trk2_phi;
    Int_t     trk2_charge;
    Float_t   trk2_massHypo;
    
    void CreateBranches(TTree* t) {
        t->Branch("run", &run);
        t->Branch("lumi", &lumi);
        t->Branch("event", &event);
        t->Branch("hiBin", &hiBin);
        t->Branch("hiHF", &hiHF);
        t->Branch("vz", &vz);
        t->Branch("centrality", &centrality);
        t->Branch("Ntrkoffline", &Ntrkoffline);
        t->Branch("B_mass", &B_mass);
        t->Branch("B_pt", &B_pt);
        t->Branch("B_eta", &B_eta);
        t->Branch("B_phi", &B_phi);
        t->Branch("B_y", &B_y);
        t->Branch("B_charge", &B_charge);
        t->Branch("B_decayId", &B_decayId);
        t->Branch("B_nTracks", &B_nTracks);
        t->Branch("B_hasDau2", &B_hasDau2);
        t->Branch("B_vtxX", &B_vtxX);
        t->Branch("B_vtxY", &B_vtxY);
        t->Branch("B_vtxZ", &B_vtxZ);
        t->Branch("B_vtxChi2", &B_vtxChi2);
        t->Branch("B_vtxNdof", &B_vtxNdof);
        t->Branch("B_vtxProb", &B_vtxProb);
        t->Branch("B_lxy", &B_lxy);
        t->Branch("B_lxySig", &B_lxySig);
        t->Branch("B_l3D", &B_l3D);
        t->Branch("B_l3DSig", &B_l3DSig);
        t->Branch("B_cosAlpha", &B_cosAlpha);
        t->Branch("jpsiIdx", &jpsiIdx);
        t->Branch("Jpsi_mass", &Jpsi_mass);
        t->Branch("Jpsi_pt", &Jpsi_pt);
        t->Branch("Jpsi_eta", &Jpsi_eta);
        t->Branch("Jpsi_phi", &Jpsi_phi);
        t->Branch("Jpsi_y", &Jpsi_y);
        t->Branch("resMass", &resMass);
        t->Branch("trk1_pt", &trk1_pt);
        t->Branch("trk1_eta", &trk1_eta);
        t->Branch("trk1_phi", &trk1_phi);
        t->Branch("trk1_charge", &trk1_charge);
        t->Branch("trk1_massHypo", &trk1_massHypo);
        t->Branch("trk2_pt", &trk2_pt);
        t->Branch("trk2_eta", &trk2_eta);
        t->Branch("trk2_phi", &trk2_phi);
        t->Branch("trk2_charge", &trk2_charge);
        t->Branch("trk2_massHypo", &trk2_massHypo);
    }
    
    void Fill(const BTreeBranches& b, int idx, const EventBranches* evt = nullptr) {
        run = b.run;
        lumi = b.lumi;
        event = b.event;
        centrality = b.centrality;
        Ntrkoffline = b.Ntrkoffline;
        vz = b.pvZ;
        
        if (evt) {
            hiBin = evt->hiBin;
            hiHF = evt->hiHF;
        } else {
            hiBin = b.centrality;
            hiHF = -999;
        }
        
        B_mass = b.B_mass->at(idx);
        B_pt = b.B_pt->at(idx);
        B_eta = b.B_eta->at(idx);
        B_phi = b.B_phi->at(idx);
        B_y = b.B_y->at(idx);
        B_charge = b.B_charge->at(idx);
        B_decayId = b.B_decayId->at(idx);
        B_nTracks = b.B_nTracks->at(idx);
        B_hasDau2 = b.B_hasDau2->at(idx);
        
        B_vtxX = b.B_vtxX->at(idx);
        B_vtxY = b.B_vtxY->at(idx);
        B_vtxZ = b.B_vtxZ->at(idx);
        B_vtxChi2 = b.B_vtxChi2->at(idx);
        B_vtxNdof = b.B_vtxNdof->at(idx);
        B_vtxProb = b.B_vtxProb->at(idx);
        
        B_lxy = b.B_lxy->at(idx);
        B_lxySig = b.B_lxySig->at(idx);
        B_l3D = b.B_l3D->at(idx);
        B_l3DSig = b.B_l3DSig->at(idx);
        B_cosAlpha = b.B_cosAlpha->at(idx);
        
        jpsiIdx = b.jpsiIdx->at(idx);
        Jpsi_mass = b.Jpsi_mass->at(idx);
        Jpsi_pt = b.Jpsi_pt->at(idx);
        Jpsi_eta = b.Jpsi_eta->at(idx);
        Jpsi_phi = b.Jpsi_phi->at(idx);
        Jpsi_y = b.Jpsi_y->at(idx);
        resMass = b.resMass->at(idx);
        
        trk1_pt = b.trk1_pt->at(idx);
        trk1_eta = b.trk1_eta->at(idx);
        trk1_phi = b.trk1_phi->at(idx);
        trk1_charge = b.trk1_charge->at(idx);
        trk1_massHypo = b.trk1_massHypo->at(idx);
        
        trk2_pt = b.trk2_pt->at(idx);
        trk2_eta = b.trk2_eta->at(idx);
        trk2_phi = b.trk2_phi->at(idx);
        trk2_charge = b.trk2_charge->at(idx);
        trk2_massHypo = b.trk2_massHypo->at(idx);
    }
};

// Flattened B meson + Jet matching
struct FlatBJetCandidate : public FlatBCandidate {
    // Jet matching
    Int_t     nJets;
    Int_t     hasMatchedJet;
    Int_t     jet_idx;
    Float_t   jet_pt;
    Float_t   jet_eta;
    Float_t   jet_phi;
    Float_t   jet_m;
    Float_t   jet_rawpt;
    Float_t   dR_Bjet;
    
    void CreateBranches(TTree* t) {
        FlatBCandidate::CreateBranches(t);
        t->Branch("nJets", &nJets);
        t->Branch("hasMatchedJet", &hasMatchedJet);
        t->Branch("jet_idx", &jet_idx);
        t->Branch("jet_pt", &jet_pt);
        t->Branch("jet_eta", &jet_eta);
        t->Branch("jet_phi", &jet_phi);
        t->Branch("jet_m", &jet_m);
        t->Branch("jet_rawpt", &jet_rawpt);
        t->Branch("dR_Bjet", &dR_Bjet);
    }
    
    void ClearJet() {
        nJets = 0;
        hasMatchedJet = 0;
        jet_idx = -1;
        jet_pt = -999;
        jet_eta = -999;
        jet_phi = -999;
        jet_m = -999;
        jet_rawpt = -999;
        dR_Bjet = 999;
    }
};

// Flattened dimuon candidate
struct FlatDimuonCandidate {
    // Event ID
    UInt_t    run;
    UInt_t    lumi;
    ULong64_t event;
    
    // Event info
    Int_t     hiBin;
    Float_t   hiHF;
    Float_t   vz;
    ULong64_t HLTriggers;
    
    // Dimuon kinematics
    Float_t   mass;
    Float_t   pt;
    Float_t   eta;
    Float_t   phi;
    Float_t   y;
    Int_t     type;
    Int_t     sign;
    ULong64_t trig;
    
    // Dimuon vertex
    Float_t   VtxProb;
    Float_t   ctau;
    Float_t   ctauErr;
    Float_t   cosAlpha;
    Float_t   ctau3D;
    Float_t   ctauErr3D;
    Float_t   cosAlpha3D;
    Float_t   dca;
    
    // mu+ kinematics
    Float_t   mupl_pt;
    Float_t   mupl_eta;
    Float_t   mupl_phi;
    Float_t   mupl_m;
    Int_t     mupl_charge;
    Int_t     mupl_type;
    Bool_t    mupl_highPurity;
    Bool_t    mupl_isSoft;
    Bool_t    mupl_isGlobal;
    Bool_t    mupl_isTracker;
    Bool_t    mupl_isPF;
    Float_t   mupl_softMva;
    Int_t     mupl_nPixValHits;
    Int_t     mupl_nMuValHits;
    Int_t     mupl_nTrkHits;
    Int_t     mupl_nPixWMea;
    Int_t     mupl_nTrkWMea;
    
    // mu- kinematics
    Float_t   mumi_pt;
    Float_t   mumi_eta;
    Float_t   mumi_phi;
    Float_t   mumi_m;
    Int_t     mumi_charge;
    Int_t     mumi_type;
    Bool_t    mumi_highPurity;
    Bool_t    mumi_isSoft;
    Bool_t    mumi_isGlobal;
    Bool_t    mumi_isTracker;
    Bool_t    mumi_isPF;
    Float_t   mumi_softMva;
    Int_t     mumi_nPixValHits;
    Int_t     mumi_nMuValHits;
    Int_t     mumi_nTrkHits;
    Int_t     mumi_nPixWMea;
    Int_t     mumi_nTrkWMea;
    
    void CreateBranches(TTree* t) {
        t->Branch("run", &run);
        t->Branch("lumi", &lumi);
        t->Branch("event", &event);
        t->Branch("hiBin", &hiBin);
        t->Branch("hiHF", &hiHF);
        t->Branch("vz", &vz);
        t->Branch("HLTriggers", &HLTriggers);
        t->Branch("mass", &mass);
        t->Branch("pt", &pt);
        t->Branch("eta", &eta);
        t->Branch("phi", &phi);
        t->Branch("y", &y);
        t->Branch("type", &type);
        t->Branch("sign", &sign);
        t->Branch("trig", &trig);
        t->Branch("VtxProb", &VtxProb);
        t->Branch("ctau", &ctau);
        t->Branch("ctauErr", &ctauErr);
        t->Branch("cosAlpha", &cosAlpha);
        t->Branch("ctau3D", &ctau3D);
        t->Branch("ctauErr3D", &ctauErr3D);
        t->Branch("cosAlpha3D", &cosAlpha3D);
        t->Branch("dca", &dca);
        t->Branch("mupl_pt", &mupl_pt);
        t->Branch("mupl_eta", &mupl_eta);
        t->Branch("mupl_phi", &mupl_phi);
        t->Branch("mupl_m", &mupl_m);
        t->Branch("mupl_charge", &mupl_charge);
        t->Branch("mupl_type", &mupl_type);
        t->Branch("mupl_highPurity", &mupl_highPurity);
        t->Branch("mupl_isSoft", &mupl_isSoft);
        t->Branch("mupl_isGlobal", &mupl_isGlobal);
        t->Branch("mupl_isTracker", &mupl_isTracker);
        t->Branch("mupl_isPF", &mupl_isPF);
        t->Branch("mupl_softMva", &mupl_softMva);
        t->Branch("mupl_nPixValHits", &mupl_nPixValHits);
        t->Branch("mupl_nMuValHits", &mupl_nMuValHits);
        t->Branch("mupl_nTrkHits", &mupl_nTrkHits);
        t->Branch("mupl_nPixWMea", &mupl_nPixWMea);
        t->Branch("mupl_nTrkWMea", &mupl_nTrkWMea);
        t->Branch("mumi_pt", &mumi_pt);
        t->Branch("mumi_eta", &mumi_eta);
        t->Branch("mumi_phi", &mumi_phi);
        t->Branch("mumi_m", &mumi_m);
        t->Branch("mumi_charge", &mumi_charge);
        t->Branch("mumi_type", &mumi_type);
        t->Branch("mumi_highPurity", &mumi_highPurity);
        t->Branch("mumi_isSoft", &mumi_isSoft);
        t->Branch("mumi_isGlobal", &mumi_isGlobal);
        t->Branch("mumi_isTracker", &mumi_isTracker);
        t->Branch("mumi_isPF", &mumi_isPF);
        t->Branch("mumi_softMva", &mumi_softMva);
        t->Branch("mumi_nPixValHits", &mumi_nPixValHits);
        t->Branch("mumi_nMuValHits", &mumi_nMuValHits);
        t->Branch("mumi_nTrkHits", &mumi_nTrkHits);
        t->Branch("mumi_nPixWMea", &mumi_nPixWMea);
        t->Branch("mumi_nTrkWMea", &mumi_nTrkWMea);
    }
};

// Flattened dielectron candidate
struct FlatDielectronCandidate {
    // Event ID
    UInt_t    run;
    UInt_t    lumi;
    ULong64_t event;
    
    // Event info
    Int_t     hiBin;
    Float_t   hiHF;
    Float_t   vz;
    Int_t     centralityBin;
    ULong64_t triggerBits;
    
    // Dielectron kinematics
    Float_t   mass;
    Float_t   pt;
    Float_t   eta;
    Float_t   phi;
    Float_t   y;
    Int_t     charge;
    ULong64_t trig;
    
    // Dielectron vertex
    Float_t   VtxProb;
    Float_t   chi2;
    Float_t   ndf;
    Float_t   ctau;
    Float_t   ctauErr;
    Float_t   ctau3D;
    Float_t   ctauErr3D;
    Float_t   cosAlpha;
    Float_t   cosAlpha3D;
    Float_t   dca;
    Float_t   vtxX;
    Float_t   vtxY;
    Float_t   vtxZ;
    
    // e+ kinematics (higher pT electron with charge > 0, or ele1 if same-sign)
    Float_t   ep_pt;
    Float_t   ep_eta;
    Float_t   ep_phi;
    Float_t   ep_y;
    Float_t   ep_mass;
    Int_t     ep_charge;
    Int_t     ep_isEB;
    Float_t   ep_energy;
    Float_t   ep_corrEnergy;
    Float_t   ep_trackPt;
    Float_t   ep_trackEta;
    Float_t   ep_trackPhi;
    Float_t   ep_trackNormalizedChi2;
    Int_t     ep_nValidHits;
    Int_t     ep_nMissingHits;
    Int_t     ep_trackerLayers;
    Int_t     ep_pixelLayers;
    Float_t   ep_dxy;
    Float_t   ep_dxyErr;
    Float_t   ep_dz;
    Float_t   ep_dzErr;
    Float_t   ep_scEta;
    Float_t   ep_scPhi;
    Float_t   ep_scEn;
    Float_t   ep_r9;
    Float_t   ep_sigmaIEtaIEta;
    Float_t   ep_hOverE;
    Float_t   ep_fbrem;
    Float_t   ep_eOverP;
    Float_t   ep_deltaEtaIn;
    Float_t   ep_deltaPhiIn;
    Float_t   ep_pfChIso;
    Float_t   ep_pfNeuIso;
    Float_t   ep_pfPhoIso;
    Int_t     ep_convVeto;
    Float_t   ep_MVAIso;
    Float_t   ep_MVAId;
    Int_t     ep_MVAIsoWP90;
    Int_t     ep_MVAIsoWP80;
    Int_t     ep_MVAIdWP90;
    Int_t     ep_MVAIdWP80;
    
    // e- kinematics
    Float_t   em_pt;
    Float_t   em_eta;
    Float_t   em_phi;
    Float_t   em_y;
    Float_t   em_mass;
    Int_t     em_charge;
    Int_t     em_isEB;
    Float_t   em_energy;
    Float_t   em_corrEnergy;
    Float_t   em_trackPt;
    Float_t   em_trackEta;
    Float_t   em_trackPhi;
    Float_t   em_trackNormalizedChi2;
    Int_t     em_nValidHits;
    Int_t     em_nMissingHits;
    Int_t     em_trackerLayers;
    Int_t     em_pixelLayers;
    Float_t   em_dxy;
    Float_t   em_dxyErr;
    Float_t   em_dz;
    Float_t   em_dzErr;
    Float_t   em_scEta;
    Float_t   em_scPhi;
    Float_t   em_scEn;
    Float_t   em_r9;
    Float_t   em_sigmaIEtaIEta;
    Float_t   em_hOverE;
    Float_t   em_fbrem;
    Float_t   em_eOverP;
    Float_t   em_deltaEtaIn;
    Float_t   em_deltaPhiIn;
    Float_t   em_pfChIso;
    Float_t   em_pfNeuIso;
    Float_t   em_pfPhoIso;
    Int_t     em_convVeto;
    Float_t   em_MVAIso;
    Float_t   em_MVAId;
    Int_t     em_MVAIsoWP90;
    Int_t     em_MVAIsoWP80;
    Int_t     em_MVAIdWP90;
    Int_t     em_MVAIdWP80;
    
    void CreateBranches(TTree* t) {
        t->Branch("run", &run);
        t->Branch("lumi", &lumi);
        t->Branch("event", &event);
        t->Branch("hiBin", &hiBin);
        t->Branch("hiHF", &hiHF);
        t->Branch("vz", &vz);
        t->Branch("centralityBin", &centralityBin);
        t->Branch("triggerBits", &triggerBits);
        t->Branch("mass", &mass);
        t->Branch("pt", &pt);
        t->Branch("eta", &eta);
        t->Branch("phi", &phi);
        t->Branch("y", &y);
        t->Branch("charge", &charge);
        t->Branch("trig", &trig);
        t->Branch("VtxProb", &VtxProb);
        t->Branch("chi2", &chi2);
        t->Branch("ndf", &ndf);
        t->Branch("ctau", &ctau);
        t->Branch("ctauErr", &ctauErr);
        t->Branch("ctau3D", &ctau3D);
        t->Branch("ctauErr3D", &ctauErr3D);
        t->Branch("cosAlpha", &cosAlpha);
        t->Branch("cosAlpha3D", &cosAlpha3D);
        t->Branch("dca", &dca);
        t->Branch("vtxX", &vtxX);
        t->Branch("vtxY", &vtxY);
        t->Branch("vtxZ", &vtxZ);
        // e+
        t->Branch("ep_pt", &ep_pt);
        t->Branch("ep_eta", &ep_eta);
        t->Branch("ep_phi", &ep_phi);
        t->Branch("ep_y", &ep_y);
        t->Branch("ep_mass", &ep_mass);
        t->Branch("ep_charge", &ep_charge);
        t->Branch("ep_isEB", &ep_isEB);
        t->Branch("ep_energy", &ep_energy);
        t->Branch("ep_corrEnergy", &ep_corrEnergy);
        t->Branch("ep_trackPt", &ep_trackPt);
        t->Branch("ep_trackEta", &ep_trackEta);
        t->Branch("ep_trackPhi", &ep_trackPhi);
        t->Branch("ep_trackNormalizedChi2", &ep_trackNormalizedChi2);
        t->Branch("ep_nValidHits", &ep_nValidHits);
        t->Branch("ep_nMissingHits", &ep_nMissingHits);
        t->Branch("ep_trackerLayers", &ep_trackerLayers);
        t->Branch("ep_pixelLayers", &ep_pixelLayers);
        t->Branch("ep_dxy", &ep_dxy);
        t->Branch("ep_dxyErr", &ep_dxyErr);
        t->Branch("ep_dz", &ep_dz);
        t->Branch("ep_dzErr", &ep_dzErr);
        t->Branch("ep_scEta", &ep_scEta);
        t->Branch("ep_scPhi", &ep_scPhi);
        t->Branch("ep_scEn", &ep_scEn);
        t->Branch("ep_r9", &ep_r9);
        t->Branch("ep_sigmaIEtaIEta", &ep_sigmaIEtaIEta);
        t->Branch("ep_hOverE", &ep_hOverE);
        t->Branch("ep_fbrem", &ep_fbrem);
        t->Branch("ep_eOverP", &ep_eOverP);
        t->Branch("ep_deltaEtaIn", &ep_deltaEtaIn);
        t->Branch("ep_deltaPhiIn", &ep_deltaPhiIn);
        t->Branch("ep_pfChIso", &ep_pfChIso);
        t->Branch("ep_pfNeuIso", &ep_pfNeuIso);
        t->Branch("ep_pfPhoIso", &ep_pfPhoIso);
        t->Branch("ep_convVeto", &ep_convVeto);
        t->Branch("ep_MVAIso", &ep_MVAIso);
        t->Branch("ep_MVAId", &ep_MVAId);
        t->Branch("ep_MVAIsoWP90", &ep_MVAIsoWP90);
        t->Branch("ep_MVAIsoWP80", &ep_MVAIsoWP80);
        t->Branch("ep_MVAIdWP90", &ep_MVAIdWP90);
        t->Branch("ep_MVAIdWP80", &ep_MVAIdWP80);
        // e-
        t->Branch("em_pt", &em_pt);
        t->Branch("em_eta", &em_eta);
        t->Branch("em_phi", &em_phi);
        t->Branch("em_y", &em_y);
        t->Branch("em_mass", &em_mass);
        t->Branch("em_charge", &em_charge);
        t->Branch("em_isEB", &em_isEB);
        t->Branch("em_energy", &em_energy);
        t->Branch("em_corrEnergy", &em_corrEnergy);
        t->Branch("em_trackPt", &em_trackPt);
        t->Branch("em_trackEta", &em_trackEta);
        t->Branch("em_trackPhi", &em_trackPhi);
        t->Branch("em_trackNormalizedChi2", &em_trackNormalizedChi2);
        t->Branch("em_nValidHits", &em_nValidHits);
        t->Branch("em_nMissingHits", &em_nMissingHits);
        t->Branch("em_trackerLayers", &em_trackerLayers);
        t->Branch("em_pixelLayers", &em_pixelLayers);
        t->Branch("em_dxy", &em_dxy);
        t->Branch("em_dxyErr", &em_dxyErr);
        t->Branch("em_dz", &em_dz);
        t->Branch("em_dzErr", &em_dzErr);
        t->Branch("em_scEta", &em_scEta);
        t->Branch("em_scPhi", &em_scPhi);
        t->Branch("em_scEn", &em_scEn);
        t->Branch("em_r9", &em_r9);
        t->Branch("em_sigmaIEtaIEta", &em_sigmaIEtaIEta);
        t->Branch("em_hOverE", &em_hOverE);
        t->Branch("em_fbrem", &em_fbrem);
        t->Branch("em_eOverP", &em_eOverP);
        t->Branch("em_deltaEtaIn", &em_deltaEtaIn);
        t->Branch("em_deltaPhiIn", &em_deltaPhiIn);
        t->Branch("em_pfChIso", &em_pfChIso);
        t->Branch("em_pfNeuIso", &em_pfNeuIso);
        t->Branch("em_pfPhoIso", &em_pfPhoIso);
        t->Branch("em_convVeto", &em_convVeto);
        t->Branch("em_MVAIso", &em_MVAIso);
        t->Branch("em_MVAId", &em_MVAId);
        t->Branch("em_MVAIsoWP90", &em_MVAIsoWP90);
        t->Branch("em_MVAIsoWP80", &em_MVAIsoWP80);
        t->Branch("em_MVAIdWP90", &em_MVAIdWP90);
        t->Branch("em_MVAIdWP80", &em_MVAIdWP80);
    }
};

// ============================================================================
// Utility functions
// ============================================================================

inline float deltaR(float eta1, float phi1, float eta2, float phi2) {
    float deta = eta1 - eta2;
    float dphi = phi1 - phi2;
    while (dphi > M_PI) dphi -= 2*M_PI;
    while (dphi < -M_PI) dphi += 2*M_PI;
    return sqrt(deta*deta + dphi*dphi);
}

#endif // HIFORESTBRANCHES_H
