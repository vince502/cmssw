// Draw Z peak from DielectronTree with quality cuts
// Usage: root -l -b -q 'drawZpeak.C("FlatOniaTree.root")'

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TCut.h>
#include <iostream>

void drawZpeak(const char* inputFile = "FlatOniaTree.root") {
    
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1);
    
    TFile* f = TFile::Open(inputFile, "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "Error: Cannot open " << inputFile << std::endl;
        return;
    }
    
    TTree* tree = (TTree*)f->Get("DielectronTree");
    if (!tree) {
        std::cerr << "Error: Cannot find DielectronTree!" << std::endl;
        return;
    }
    
    std::cout << "DielectronTree entries: " << tree->GetEntries() << std::endl;
    
    // ========================================
    // Define quality cuts
    // ========================================
    
    // Basic kinematic cuts
    TCut kineCut = "ep_pt > 20 && em_pt > 20 && abs(ep_eta) < 2.5 && abs(em_eta) < 2.5";
    
    // Electron ID cuts (loose)
    TCut ep_idLoose = "ep_sigmaIEtaIEta < 0.015 && ep_hOverE < 0.15 && abs(ep_deltaEtaIn) < 0.01 && abs(ep_deltaPhiIn) < 0.1";
    TCut em_idLoose = "em_sigmaIEtaIEta < 0.015 && em_hOverE < 0.15 && abs(em_deltaEtaIn) < 0.01 && abs(em_deltaPhiIn) < 0.1";
    
    // Electron ID cuts (tight)
    TCut ep_idTight = "ep_sigmaIEtaIEta < 0.011 && ep_hOverE < 0.10 && abs(ep_deltaEtaIn) < 0.007 && abs(ep_deltaPhiIn) < 0.06";
    TCut em_idTight = "em_sigmaIEtaIEta < 0.011 && em_hOverE < 0.10 && abs(em_deltaEtaIn) < 0.007 && abs(em_deltaPhiIn) < 0.06";
    
    // Conversion veto
    TCut convVeto = "ep_convVeto == 1 && em_convVeto == 1";
    
    // Missing hits cut
    TCut missingHits = "ep_nMissingHits <= 1 && em_nMissingHits <= 1";
    
    // Isolation cuts (relative isolation < 0.15)
    TCut ep_iso = "(ep_pfChIso + ep_pfNeuIso + ep_pfPhoIso) / ep_pt < 0.15";
    TCut em_iso = "(em_pfChIso + em_pfNeuIso + em_pfPhoIso) / em_pt < 0.15";
    
    // Combined cuts
    TCut looseCut = kineCut + ep_idLoose + em_idLoose;
    TCut mediumCut = looseCut + convVeto + missingHits;
    TCut tightCut = kineCut + ep_idTight + em_idTight + convVeto + missingHits + ep_iso + em_iso;
    
    // Mass window for Z
    TCut massWindow = "mass > 60 && mass < 120";
    
    // ========================================
    // Create histograms
    // ========================================
    TH1F* h_nocut = new TH1F("h_nocut", "No cuts", 60, 60, 120);
    TH1F* h_loose = new TH1F("h_loose", "Loose ID", 60, 60, 120);
    TH1F* h_medium = new TH1F("h_medium", "Medium ID", 60, 60, 120);
    TH1F* h_tight = new TH1F("h_tight", "Tight ID", 60, 60, 120);
    
    h_nocut->SetLineColor(kBlack);
    h_loose->SetLineColor(kBlue);
    h_medium->SetLineColor(kGreen+2);
    h_tight->SetLineColor(kRed);
    
    h_nocut->SetLineWidth(2);
    h_loose->SetLineWidth(2);
    h_medium->SetLineWidth(2);
    h_tight->SetLineWidth(2);
    
    // ========================================
    // Fill histograms
    // ========================================
    std::cout << "Drawing with no cuts..." << std::endl;
    tree->Draw("mass >> h_nocut", massWindow, "goff");
    std::cout << "  Entries: " << h_nocut->GetEntries() << std::endl;
    
    std::cout << "Drawing with loose cuts..." << std::endl;
    tree->Draw("mass >> h_loose", massWindow + looseCut, "goff");
    std::cout << "  Entries: " << h_loose->GetEntries() << std::endl;
    
    std::cout << "Drawing with medium cuts..." << std::endl;
    tree->Draw("mass >> h_medium", massWindow + mediumCut, "goff");
    std::cout << "  Entries: " << h_medium->GetEntries() << std::endl;
    
    std::cout << "Drawing with tight cuts..." << std::endl;
    tree->Draw("mass >> h_tight", massWindow + tightCut, "goff");
    std::cout << "  Entries: " << h_tight->GetEntries() << std::endl;
    
    // ========================================
    // Draw canvas
    // ========================================
    TCanvas* c1 = new TCanvas("c1", "Z Peak", 800, 600);
    c1->SetLeftMargin(0.12);
    c1->SetRightMargin(0.05);
    c1->SetTopMargin(0.08);
    c1->SetBottomMargin(0.12);
    
    // Find maximum for y-axis
    float ymax = h_nocut->GetMaximum() * 1.3;
    
    h_nocut->SetTitle("");
    h_nocut->GetXaxis()->SetTitle("m_{ee} (GeV)");
    h_nocut->GetYaxis()->SetTitle("Entries / 1 GeV");
    h_nocut->GetXaxis()->SetTitleSize(0.05);
    h_nocut->GetYaxis()->SetTitleSize(0.05);
    h_nocut->GetXaxis()->SetLabelSize(0.04);
    h_nocut->GetYaxis()->SetLabelSize(0.04);
    h_nocut->SetMaximum(ymax);
    
    h_nocut->Draw("hist");
    h_loose->Draw("hist same");
    h_medium->Draw("hist same");
    h_tight->Draw("hist same");
    
    // Legend
    TLegend* leg = new TLegend(0.65, 0.65, 0.92, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->SetTextSize(0.035);
    leg->AddEntry(h_nocut, Form("No cuts (%d)", (int)h_nocut->GetEntries()), "l");
    leg->AddEntry(h_loose, Form("Loose ID (%d)", (int)h_loose->GetEntries()), "l");
    leg->AddEntry(h_medium, Form("Medium ID (%d)", (int)h_medium->GetEntries()), "l");
    leg->AddEntry(h_tight, Form("Tight ID (%d)", (int)h_tight->GetEntries()), "l");
    leg->Draw();
    
    // Labels
    TLatex latex;
    latex.SetNDC();
    latex.SetTextSize(0.045);
    latex.SetTextFont(62);
    latex.DrawLatex(0.15, 0.94, "CMS");
    latex.SetTextFont(52);
    latex.DrawLatex(0.23, 0.94, "Preliminary");
    latex.SetTextFont(42);
    latex.SetTextSize(0.035);
    latex.DrawLatex(0.65, 0.94, "PbPb 2025");
    
    c1->SaveAs("Zpeak_ee.png");
    c1->SaveAs("Zpeak_ee.pdf");
    
    // ========================================
    // Additional plots: pt and eta distributions
    // ========================================
    TCanvas* c2 = new TCanvas("c2", "Electron kinematics", 1200, 500);
    c2->Divide(3, 1);
    
    // pt distribution
    c2->cd(1);
    TH1F* h_pt = new TH1F("h_pt", ";Electron p_{T} (GeV);Entries", 50, 0, 100);
    h_pt->SetLineColor(kBlue);
    h_pt->SetLineWidth(2);
    tree->Draw("ep_pt >> h_pt", tightCut + massWindow, "goff");
    tree->Draw("em_pt >>+ h_pt", tightCut + massWindow, "goff");
    h_pt->Draw("hist");
    
    // eta distribution
    c2->cd(2);
    TH1F* h_eta = new TH1F("h_eta", ";Electron #eta;Entries", 50, -2.5, 2.5);
    h_eta->SetLineColor(kBlue);
    h_eta->SetLineWidth(2);
    tree->Draw("ep_eta >> h_eta", tightCut + massWindow, "goff");
    tree->Draw("em_eta >>+ h_eta", tightCut + massWindow, "goff");
    h_eta->Draw("hist");
    
    // Z pt distribution
    c2->cd(3);
    TH1F* h_Zpt = new TH1F("h_Zpt", ";Z p_{T} (GeV);Entries", 50, 0, 100);
    h_Zpt->SetLineColor(kRed);
    h_Zpt->SetLineWidth(2);
    tree->Draw("pt >> h_Zpt", tightCut + massWindow, "goff");
    h_Zpt->Draw("hist");
    
    c2->SaveAs("Zpeak_kinematics.png");
    c2->SaveAs("Zpeak_kinematics.pdf");
    
    // ========================================
    // Print cut flow
    // ========================================
    std::cout << "\n=== Cut Flow ===" << std::endl;
    std::cout << "Mass window (60-120 GeV):  " << h_nocut->GetEntries() << std::endl;
    std::cout << "+ Loose ID:                " << h_loose->GetEntries() 
              << " (eff: " << 100.*h_loose->GetEntries()/h_nocut->GetEntries() << "%)" << std::endl;
    std::cout << "+ Conv veto + missing hits:" << h_medium->GetEntries() 
              << " (eff: " << 100.*h_medium->GetEntries()/h_nocut->GetEntries() << "%)" << std::endl;
    std::cout << "+ Tight ID + Isolation:    " << h_tight->GetEntries() 
              << " (eff: " << 100.*h_tight->GetEntries()/h_nocut->GetEntries() << "%)" << std::endl;
    
    std::cout << "\nOutput saved to Zpeak_ee.png/pdf and Zpeak_kinematics.png/pdf" << std::endl;
    
    f->Close();
}
