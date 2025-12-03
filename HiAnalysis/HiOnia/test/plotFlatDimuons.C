// Example analysis script for flattened dimuon tree
// Usage: root -l -b -q 'plotFlatDimuons.C("FlatDimuonTree.root")'

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
    
    // Example selections
    TCut baseCut = "QQ_sign == 0";  // Opposite sign
    TCut vtxCut = "QQ_VtxProb > 0.01";  // Good vertex
    TCut kineCut = "mupl_pt > 3.5 && mumi_pt > 3.5 && abs(mupl_eta) < 2.4 && abs(mumi_eta) < 2.4";
    TCut idCut = "mupl_isSoftCutBased && mumi_isSoftCutBased";  // Soft muon ID
    TCut qualCut = "mupl_highPurity && mumi_highPurity";  // High purity tracks
    
    TCut looseCut = baseCut && vtxCut && kineCut;
    TCut tightCut = looseCut && idCut && qualCut;
    
    // Create canvas
    TCanvas* c1 = new TCanvas("c1", "Dimuon Analysis", 1600, 1200);
    c1->Divide(3, 2);
    
    // 1. Invariant mass (loose cuts)
    c1->cd(1);
    tree->Draw("QQ_mass>>h1(100, 2.5, 3.5)", looseCut, "");
    TH1F* h1 = (TH1F*)gDirectory->Get("h1");
    h1->SetTitle("J/#psi #rightarrow #mu^{+}#mu^{-} (Loose);M_{#mu#mu} [GeV/c^{2}];Candidates");
    h1->SetLineColor(kBlue);
    h1->SetLineWidth(2);
    
    // 2. Invariant mass (tight cuts)
    c1->cd(2);
    tree->Draw("QQ_mass>>h2(100, 2.5, 3.5)", tightCut, "");
    TH1F* h2 = (TH1F*)gDirectory->Get("h2");
    h2->SetTitle("J/#psi #rightarrow #mu^{+}#mu^{-} (Tight);M_{#mu#mu} [GeV/c^{2}];Candidates");
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
    h6->SetLogy();
    
    c1->SaveAs("FlatDimuon_Plots.png");
    
    // Print statistics
    std::cout << "\n=== Selection Statistics ===" << std::endl;
    std::cout << "Total candidates: " << tree->GetEntries() << std::endl;
    std::cout << "After loose cuts: " << tree->GetEntries(looseCut) << std::endl;
    std::cout << "After tight cuts: " << tree->GetEntries(tightCut) << std::endl;
    
    // Print some example events
    std::cout << "\n=== Example Candidates (tight cuts) ===" << std::endl;
    tree->Scan("runNb:eventNb:QQ_mass:QQ_pt:mupl_pt:mumi_pt:mupl_isSoftCutBased:mumi_isSoftCutBased", 
               tightCut, "", 10);
    
    std::cout << "\nPlots saved to: FlatDimuon_Plots.png" << std::endl;
}
