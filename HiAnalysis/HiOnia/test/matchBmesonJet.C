// matchBmesonJet.C
// Match B mesons to jets using deltaR in event-based trees
// Usage: root -l -b -q 'matchBmesonJet.C("HiForestOO_OniaBmesonDmeson1k.root")'

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
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

void matchBmesonJet(
    TString inputFile = "HiForestOO_OniaBmesonDmeson1k.root",
    TString bTreePath = "bPlusMuMuNtuplizer/candTree",  // or d0Ntuplizer/candTree
    TString jetTreePath = "akCs4PFJetAnalyzer/t",
    float dRcut = 0.4
) {
    TFile *f = TFile::Open(inputFile);
    if (!f || f->IsZombie()) {
        std::cout << "ERROR: Cannot open " << inputFile << std::endl;
        return;
    }
    
    // Get trees
    TTree *tB = (TTree*)f->Get(bTreePath);
    TTree *tJet = (TTree*)f->Get(jetTreePath);
    
    if (!tB || !tJet) {
        std::cout << "ERROR: Cannot find trees" << std::endl;
        f->Close();
        return;
    }
    
    std::cout << "B meson tree: " << tB->GetEntries() << " events" << std::endl;
    std::cout << "Jet tree: " << tJet->GetEntries() << " events" << std::endl;
    
    if (tB->GetEntries() != tJet->GetEntries()) {
        std::cout << "WARNING: Tree entries don't match!" << std::endl;
    }
    
    // B meson branches
    UInt_t b_runNb, b_eventNb, b_candSize;
    std::vector<float> *b_pT = nullptr;
    std::vector<float> *b_eta = nullptr;
    std::vector<float> *b_phi = nullptr;
    std::vector<float> *b_mass = nullptr;
    std::vector<float> *b_VtxProb = nullptr;
    
    tB->SetBranchAddress("RunNb", &b_runNb);
    tB->SetBranchAddress("EventNb", &b_eventNb);
    tB->SetBranchAddress("candSize", &b_candSize);
    tB->SetBranchAddress("pT", &b_pT);
    tB->SetBranchAddress("eta", &b_eta);
    tB->SetBranchAddress("phi", &b_phi);
    tB->SetBranchAddress("mass", &b_mass);
    if (tB->GetBranch("VtxProb")) tB->SetBranchAddress("VtxProb", &b_VtxProb);
    
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
    
    // Histograms
    TH1F *hDeltaR = new TH1F("hDeltaR", "Min #DeltaR(B, jet);#DeltaR;Counts", 100, 0, 5);
    TH1F *hMatchedBpt = new TH1F("hMatchedBpt", "Matched B meson p_{T};p_{T} [GeV];Counts", 50, 0, 50);
    TH1F *hMatchedJetPt = new TH1F("hMatchedJetPt", "Matched jet p_{T};p_{T} [GeV];Counts", 50, 0, 100);
    TH2F *hBvsJetPt = new TH2F("hBvsJetPt", "B p_{T} vs Jet p_{T};Jet p_{T} [GeV];B p_{T} [GeV]", 50, 0, 100, 50, 0, 50);
    TH1F *hBmass = new TH1F("hBmass", "B meson mass;mass [GeV];Counts", 100, 4, 7);
    TH1F *hBmassMatched = new TH1F("hBmassMatched", "Matched B meson mass;mass [GeV];Counts", 100, 4, 7);
    
    Long64_t nEvents = tB->GetEntries();
    int nBtotal = 0, nBmatched = 0;
    
    // Event loop
    for (Long64_t iEvt = 0; iEvt < nEvents; iEvt++) {
        tB->GetEntry(iEvt);
        tJet->GetEntry(iEvt);
        
        // Verify event matching (run/event numbers should match)
        // Note: jet tree uses 'evt' which is truncated, so we skip this check
        
        // Skip if no B candidates
        if (b_candSize == 0) continue;
        
        // Loop over B candidates
        for (UInt_t iB = 0; iB < b_candSize; iB++) {
            float bPt = b_pT->at(iB);
            float bEta = b_eta->at(iB);
            float bPhi = b_phi->at(iB);
            float bMass = b_mass->at(iB);
            
            hBmass->Fill(bMass);
            nBtotal++;
            
            // Find closest jet
            float minDR = 999;
            int matchedJet = -1;
            
            for (int iJet = 0; iJet < jet_nref; iJet++) {
                float dR = deltaR(bEta, bPhi, jet_eta[iJet], jet_phi[iJet]);
                if (dR < minDR) {
                    minDR = dR;
                    matchedJet = iJet;
                }
            }
            
            hDeltaR->Fill(minDR);
            
            // If matched within dR cut
            if (minDR < dRcut && matchedJet >= 0) {
                nBmatched++;
                hMatchedBpt->Fill(bPt);
                hMatchedJetPt->Fill(jet_pt[matchedJet]);
                hBvsJetPt->Fill(jet_pt[matchedJet], bPt);
                hBmassMatched->Fill(bMass);
            }
        }
    }
    
    std::cout << "\n=== Matching Summary ===" << std::endl;
    std::cout << "Total B candidates: " << nBtotal << std::endl;
    std::cout << "Matched to jets (dR < " << dRcut << "): " << nBmatched << std::endl;
    std::cout << "Matching efficiency: " << (nBtotal > 0 ? 100.0*nBmatched/nBtotal : 0) << "%" << std::endl;
    
    // Draw
    TCanvas *c1 = new TCanvas("c1", "B-Jet Matching", 1200, 800);
    c1->Divide(3, 2);
    
    c1->cd(1);
    hDeltaR->Draw();
    
    c1->cd(2);
    hBmass->SetLineColor(kBlue);
    hBmass->Draw();
    hBmassMatched->SetLineColor(kRed);
    hBmassMatched->Draw("same");
    
    c1->cd(3);
    hBvsJetPt->Draw("colz");
    
    c1->cd(4);
    hMatchedBpt->Draw();
    
    c1->cd(5);
    hMatchedJetPt->Draw();
    
    c1->SaveAs("BmesonJetMatching.png");
    std::cout << "Saved BmesonJetMatching.png" << std::endl;
    
    f->Close();
}

// Example for D0 matching
void matchD0Jet(TString inputFile = "HiForestOO_OniaBmesonDmeson1k.root") {
    matchBmesonJet(inputFile, "d0Ntuplizer/candTree", "akCs4PFJetAnalyzer/t", 0.3);
}
