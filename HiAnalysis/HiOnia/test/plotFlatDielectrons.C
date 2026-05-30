// Example analysis script for flattened dielectron tree
// Usage: root -l -b -q 'plotFlatDielectrons.C("FlatDielectronTree.root")'

#include <fstream>
#include <iostream>

void plotFlatDielectrons(const char* inputFile = "FlatDielectronTree.root") {
    
    TFile* f = TFile::Open(inputFile);
    if (!f || f->IsZombie()) {
        std::cerr << "Cannot open " << inputFile << std::endl;
        return;
    }
    
    TTree* tree = (TTree*)f->Get("dielectronTree");
    if (!tree) {
        std::cerr << "Cannot find dielectronTree!" << std::endl;
        return;
    }
    
    std::cout << "Tree has " << tree->GetEntries() << " dielectron candidates" << std::endl;
    
    // Bc semileptonic inspection selections (J/psi + psi(2S) window)
    TCut baseCut = "ee_charge == 0";  // Opposite sign
    TCut massWindow = "ee_mass > 2.2 && ee_mass < 4.0";
    TCut jpsiWindow = "ee_mass > 2.9 && ee_mass < 3.3";
    TCut psi2sWindow = "ee_mass > 3.55 && ee_mass < 3.85";
    TCut vtxCut = "ee_vProb > 0.01";  // Good vertex
    TCut kineCut = "ele1_pt > 3.0 && ele2_pt > 3.0 && abs(ele1_eta) < 2.4 && abs(ele2_eta) < 2.4";
    TCut isoCut = "ele1_pfChIso < 5.0 && ele2_pfChIso < 5.0";  // Loose isolation
    TCut convCut = "ele1_convVeto == 1 && ele2_convVeto == 1";  // Conversion veto
    TCut hOverECut = "ele1_hOverE < 0.15 && ele2_hOverE < 0.15";  // H/E cut
    
    TCut looseCut = baseCut && massWindow && vtxCut && kineCut;
    TCut tightCut = looseCut && isoCut && convCut && hOverECut;
    
    // Create canvas
    TCanvas* c1 = new TCanvas("c1", "Dielectron Analysis", 1600, 1200);
    c1->Divide(3, 2);
    
    // 1. Invariant mass (loose cuts) in 2.2-4.0 GeV
    c1->cd(1);
    tree->Draw("ee_mass>>h1(180, 2.2, 4.0)", looseCut, "");
    TH1F* h1 = (TH1F*)gDirectory->Get("h1");
    h1->SetTitle("J/#psi + #psi(2S) #rightarrow e^{+}e^{-} (Loose);M_{ee} [GeV/c^{2}];Candidates");
    h1->SetLineColor(kBlue);
    h1->SetLineWidth(2);
    
    // 2. Invariant mass (tight cuts) in 2.2-4.0 GeV
    c1->cd(2);
    tree->Draw("ee_mass>>h2(180, 2.2, 4.0)", tightCut, "");
    TH1F* h2 = (TH1F*)gDirectory->Get("h2");
    h2->SetTitle("J/#psi + #psi(2S) #rightarrow e^{+}e^{-} (Tight);M_{ee} [GeV/c^{2}];Candidates");
    h2->SetLineColor(kRed);
    h2->SetLineWidth(2);
    
    // 3. pT distribution
    c1->cd(3);
    tree->Draw("ee_pt>>h3(100, 0, 30)", looseCut, "");
    TH1F* h3 = (TH1F*)gDirectory->Get("h3");
    h3->SetTitle("Dielectron p_{T};p_{T}^{ee} [GeV/c];Candidates");
    h3->SetLineColor(kGreen+2);
    h3->SetLineWidth(2);
    
    // 4. Electron isolation
    c1->cd(4);
    tree->Draw("ele1_pfChIso:ele2_pfChIso>>h4(50, 0, 10, 50, 0, 10)", baseCut && vtxCut, "colz");
    TH2F* h4 = (TH2F*)gDirectory->Get("h4");
    h4->SetTitle("Electron Isolation;e^{-} PF Ch Iso;e^{+} PF Ch Iso");
    
    // 5. H/E vs sigma ieta ieta
    c1->cd(5);
    tree->Draw("ele1_hOverE:ele1_sigmaIEtaIEta>>h5(50, 0, 0.05, 50, 0, 0.3)", baseCut && vtxCut, "colz");
    TH2F* h5 = (TH2F*)gDirectory->Get("h5");
    h5->SetTitle("Electron Shower Shape;#sigma_{i#etai#eta};H/E");
    
    // 6. Vertex probability
    c1->cd(6);
    tree->Draw("ee_vProb>>h6(100, 0, 1)", baseCut, "");
    TH1F* h6 = (TH1F*)gDirectory->Get("h6");
    h6->SetTitle("Vertex Probability;Vertex Prob;Candidates");
    h6->SetLineColor(kMagenta+2);
    h6->SetLineWidth(2);
    gPad->SetLogy();
    
    c1->SaveAs("FlatDielectron_JpsiPsi2S_Plots.png");
    c1->SaveAs("FlatDielectron_JpsiPsi2S_Plots.pdf");
    
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
    tree->Scan("runNb:eventNb:ee_mass:ee_pt:ele1_pt:ele2_pt:ele1_pfChIso:ele2_pfChIso", 
               tightCut, "", 10);
    
    // Write plain-text summary for quick inspection
    std::ofstream txt("FlatDielectron_JpsiPsi2S_Summary.txt");
    txt << "Flat dielectron summary (Bc semileptonic inspection)\n";
    txt << "Input file: " << inputFile << "\n";
    txt << "Mass window: 2.2 < m(ee) < 4.0 GeV\n";
    txt << "J/psi window: 2.9 < m(ee) < 3.3 GeV\n";
    txt << "psi(2S) window: 3.55 < m(ee) < 3.85 GeV\n";
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

    std::cout << "\nPlots saved to: FlatDielectron_JpsiPsi2S_Plots.png/.pdf" << std::endl;
    std::cout << "Text summary: FlatDielectron_JpsiPsi2S_Summary.txt" << std::endl;
}
