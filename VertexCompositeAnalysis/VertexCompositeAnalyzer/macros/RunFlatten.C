//////////////////////////////////////////////////////////
// RunFlatten.C
// Simple execution script for FlattenBDiMuMu macro
// Usage examples and command line interface
//////////////////////////////////////////////////////////

#include <iostream>
#include <string>

// Load the flattening macro
void LoadFlattenMacro() {
    gROOT->ProcessLine("#include \"FlattenBDiMuMu.C\"");
}

// Basic usage with default files
void RunFlatten() {
    std::cout << "=== RunFlatten.C - Basic Usage ===" << std::endl;
    std::cout << "Using default input/output files:" << std::endl;
    std::cout << "Input:  BDiMuMu_Ntuple.root" << std::endl;  
    std::cout << "Output: BDiMuMu_Flat.root" << std::endl;
    std::cout << std::endl;
    
    LoadFlattenMacro();
    FlattenBDiMuMu();
}

// Advanced usage with custom files
void RunFlatten(const char* inputFile, const char* outputFile) {
    std::cout << "=== RunFlatten.C - Custom Files ===" << std::endl;
    std::cout << "Input:  " << inputFile << std::endl;
    std::cout << "Output: " << outputFile << std::endl;
    std::cout << std::endl;
    
    LoadFlattenMacro();
    FlattenBDiMuMu(inputFile, outputFile);
}

// Batch processing example
void RunFlattenBatch() {
    std::cout << "=== RunFlatten.C - Batch Processing Example ===" << std::endl;
    
    // Example file list
    std::vector<std::string> inputFiles = {
        "BDiMuMu_Run1.root",
        "BDiMuMu_Run2.root", 
        "BDiMuMu_Run3.root"
    };
    
    LoadFlattenMacro();
    
    for (size_t i = 0; i < inputFiles.size(); i++) {
        std::string outputFile = "BDiMuMu_Flat_Run" + std::to_string(i+1) + ".root";
        
        std::cout << "Processing file " << (i+1) << "/" << inputFiles.size() << std::endl;
        std::cout << "Input:  " << inputFiles[i] << std::endl;
        std::cout << "Output: " << outputFile << std::endl;
        
        FlattenBDiMuMu(inputFiles[i].c_str(), outputFile.c_str());
        
        std::cout << "Completed file " << (i+1) << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }
    
    std::cout << "Batch processing complete!" << std::endl;
}

// Quick analysis example after flattening
void RunFlattenAndAnalyze(const char* inputFile = "BDiMuMu_Ntuple.root", 
                          const char* outputFile = "BDiMuMu_Flat.root") {
    
    std::cout << "=== RunFlatten.C - Flatten and Quick Analysis ===" << std::endl;
    
    // Step 1: Flatten the tree
    std::cout << "Step 1: Flattening tree..." << std::endl;
    LoadFlattenMacro();
    FlattenBDiMuMu(inputFile, outputFile);
    
    // Step 2: Quick analysis
    std::cout << std::endl << "Step 2: Quick analysis..." << std::endl;
    
    TFile* f = TFile::Open(outputFile, "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "Error: Cannot open flattened file " << outputFile << std::endl;
        return;
    }
    
    TTree* flatTree = (TTree*)f->Get("bDiMuMuFlatTree");
    if (!flatTree) {
        std::cerr << "Error: Cannot find bDiMuMuFlatTree in " << outputFile << std::endl;
        return;
    }
    
    std::cout << "Flat tree contains " << flatTree->GetEntries() << " B meson candidates" << std::endl;
    
    // Count B meson types
    Long64_t nBPlus = flatTree->GetEntries("bMesonType == 0");
    Long64_t nBZero = flatTree->GetEntries("bMesonType == 1"); 
    Long64_t nBc = flatTree->GetEntries("bMesonType == 2");
    
    std::cout << "B meson breakdown:" << std::endl;
    std::cout << "  B+ candidates: " << nBPlus << std::endl;
    std::cout << "  B0 candidates: " << nBZero << std::endl;
    std::cout << "  Bc candidates: " << nBc << std::endl;
    
    // Basic kinematic distributions
    std::cout << std::endl << "Creating basic plots..." << std::endl;
    
    TCanvas* c1 = new TCanvas("c1", "B meson mass distributions", 1200, 400);
    c1->Divide(3, 1);
    
    c1->cd(1);
    flatTree->Draw("bMesonMass", "bMesonType == 0", "");
    gPad->SetTitle("B+ mass");
    
    c1->cd(2);
    flatTree->Draw("bMesonMass", "bMesonType == 1", "");
    gPad->SetTitle("B0 mass");
    
    c1->cd(3);
    flatTree->Draw("bMesonMass", "bMesonType == 2", "");
    gPad->SetTitle("Bc mass");
    
    c1->SaveAs("BDiMuMu_MassDistributions.png");
    
    TCanvas* c2 = new TCanvas("c2", "Kinematic distributions", 1200, 800);
    c2->Divide(2, 2);
    
    c2->cd(1);
    flatTree->Draw("bMesonPt", "", "");
    gPad->SetTitle("B meson pT");
    
    c2->cd(2);
    flatTree->Draw("dimuonMass", "", "");
    gPad->SetTitle("Dimuon mass (J/ψ)");
    
    c2->cd(3);
    flatTree->Draw("muon1Pt", "", "");
    gPad->SetTitle("Leading muon pT");
    
    c2->cd(4);
    flatTree->Draw("track1Pt", "nTracks >= 1", "");
    gPad->SetTitle("Track pT");
    
    c2->SaveAs("BDiMuMu_Kinematics.png");
    
    std::cout << "Plots saved as BDiMuMu_MassDistributions.png and BDiMuMu_Kinematics.png" << std::endl;
    
    f->Close();
    delete f;
    
    std::cout << std::endl << "Analysis complete!" << std::endl;
    std::cout << "You can now perform detailed analysis on the flat tree:" << std::endl;
    std::cout << "  root> TFile* f = TFile::Open(\"" << outputFile << "\");" << std::endl;
    std::cout << "  root> TTree* t = (TTree*)f->Get(\"bDiMuMuFlatTree\");" << std::endl;
    std::cout << "  root> t->Draw(\"bMesonMass\", \"muon1Pt > 4 && track1Pt > 1\");" << std::endl;
}

// Help function showing usage examples
void ShowUsage() {
    std::cout << "=== FlattenBDiMuMu Usage Examples ===" << std::endl;
    std::cout << std::endl;
    std::cout << "1. Basic usage (default files):" << std::endl;
    std::cout << "   root -l 'RunFlatten.C()'" << std::endl;
    std::cout << std::endl;
    std::cout << "2. Custom input/output files:" << std::endl;
    std::cout << "   root -l 'RunFlatten.C(\"myInput.root\", \"myOutput.root\")'" << std::endl;
    std::cout << std::endl;
    std::cout << "3. Direct flattening:" << std::endl;
    std::cout << "   root -l 'FlattenBDiMuMu.C(\"input.root\", \"output.root\")'" << std::endl;
    std::cout << std::endl;
    std::cout << "4. Batch processing:" << std::endl;
    std::cout << "   root -l 'RunFlatten.C' -e 'RunFlattenBatch()'" << std::endl;
    std::cout << std::endl;
    std::cout << "5. Flatten and analyze:" << std::endl;
    std::cout << "   root -l 'RunFlatten.C' -e 'RunFlattenAndAnalyze()'" << std::endl;
    std::cout << std::endl;
    std::cout << "6. Show this help:" << std::endl;
    std::cout << "   root -l 'RunFlatten.C' -e 'ShowUsage()'" << std::endl;
    std::cout << std::endl;
    std::cout << "Input file should contain: bDiMuMuNtuplizer/bDiMuMuTree" << std::endl;
    std::cout << "Output file will contain: bDiMuMuFlatTree (flat structure)" << std::endl;
    std::cout << std::endl;
    std::cout << "Flat tree variables:" << std::endl;
    std::cout << "  Event info: runNb, eventNb, centrality, nPV, bestvx/y/z" << std::endl;
    std::cout << "  B meson: bMesonType, bMesonPt/Eta/Phi/Mass/Charge" << std::endl;
    std::cout << "  Vertex: bMesonVtxChi2/Ndof/Prob, bMesonVtxX/Y/Z" << std::endl;
    std::cout << "  Topology: bMesonPointingAngle2D/3D, bMesonDecayLength2D/3D" << std::endl;
    std::cout << "  Dimuon: dimuonPt/Eta/Phi/Mass/Charge" << std::endl;
    std::cout << "  Muons: muon1*/muon2* (leading/subleading by pT)" << std::endl;
    std::cout << "  Tracks: track1*/track2* (additional tracks for B reconstruction)" << std::endl;
    std::cout << "  K*0: kstarPt/Eta/Phi/Mass (for B0 only)" << std::endl;
}