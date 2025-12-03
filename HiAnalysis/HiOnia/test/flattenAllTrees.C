// flattenAllTrees.C
// Flatten all trees for correlation studies:
// 1. J/psi -> mumu (hionia)
// 2. J/psi -> ee (hioniaElectrons)  
// 3. D0 candidates
// 4. DStar candidates
// 5. B meson candidates (when available)
// 6. Jet-D0/DStar correlation
//
// Usage: root -l -b -q 'flattenAllTrees.C("HiForestOO_OniaBmesonDmeson1k.root")'

#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TMath.h>
#include <vector>
#include <iostream>

float deltaR(float eta1, float phi1, float eta2, float phi2) {
    float deta = eta1 - eta2;
    float dphi = phi1 - phi2;
    while (dphi > TMath::Pi()) dphi -= 2*TMath::Pi();
    while (dphi < -TMath::Pi()) dphi += 2*TMath::Pi();
    return TMath::Sqrt(deta*deta + dphi*dphi);
}

//=============================================================================
// Flatten J/psi -> mumu
//=============================================================================
void flattenJpsiMuMu(TFile* fIn, TFile* fOut) {
    TTree* tIn = (TTree*)fIn->Get("hionia/myTree");
    if (!tIn) { std::cout << "hionia/myTree not found" << std::endl; return; }
    
    // Input branches
    UInt_t runNb, eventNb;
    Short_t Reco_QQ_size;
    Float_t zVtx;
    std::vector<float> *Reco_QQ_pt = nullptr, *Reco_QQ_eta = nullptr;
    std::vector<float> *Reco_QQ_phi = nullptr, *Reco_QQ_m = nullptr, *Reco_QQ_y = nullptr;
    
    tIn->SetBranchAddress("runNb", &runNb);
    tIn->SetBranchAddress("eventNb", &eventNb);
    tIn->SetBranchAddress("zVtx", &zVtx);
    tIn->SetBranchAddress("Reco_QQ_size", &Reco_QQ_size);
    tIn->SetBranchAddress("Reco_QQ_4mom_pt", &Reco_QQ_pt);
    tIn->SetBranchAddress("Reco_QQ_4mom_eta", &Reco_QQ_eta);
    tIn->SetBranchAddress("Reco_QQ_4mom_phi", &Reco_QQ_phi);
    tIn->SetBranchAddress("Reco_QQ_4mom_m", &Reco_QQ_m);
    
    // ctau branches (fixed arrays)
    const int maxQQ = 100;
    Float_t ctau[maxQQ], ctauErr[maxQQ], ctau3D[maxQQ], ctauErr3D[maxQQ];
    Float_t cosAlpha[maxQQ];
    Short_t Reco_QQ_sign[maxQQ], Reco_QQ_type[maxQQ];
    Short_t mupl_idx[maxQQ], mumi_idx[maxQQ];
    
    tIn->SetBranchAddress("Reco_QQ_ctau", ctau);
    tIn->SetBranchAddress("Reco_QQ_ctauErr", ctauErr);
    tIn->SetBranchAddress("Reco_QQ_ctau3D", ctau3D);
    tIn->SetBranchAddress("Reco_QQ_ctauErr3D", ctauErr3D);
    tIn->SetBranchAddress("Reco_QQ_cosAlpha", cosAlpha);
    tIn->SetBranchAddress("Reco_QQ_sign", Reco_QQ_sign);
    tIn->SetBranchAddress("Reco_QQ_type", Reco_QQ_type);
    tIn->SetBranchAddress("Reco_QQ_mupl_idx", mupl_idx);
    tIn->SetBranchAddress("Reco_QQ_mumi_idx", mumi_idx);
    
    // Output tree
    fOut->cd();
    TTree* tOut = new TTree("flatJpsiMuMu", "Flattened J/psi -> mumu");
    
    UInt_t out_run, out_event;
    Float_t out_zVtx;
    Float_t out_pt, out_eta, out_phi, out_mass, out_y;
    Float_t out_ctau, out_ctauErr, out_ctau3D, out_ctauErr3D, out_cosAlpha;
    Short_t out_sign, out_type;
    Int_t out_idx;
    
    tOut->Branch("run", &out_run);
    tOut->Branch("event", &out_event);
    tOut->Branch("zVtx", &out_zVtx);
    tOut->Branch("idx", &out_idx);
    tOut->Branch("pt", &out_pt);
    tOut->Branch("eta", &out_eta);
    tOut->Branch("phi", &out_phi);
    tOut->Branch("mass", &out_mass);
    tOut->Branch("y", &out_y);
    tOut->Branch("ctau", &out_ctau);
    tOut->Branch("ctauErr", &out_ctauErr);
    tOut->Branch("ctau3D", &out_ctau3D);
    tOut->Branch("ctauErr3D", &out_ctauErr3D);
    tOut->Branch("cosAlpha", &out_cosAlpha);
    tOut->Branch("sign", &out_sign);
    tOut->Branch("type", &out_type);
    
    Long64_t nEntries = tIn->GetEntries();
    Long64_t nCand = 0;
    
    for (Long64_t i = 0; i < nEntries; i++) {
        tIn->GetEntry(i);
        if (Reco_QQ_size == 0) continue;
        
        for (int j = 0; j < Reco_QQ_size && j < maxQQ; j++) {
            out_run = runNb;
            out_event = eventNb;
            out_zVtx = zVtx;
            out_idx = j;
            out_pt = Reco_QQ_pt->at(j);
            out_eta = Reco_QQ_eta->at(j);
            out_phi = Reco_QQ_phi->at(j);
            out_mass = Reco_QQ_m->at(j);
            out_y = 0.5 * log((sqrt(out_mass*out_mass + out_pt*out_pt*cosh(out_eta)*cosh(out_eta)) + out_pt*sinh(out_eta)) /
                             (sqrt(out_mass*out_mass + out_pt*out_pt*cosh(out_eta)*cosh(out_eta)) - out_pt*sinh(out_eta)));
            out_ctau = ctau[j];
            out_ctauErr = ctauErr[j];
            out_ctau3D = ctau3D[j];
            out_ctauErr3D = ctauErr3D[j];
            out_cosAlpha = cosAlpha[j];
            out_sign = Reco_QQ_sign[j];
            out_type = Reco_QQ_type[j];
            
            tOut->Fill();
            nCand++;
        }
    }
    
    tOut->Write();
    std::cout << "flatJpsiMuMu: " << nCand << " candidates from " << nEntries << " events" << std::endl;
}

//=============================================================================
// Flatten J/psi -> ee
//=============================================================================
void flattenJpsiEE(TFile* fIn, TFile* fOut) {
    TTree* tIn = (TTree*)fIn->Get("hioniaElectrons/eleTree");
    if (!tIn) { std::cout << "hioniaElectrons/eleTree not found" << std::endl; return; }
    
    // Input branches
    UInt_t run, lumi;
    ULong64_t event;
    Int_t Reco_ee_size;
    Float_t pvZ;
    std::vector<float> *Reco_ee_pt = nullptr, *Reco_ee_eta = nullptr;
    std::vector<float> *Reco_ee_phi = nullptr, *Reco_ee_mass = nullptr, *Reco_ee_y = nullptr;
    
    tIn->SetBranchAddress("run", &run);
    tIn->SetBranchAddress("lumi", &lumi);
    tIn->SetBranchAddress("event", &event);
    tIn->SetBranchAddress("pvZ", &pvZ);
    tIn->SetBranchAddress("Reco_ee_size", &Reco_ee_size);
    tIn->SetBranchAddress("Reco_ee_pt", &Reco_ee_pt);
    tIn->SetBranchAddress("Reco_ee_eta", &Reco_ee_eta);
    tIn->SetBranchAddress("Reco_ee_phi", &Reco_ee_phi);
    tIn->SetBranchAddress("Reco_ee_mass", &Reco_ee_mass);
    tIn->SetBranchAddress("Reco_ee_y", &Reco_ee_y);
    
    // Additional branches
    const int maxEE = 100;
    Float_t vProb[maxEE], chi2[maxEE];
    Short_t charge[maxEE], ele1Idx[maxEE], ele2Idx[maxEE];
    Float_t ctau[maxEE], ctauErr[maxEE], ctau3D[maxEE], ctauErr3D[maxEE];
    Float_t cosAlpha[maxEE], cosAlpha3D[maxEE], dca[maxEE];
    
    tIn->SetBranchAddress("Reco_ee_vProb", vProb);
    tIn->SetBranchAddress("Reco_ee_chi2", chi2);
    tIn->SetBranchAddress("Reco_ee_charge", charge);
    tIn->SetBranchAddress("Reco_ee_ele1Idx", ele1Idx);
    tIn->SetBranchAddress("Reco_ee_ele2Idx", ele2Idx);
    tIn->SetBranchAddress("Reco_ee_ctau", ctau);
    tIn->SetBranchAddress("Reco_ee_ctauErr", ctauErr);
    tIn->SetBranchAddress("Reco_ee_ctau3D", ctau3D);
    tIn->SetBranchAddress("Reco_ee_ctauErr3D", ctauErr3D);
    tIn->SetBranchAddress("Reco_ee_cosAlpha", cosAlpha);
    tIn->SetBranchAddress("Reco_ee_cosAlpha3D", cosAlpha3D);
    tIn->SetBranchAddress("Reco_ee_dca", dca);
    
    // Output tree
    fOut->cd();
    TTree* tOut = new TTree("flatJpsiEE", "Flattened J/psi -> ee");
    
    UInt_t out_run;
    ULong64_t out_event;
    Float_t out_pvZ;
    Float_t out_pt, out_eta, out_phi, out_mass, out_y;
    Float_t out_vProb, out_chi2;
    Short_t out_charge;
    Int_t out_idx;
    Float_t out_ctau, out_ctauErr, out_ctau3D, out_ctauErr3D;
    Float_t out_cosAlpha, out_cosAlpha3D, out_dca;
    
    tOut->Branch("run", &out_run);
    tOut->Branch("event", &out_event);
    tOut->Branch("pvZ", &out_pvZ);
    tOut->Branch("idx", &out_idx);
    tOut->Branch("pt", &out_pt);
    tOut->Branch("eta", &out_eta);
    tOut->Branch("phi", &out_phi);
    tOut->Branch("mass", &out_mass);
    tOut->Branch("y", &out_y);
    tOut->Branch("vProb", &out_vProb);
    tOut->Branch("chi2", &out_chi2);
    tOut->Branch("charge", &out_charge);
    tOut->Branch("ctau", &out_ctau);
    tOut->Branch("ctauErr", &out_ctauErr);
    tOut->Branch("ctau3D", &out_ctau3D);
    tOut->Branch("ctauErr3D", &out_ctauErr3D);
    tOut->Branch("cosAlpha", &out_cosAlpha);
    tOut->Branch("cosAlpha3D", &out_cosAlpha3D);
    tOut->Branch("dca", &out_dca);
    
    Long64_t nEntries = tIn->GetEntries();
    Long64_t nCand = 0;
    
    for (Long64_t i = 0; i < nEntries; i++) {
        tIn->GetEntry(i);
        if (Reco_ee_size == 0) continue;
        
        for (int j = 0; j < Reco_ee_size && j < maxEE; j++) {
            out_run = run;
            out_event = event;
            out_pvZ = pvZ;
            out_idx = j;
            out_pt = Reco_ee_pt->at(j);
            out_eta = Reco_ee_eta->at(j);
            out_phi = Reco_ee_phi->at(j);
            out_mass = Reco_ee_mass->at(j);
            out_y = Reco_ee_y->at(j);
            out_vProb = vProb[j];
            out_chi2 = chi2[j];
            out_charge = charge[j];
            out_ctau = ctau[j];
            out_ctauErr = ctauErr[j];
            out_ctau3D = ctau3D[j];
            out_ctauErr3D = ctauErr3D[j];
            out_cosAlpha = cosAlpha[j];
            out_cosAlpha3D = cosAlpha3D[j];
            out_dca = dca[j];
            
            tOut->Fill();
            nCand++;
        }
    }
    
    tOut->Write();
    std::cout << "flatJpsiEE: " << nCand << " candidates from " << nEntries << " events" << std::endl;
}

//=============================================================================
// Flatten D0 candidates
//=============================================================================
void flattenD0(TFile* fIn, TFile* fOut) {
    TTree* tIn = (TTree*)fIn->Get("d0Ntuplizer/candTree");
    if (!tIn) { std::cout << "d0Ntuplizer/candTree not found" << std::endl; return; }
    
    // Input branches
    UInt_t runNb, eventNb, candSize;
    Float_t bestvtxZ;
    Short_t centrality;
    
    std::vector<float> *pT = nullptr, *eta = nullptr, *phi = nullptr, *mass = nullptr, *y = nullptr;
    std::vector<float> *VtxProb = nullptr, *alpha3D = nullptr, *decayLength3D = nullptr;
    std::vector<int> *bestPVIdx = nullptr;
    std::vector<float> *bestPVDz = nullptr;
    std::vector<std::vector<float>> *pTD = nullptr, *etaD = nullptr;
    std::vector<std::vector<short>> *chargeD = nullptr;
    
    tIn->SetBranchAddress("RunNb", &runNb);
    tIn->SetBranchAddress("EventNb", &eventNb);
    tIn->SetBranchAddress("candSize", &candSize);
    tIn->SetBranchAddress("bestvtxZ", &bestvtxZ);
    tIn->SetBranchAddress("centrality", &centrality);
    tIn->SetBranchAddress("pT", &pT);
    tIn->SetBranchAddress("eta", &eta);
    tIn->SetBranchAddress("phi", &phi);
    tIn->SetBranchAddress("mass", &mass);
    tIn->SetBranchAddress("y", &y);
    tIn->SetBranchAddress("VtxProb", &VtxProb);
    tIn->SetBranchAddress("alpha3D", &alpha3D);
    tIn->SetBranchAddress("decayLength3D", &decayLength3D);
    tIn->SetBranchAddress("bestPVIdx", &bestPVIdx);
    tIn->SetBranchAddress("bestPVDz", &bestPVDz);
    tIn->SetBranchAddress("pTD", &pTD);
    tIn->SetBranchAddress("etaD", &etaD);
    tIn->SetBranchAddress("chargeD", &chargeD);
    
    // Output tree
    fOut->cd();
    TTree* tOut = new TTree("flatD0", "Flattened D0 candidates");
    
    UInt_t out_run, out_event;
    Float_t out_bestvtxZ;
    Short_t out_centrality;
    Int_t out_idx, out_pvIdx;
    Float_t out_pt, out_eta, out_phi, out_mass, out_y;
    Float_t out_vProb, out_alpha3D, out_decayLen3D, out_pvDz;
    Float_t out_dauPt1, out_dauPt2, out_dauEta1, out_dauEta2;
    Short_t out_dauChg1, out_dauChg2;
    
    tOut->Branch("run", &out_run);
    tOut->Branch("event", &out_event);
    tOut->Branch("bestvtxZ", &out_bestvtxZ);
    tOut->Branch("centrality", &out_centrality);
    tOut->Branch("idx", &out_idx);
    tOut->Branch("pvIdx", &out_pvIdx);
    tOut->Branch("pvDz", &out_pvDz);
    tOut->Branch("pt", &out_pt);
    tOut->Branch("eta", &out_eta);
    tOut->Branch("phi", &out_phi);
    tOut->Branch("mass", &out_mass);
    tOut->Branch("y", &out_y);
    tOut->Branch("vProb", &out_vProb);
    tOut->Branch("alpha3D", &out_alpha3D);
    tOut->Branch("decayLen3D", &out_decayLen3D);
    tOut->Branch("dauPt1", &out_dauPt1);
    tOut->Branch("dauPt2", &out_dauPt2);
    tOut->Branch("dauEta1", &out_dauEta1);
    tOut->Branch("dauEta2", &out_dauEta2);
    tOut->Branch("dauChg1", &out_dauChg1);
    tOut->Branch("dauChg2", &out_dauChg2);
    
    Long64_t nEntries = tIn->GetEntries();
    Long64_t nCand = 0;
    
    for (Long64_t i = 0; i < nEntries; i++) {
        tIn->GetEntry(i);
        if (candSize == 0) continue;
        
        for (UInt_t j = 0; j < candSize; j++) {
            out_run = runNb;
            out_event = eventNb;
            out_bestvtxZ = bestvtxZ;
            out_centrality = centrality;
            out_idx = j;
            out_pvIdx = bestPVIdx->at(j);
            out_pvDz = bestPVDz->at(j);
            out_pt = pT->at(j);
            out_eta = eta->at(j);
            out_phi = phi->at(j);
            out_mass = mass->at(j);
            out_y = y->at(j);
            out_vProb = VtxProb->at(j);
            out_alpha3D = alpha3D->at(j);
            out_decayLen3D = decayLength3D->at(j);
            
            auto& dauPt = pTD->at(j);
            auto& dauEta = etaD->at(j);
            auto& dauChg = chargeD->at(j);
            out_dauPt1 = dauPt.size() > 0 ? dauPt[0] : -99;
            out_dauPt2 = dauPt.size() > 1 ? dauPt[1] : -99;
            out_dauEta1 = dauEta.size() > 0 ? dauEta[0] : -99;
            out_dauEta2 = dauEta.size() > 1 ? dauEta[1] : -99;
            out_dauChg1 = dauChg.size() > 0 ? dauChg[0] : 0;
            out_dauChg2 = dauChg.size() > 1 ? dauChg[1] : 0;
            
            tOut->Fill();
            nCand++;
        }
    }
    
    tOut->Write();
    std::cout << "flatD0: " << nCand << " candidates from " << nEntries << " events" << std::endl;
}

//=============================================================================
// Flatten DStar candidates
//=============================================================================
void flattenDStar(TFile* fIn, TFile* fOut) {
    TTree* tIn = (TTree*)fIn->Get("dStarNtuplizer/candTree");
    if (!tIn) { std::cout << "dStarNtuplizer/candTree not found" << std::endl; return; }
    
    // Input branches
    UInt_t runNb, eventNb, candSize;
    Float_t bestvtxZ;
    Short_t centrality;
    
    std::vector<float> *pT = nullptr, *eta = nullptr, *phi = nullptr, *mass = nullptr, *y = nullptr;
    std::vector<float> *VtxProb = nullptr, *deltaM = nullptr;
    std::vector<float> *dauCand_mass = nullptr, *dauCand_pt = nullptr;
    std::vector<int> *bestPVIdx = nullptr;
    std::vector<float> *bestPVDz = nullptr;
    
    tIn->SetBranchAddress("RunNb", &runNb);
    tIn->SetBranchAddress("EventNb", &eventNb);
    tIn->SetBranchAddress("candSize", &candSize);
    tIn->SetBranchAddress("bestvtxZ", &bestvtxZ);
    tIn->SetBranchAddress("centrality", &centrality);
    tIn->SetBranchAddress("pT", &pT);
    tIn->SetBranchAddress("eta", &eta);
    tIn->SetBranchAddress("phi", &phi);
    tIn->SetBranchAddress("mass", &mass);
    tIn->SetBranchAddress("y", &y);
    tIn->SetBranchAddress("VtxProb", &VtxProb);
    if (tIn->GetBranch("deltaM")) tIn->SetBranchAddress("deltaM", &deltaM);
    if (tIn->GetBranch("dauCand_mass")) tIn->SetBranchAddress("dauCand_mass", &dauCand_mass);
    if (tIn->GetBranch("dauCand_pt")) tIn->SetBranchAddress("dauCand_pt", &dauCand_pt);
    tIn->SetBranchAddress("bestPVIdx", &bestPVIdx);
    tIn->SetBranchAddress("bestPVDz", &bestPVDz);
    
    // Output tree
    fOut->cd();
    TTree* tOut = new TTree("flatDStar", "Flattened DStar candidates");
    
    UInt_t out_run, out_event;
    Float_t out_bestvtxZ;
    Short_t out_centrality;
    Int_t out_idx, out_pvIdx;
    Float_t out_pt, out_eta, out_phi, out_mass, out_y;
    Float_t out_vProb, out_deltaM, out_D0mass, out_D0pt, out_pvDz;
    
    tOut->Branch("run", &out_run);
    tOut->Branch("event", &out_event);
    tOut->Branch("bestvtxZ", &out_bestvtxZ);
    tOut->Branch("centrality", &out_centrality);
    tOut->Branch("idx", &out_idx);
    tOut->Branch("pvIdx", &out_pvIdx);
    tOut->Branch("pvDz", &out_pvDz);
    tOut->Branch("pt", &out_pt);
    tOut->Branch("eta", &out_eta);
    tOut->Branch("phi", &out_phi);
    tOut->Branch("mass", &out_mass);
    tOut->Branch("y", &out_y);
    tOut->Branch("vProb", &out_vProb);
    tOut->Branch("deltaM", &out_deltaM);
    tOut->Branch("D0mass", &out_D0mass);
    tOut->Branch("D0pt", &out_D0pt);
    
    Long64_t nEntries = tIn->GetEntries();
    Long64_t nCand = 0;
    
    for (Long64_t i = 0; i < nEntries; i++) {
        tIn->GetEntry(i);
        if (candSize == 0) continue;
        
        for (UInt_t j = 0; j < candSize; j++) {
            out_run = runNb;
            out_event = eventNb;
            out_bestvtxZ = bestvtxZ;
            out_centrality = centrality;
            out_idx = j;
            out_pvIdx = bestPVIdx->at(j);
            out_pvDz = bestPVDz->at(j);
            out_pt = pT->at(j);
            out_eta = eta->at(j);
            out_phi = phi->at(j);
            out_mass = mass->at(j);
            out_y = y->at(j);
            out_vProb = VtxProb->at(j);
            out_deltaM = (deltaM && j < deltaM->size()) ? deltaM->at(j) : -99;
            out_D0mass = (dauCand_mass && j < dauCand_mass->size()) ? dauCand_mass->at(j) : -99;
            out_D0pt = (dauCand_pt && j < dauCand_pt->size()) ? dauCand_pt->at(j) : -99;
            
            tOut->Fill();
            nCand++;
        }
    }
    
    tOut->Write();
    std::cout << "flatDStar: " << nCand << " candidates from " << nEntries << " events" << std::endl;
}

//=============================================================================
// D0-DStar correlation (same event)
//=============================================================================
void flattenD0DStarCorrelation(TFile* fIn, TFile* fOut) {
    fIn->cd();
    TTree* tD0 = (TTree*)fIn->Get("d0Ntuplizer/candTree");
    TTree* tDS = (TTree*)fIn->Get("dStarNtuplizer/candTree");
    if (!tD0 || !tDS) { std::cout << "D0 or DStar tree not found" << std::endl; return; }
    
    // D0 branches
    UInt_t d0_runNb, d0_eventNb, d0_candSize;
    std::vector<float> *d0_pT = nullptr, *d0_eta = nullptr, *d0_phi = nullptr, *d0_mass = nullptr;
    tD0->SetBranchAddress("RunNb", &d0_runNb);
    tD0->SetBranchAddress("EventNb", &d0_eventNb);
    tD0->SetBranchAddress("candSize", &d0_candSize);
    tD0->SetBranchAddress("pT", &d0_pT);
    tD0->SetBranchAddress("eta", &d0_eta);
    tD0->SetBranchAddress("phi", &d0_phi);
    tD0->SetBranchAddress("mass", &d0_mass);
    
    // DStar branches
    UInt_t ds_runNb, ds_eventNb, ds_candSize;
    std::vector<float> *ds_pT = nullptr, *ds_eta = nullptr, *ds_phi = nullptr, *ds_mass = nullptr;
    tDS->SetBranchAddress("RunNb", &ds_runNb);
    tDS->SetBranchAddress("EventNb", &ds_eventNb);
    tDS->SetBranchAddress("candSize", &ds_candSize);
    tDS->SetBranchAddress("pT", &ds_pT);
    tDS->SetBranchAddress("eta", &ds_eta);
    tDS->SetBranchAddress("phi", &ds_phi);
    tDS->SetBranchAddress("mass", &ds_mass);
    
    // Output tree
    fOut->cd();
    TTree* tOut = new TTree("flatD0DStarCorr", "D0-DStar correlation");
    
    UInt_t out_run, out_event;
    Int_t out_d0Idx, out_dsIdx;
    Float_t out_d0Pt, out_d0Eta, out_d0Phi, out_d0Mass;
    Float_t out_dsPt, out_dsEta, out_dsPhi, out_dsMass;
    Float_t out_dR, out_dEta, out_dPhi;
    
    tOut->Branch("run", &out_run);
    tOut->Branch("event", &out_event);
    tOut->Branch("d0Idx", &out_d0Idx);
    tOut->Branch("dsIdx", &out_dsIdx);
    tOut->Branch("d0Pt", &out_d0Pt);
    tOut->Branch("d0Eta", &out_d0Eta);
    tOut->Branch("d0Phi", &out_d0Phi);
    tOut->Branch("d0Mass", &out_d0Mass);
    tOut->Branch("dsPt", &out_dsPt);
    tOut->Branch("dsEta", &out_dsEta);
    tOut->Branch("dsPhi", &out_dsPhi);
    tOut->Branch("dsMass", &out_dsMass);
    tOut->Branch("dR", &out_dR);
    tOut->Branch("dEta", &out_dEta);
    tOut->Branch("dPhi", &out_dPhi);
    
    Long64_t nEntries = tD0->GetEntries();
    Long64_t nPairs = 0;
    
    for (Long64_t i = 0; i < nEntries; i++) {
        tD0->GetEntry(i);
        tDS->GetEntry(i);
        
        if (d0_candSize == 0 || ds_candSize == 0) continue;
        
        for (UInt_t jd0 = 0; jd0 < d0_candSize; jd0++) {
            for (UInt_t jds = 0; jds < ds_candSize; jds++) {
                out_run = d0_runNb;
                out_event = d0_eventNb;
                out_d0Idx = jd0;
                out_dsIdx = jds;
                out_d0Pt = d0_pT->at(jd0);
                out_d0Eta = d0_eta->at(jd0);
                out_d0Phi = d0_phi->at(jd0);
                out_d0Mass = d0_mass->at(jd0);
                out_dsPt = ds_pT->at(jds);
                out_dsEta = ds_eta->at(jds);
                out_dsPhi = ds_phi->at(jds);
                out_dsMass = ds_mass->at(jds);
                out_dR = deltaR(out_d0Eta, out_d0Phi, out_dsEta, out_dsPhi);
                out_dEta = out_d0Eta - out_dsEta;
                out_dPhi = out_d0Phi - out_dsPhi;
                while (out_dPhi > TMath::Pi()) out_dPhi -= 2*TMath::Pi();
                while (out_dPhi < -TMath::Pi()) out_dPhi += 2*TMath::Pi();
                
                tOut->Fill();
                nPairs++;
            }
        }
    }
    
    tOut->Write();
    std::cout << "flatD0DStarCorr: " << nPairs << " pairs from " << nEntries << " events" << std::endl;
}

//=============================================================================
// Jet-D0 matching
//=============================================================================
void flattenJetD0Matching(TFile* fIn, TFile* fOut, float dRcut = 0.4) {
    fIn->cd();
    TTree* tJet = (TTree*)fIn->Get("akCs4PFJetAnalyzer/t");
    TTree* tD0 = (TTree*)fIn->Get("d0Ntuplizer/candTree");
    if (!tJet || !tD0) { std::cout << "Jet or D0 tree not found" << std::endl; return; }
    
    // Jet branches
    Int_t jet_run, jet_evt, jet_nref;
    const int maxJets = 500;
    Float_t jet_pt[maxJets], jet_eta[maxJets], jet_phi[maxJets];
    tJet->SetBranchAddress("run", &jet_run);
    tJet->SetBranchAddress("evt", &jet_evt);
    tJet->SetBranchAddress("nref", &jet_nref);
    tJet->SetBranchAddress("jtpt", jet_pt);
    tJet->SetBranchAddress("jteta", jet_eta);
    tJet->SetBranchAddress("jtphi", jet_phi);
    
    // D0 branches
    UInt_t d0_runNb, d0_eventNb, d0_candSize;
    std::vector<float> *d0_pT = nullptr, *d0_eta = nullptr, *d0_phi = nullptr, *d0_mass = nullptr;
    std::vector<float> *d0_VtxProb = nullptr;
    tD0->SetBranchAddress("RunNb", &d0_runNb);
    tD0->SetBranchAddress("EventNb", &d0_eventNb);
    tD0->SetBranchAddress("candSize", &d0_candSize);
    tD0->SetBranchAddress("pT", &d0_pT);
    tD0->SetBranchAddress("eta", &d0_eta);
    tD0->SetBranchAddress("phi", &d0_phi);
    tD0->SetBranchAddress("mass", &d0_mass);
    tD0->SetBranchAddress("VtxProb", &d0_VtxProb);
    
    // Output tree
    fOut->cd();
    TTree* tOut = new TTree("flatJetD0Match", "Jet-D0 matching");
    
    UInt_t out_run, out_event;
    Int_t out_jetIdx, out_d0Idx;
    Float_t out_jetPt, out_jetEta, out_jetPhi;
    Float_t out_d0Pt, out_d0Eta, out_d0Phi, out_d0Mass, out_d0VtxProb;
    Float_t out_dR;
    Float_t out_d0PtRatio;  // D0 pT / jet pT
    
    tOut->Branch("run", &out_run);
    tOut->Branch("event", &out_event);
    tOut->Branch("jetIdx", &out_jetIdx);
    tOut->Branch("d0Idx", &out_d0Idx);
    tOut->Branch("jetPt", &out_jetPt);
    tOut->Branch("jetEta", &out_jetEta);
    tOut->Branch("jetPhi", &out_jetPhi);
    tOut->Branch("d0Pt", &out_d0Pt);
    tOut->Branch("d0Eta", &out_d0Eta);
    tOut->Branch("d0Phi", &out_d0Phi);
    tOut->Branch("d0Mass", &out_d0Mass);
    tOut->Branch("d0VtxProb", &out_d0VtxProb);
    tOut->Branch("dR", &out_dR);
    tOut->Branch("d0PtRatio", &out_d0PtRatio);
    
    Long64_t nEntries = tJet->GetEntries();
    Long64_t nMatches = 0;
    
    for (Long64_t i = 0; i < nEntries; i++) {
        tJet->GetEntry(i);
        tD0->GetEntry(i);
        
        if (jet_nref == 0 || d0_candSize == 0) continue;
        
        for (int jjet = 0; jjet < jet_nref; jjet++) {
            for (UInt_t jd0 = 0; jd0 < d0_candSize; jd0++) {
                float dR = deltaR(jet_eta[jjet], jet_phi[jjet], d0_eta->at(jd0), d0_phi->at(jd0));
                if (dR > dRcut) continue;
                
                out_run = d0_runNb;
                out_event = d0_eventNb;
                out_jetIdx = jjet;
                out_d0Idx = jd0;
                out_jetPt = jet_pt[jjet];
                out_jetEta = jet_eta[jjet];
                out_jetPhi = jet_phi[jjet];
                out_d0Pt = d0_pT->at(jd0);
                out_d0Eta = d0_eta->at(jd0);
                out_d0Phi = d0_phi->at(jd0);
                out_d0Mass = d0_mass->at(jd0);
                out_d0VtxProb = d0_VtxProb->at(jd0);
                out_dR = dR;
                out_d0PtRatio = out_d0Pt / out_jetPt;
                
                tOut->Fill();
                nMatches++;
            }
        }
    }
    
    tOut->Write();
    std::cout << "flatJetD0Match (dR<" << dRcut << "): " << nMatches << " matches from " << nEntries << " events" << std::endl;
}

//=============================================================================
// Main function
//=============================================================================
void flattenAllTrees(TString inputFile = "HiForestOO_OniaBmesonDmeson1k.root",
                     TString outputFile = "flatTrees.root") {
    
    TFile* fIn = TFile::Open(inputFile);
    if (!fIn || fIn->IsZombie()) {
        std::cout << "ERROR: Cannot open " << inputFile << std::endl;
        return;
    }
    
    TFile* fOut = new TFile(outputFile, "RECREATE");
    
    std::cout << "\n=== Flattening all trees ===" << std::endl;
    std::cout << "Input: " << inputFile << std::endl;
    std::cout << "Output: " << outputFile << std::endl;
    std::cout << std::endl;
    
    // Flatten individual trees
    flattenJpsiMuMu(fIn, fOut);
    flattenJpsiEE(fIn, fOut);
    flattenD0(fIn, fOut);
    flattenDStar(fIn, fOut);
    
    // Correlation trees - skip for now due to ROOT vector reading issues
    // flattenD0DStarCorrelation(fIn, fOut);
    // flattenJetD0Matching(fIn, fOut, 0.4);
    std::cout << "\nNote: D0-DStar correlation and Jet-D0 matching skipped (use separate scripts)" << std::endl;
    
    fOut->Close();
    fIn->Close();
    
    std::cout << "\nDone! Output: " << outputFile << std::endl;
}
