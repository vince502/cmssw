// Example analysis script for flattened dimuon tree
// Usage: root -l -b -q 'plotFlatDimuons.C("FlatDimuonTree.root")'

#include <fstream>
#include <iostream>

void plotFlatDimuons(const char* inputFile = "FlatDimuonTree.root") {
    
    TFile* f = TFile::Open(inputFile);
    if (!f || f->IsZombie()) {
        std::cerr << "Cannot open " << inputFile << std::endl;
        return;
    }
    
    TTree* tree = (TTree*)f->Get("dimuonTree");
    if (!tree) {
        std::cerr << "Cannot find dimuonTree!" << std::endl;
        return;
    }
    
    std::cout << "Tree has " << tree->GetEntries() << " dimuon candidates" << std::endl;
    
    // Bc semileptonic inspection selections (J/psi + psi(2S) window)
    TCut baseCut = "QQ_sign == 0";  // Opposite sign
    TCut massWindow = "QQ_mass > 2.2 && QQ_mass < 4.0";
    TCut jpsiWindow = "QQ_mass > 2.9 && QQ_mass < 3.3";
    TCut psi2sWindow = "QQ_mass > 3.55 && QQ_mass < 3.85";
    TCut vtxCut = "QQ_VtxProb > 0.01";  // Good vertex
    TCut kineCut = "mupl_pt > 3.5 && mumi_pt > 3.5 && abs(mupl_eta) < 2.4 && abs(mumi_eta) < 2.4";
    TCut idCut = "mupl_isSoftCutBased && mumi_isSoftCutBased";  // Soft muon ID
    TCut qualCut = "mupl_highPurity && mumi_highPurity";  // High purity tracks
    
    TCut looseCut = baseCut && massWindow && vtxCut && kineCut;
    TCut tightCut = looseCut && idCut && qualCut;
    
    // Create canvas
    TCanvas* c1 = new TCanvas("c1", "Dimuon Analysis", 1600, 1200);
    c1->Divide(3, 2);
    
    // 1. Invariant mass (loose cuts) in 2.2-4.0 GeV
    c1->cd(1);
    tree->Draw("QQ_mass>>h1(180, 2.2, 4.0)", looseCut, "");
    TH1F* h1 = (TH1F*)gDirectory->Get("h1");
    h1->SetTitle("J/#psi + #psi(2S) #rightarrow #mu^{+}#mu^{-} (Loose);M_{#mu#mu} [GeV/c^{2}];Candidates");
    h1->SetLineColor(kBlue);
    h1->SetLineWidth(2);
    
    // 2. Invariant mass (tight cuts) in 2.2-4.0 GeV
    c1->cd(2);
    tree->Draw("QQ_mass>>h2(180, 2.2, 4.0)", tightCut, "");
    TH1F* h2 = (TH1F*)gDirectory->Get("h2");
    h2->SetTitle("J/#psi + #psi(2S) #rightarrow #mu^{+}#mu^{-} (Tight);M_{#mu#mu} [GeV/c^{2}];Candidates");
    h2->SetLineColor(kRed);
    h2->SetLineWidth(2);
    
    // 3. pT distribution
    c1->cd(3);
    tree->Draw("QQ_pt>>h3(100, 0, 30)", looseCut, "");
    TH1F* h3 = (TH1F*)gDirectory->Get("h3");
    h3->SetTitle("Dimuon p_{T};p_{T}^{#mu#mu} [GeV/c];Candidates");
    h3->SetLineColor(kGreen+2);
    h3->SetLineWidth(2);
    
    // 4. Muon pT vs pT
    c1->cd(4);
    tree->Draw("mupl_pt:mumi_pt>>h4(50, 0, 20, 50, 0, 20)", baseCut && vtxCut, "colz");
    TH2F* h4 = (TH2F*)gDirectory->Get("h4");
    h4->SetTitle("Muon kinematics;#mu^{-} p_{T} [GeV/c];#mu^{+} p_{T} [GeV/c]");
    
    // 5. Soft MVA vs pT
    c1->cd(5);
    tree->Draw("mupl_softMvaRun3Value:mupl_pt>>h5(50, 0, 20, 50, 0, 1)", baseCut && vtxCut, "colz");
    TH2F* h5 = (TH2F*)gDirectory->Get("h5");
    h5->SetTitle("Soft MVA ID;#mu^{+} p_{T} [GeV/c];Soft MVA Run3 Value");
    
    // 6. Vertex probability
    c1->cd(6);
    tree->Draw("QQ_VtxProb>>h6(100, 0, 1)", baseCut, "");
    TH1F* h6 = (TH1F*)gDirectory->Get("h6");
    h6->SetTitle("Vertex Probability;Vertex Prob;Candidates");
    h6->SetLineColor(kMagenta+2);
    h6->SetLineWidth(2);
    gPad->SetLogy();
    
    c1->SaveAs("FlatDimuon_JpsiPsi2S_Plots.png");
    c1->SaveAs("FlatDimuon_JpsiPsi2S_Plots.pdf");
    
    // Print statistics
    Long64_t nTotal = tree->GetEntries();
    Long64_t nOS = tree->GetEntries(baseCut);
    Long64_t nMass = tree->GetEntries(baseCut && massWindow);
    Long64_t nLoose = tree->GetEntries(looseCut);
    Long64_t nTight = tree->GetEntries(tightCut);
    Long64_t nJpsiLoose = tree->GetEntries(looseCut && jpsiWindow);
    Long64_t nPsi2SLoose = tree->GetEntries(looseCut && psi2sWindow);
    Long64_t nJpsiTight = tree->GetEntries(tightCut && jpsiWindow);
    Long64_t nPsi2STight = tree->GetEntries(tightCut && psi2sWindow);

    std::cout << "\n=== Selection Statistics ===" << std::endl;
    std::cout << "Total candidates: " << nTotal << std::endl;
    std::cout << "Opposite-sign: " << nOS << std::endl;
    std::cout << "In 2.2-4.0 GeV: " << nMass << std::endl;
    std::cout << "After loose cuts: " << nLoose << std::endl;
    std::cout << "After tight cuts: " << nTight << std::endl;
    std::cout << "J/psi loose/tight: " << nJpsiLoose << " / " << nJpsiTight << std::endl;
    std::cout << "psi(2S) loose/tight: " << nPsi2SLoose << " / " << nPsi2STight << std::endl;
    
    // Print some example events
    std::cout << "\n=== Example Candidates (tight cuts) ===" << std::endl;
    tree->Scan("runNb:eventNb:QQ_mass:QQ_pt:mupl_pt:mumi_pt:mupl_isSoftCutBased:mumi_isSoftCutBased", 
               tightCut, "", 10);
    
    // Write plain-text summary for quick inspection
    std::ofstream txt("FlatDimuon_JpsiPsi2S_Summary.txt");
    txt << "Flat dimuon summary (Bc semileptonic inspection)\n";
    txt << "Input file: " << inputFile << "\n";
    txt << "Mass window: 2.2 < m(mumu) < 4.0 GeV\n";
    txt << "J/psi window: 2.9 < m(mumu) < 3.3 GeV\n";
    txt << "psi(2S) window: 3.55 < m(mumu) < 3.85 GeV\n";
    txt << "Total candidates: " << nTotal << "\n";
    txt << "Opposite-sign: " << nOS << "\n";
    txt << "In 2.2-4.0 GeV: " << nMass << "\n";
    txt << "Loose: " << nLoose << "\n";
    txt << "Tight: " << nTight << "\n";
    txt << "J/psi loose: " << nJpsiLoose << "\n";
    txt << "J/psi tight: " << nJpsiTight << "\n";
    txt << "psi(2S) loose: " << nPsi2SLoose << "\n";
    txt << "psi(2S) tight: " << nPsi2STight << "\n";
    txt.close();

    std::cout << "\nPlots saved to: FlatDimuon_JpsiPsi2S_Plots.png/.pdf" << std::endl;
    std::cout << "Text summary: FlatDimuon_JpsiPsi2S_Summary.txt" << std::endl;
}
