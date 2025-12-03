// flattenCompositeTree.C
// Flatten event-based composite candidate trees to candidate-level trees
// Usage: root -l -b -q 'flattenCompositeTree.C("input.root", "d0Ntuplizer/candTree", "output.root")'

#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <vector>
#include <iostream>

void flattenCompositeTree(
    TString inputFile = "HiForestOO_OniaBmesonDmeson1k.root",
    TString treePath = "d0Ntuplizer/candTree",
    TString outputFile = "flatD0.root"
) {
    // Open input file
    TFile *fIn = TFile::Open(inputFile);
    if (!fIn || fIn->IsZombie()) {
        std::cout << "ERROR: Cannot open input file " << inputFile << std::endl;
        return;
    }
    
    TTree *tIn = (TTree*)fIn->Get(treePath);
    if (!tIn) {
        std::cout << "ERROR: Cannot find tree " << treePath << std::endl;
        fIn->Close();
        return;
    }
    
    std::cout << "Input tree: " << treePath << " with " << tIn->GetEntries() << " events" << std::endl;
    
    // Event-level variables
    UInt_t runNb, lsNb, eventNb, candSize;
    Short_t nPV, centrality;
    Int_t Ntrkoffline;
    Float_t bestvtxX, bestvtxY, bestvtxZ;
    
    // Candidate vectors
    std::vector<float> *pT = nullptr;
    std::vector<float> *eta = nullptr;
    std::vector<float> *phi = nullptr;
    std::vector<float> *mass = nullptr;
    std::vector<float> *y = nullptr;
    std::vector<int> *pdgId = nullptr;
    std::vector<float> *mva = nullptr;
    
    // Vertex vectors
    std::vector<float> *VtxProb = nullptr;
    std::vector<float> *VtxChi2 = nullptr;
    std::vector<float> *alpha3D = nullptr;
    std::vector<float> *decayLength3D = nullptr;
    std::vector<float> *decayLengthSig3D = nullptr;
    
    // Daughter vectors (vector of vectors)
    std::vector<std::vector<float>> *pTD = nullptr;
    std::vector<std::vector<float>> *etaD = nullptr;
    std::vector<std::vector<float>> *phiD = nullptr;
    std::vector<std::vector<short>> *chargeD = nullptr;
    std::vector<std::vector<float>> *dedxD = nullptr;
    
    // Set branch addresses - event level
    tIn->SetBranchAddress("RunNb", &runNb);
    tIn->SetBranchAddress("LSNb", &lsNb);
    tIn->SetBranchAddress("EventNb", &eventNb);
    tIn->SetBranchAddress("candSize", &candSize);
    tIn->SetBranchAddress("nPV", &nPV);
    tIn->SetBranchAddress("bestvtxX", &bestvtxX);
    tIn->SetBranchAddress("bestvtxY", &bestvtxY);
    tIn->SetBranchAddress("bestvtxZ", &bestvtxZ);
    
    // Optional branches
    if (tIn->GetBranch("centrality")) tIn->SetBranchAddress("centrality", &centrality);
    if (tIn->GetBranch("Ntrkoffline")) tIn->SetBranchAddress("Ntrkoffline", &Ntrkoffline);
    
    // Candidate vectors
    tIn->SetBranchAddress("pT", &pT);
    tIn->SetBranchAddress("eta", &eta);
    tIn->SetBranchAddress("phi", &phi);
    tIn->SetBranchAddress("mass", &mass);
    tIn->SetBranchAddress("y", &y);
    if (tIn->GetBranch("pdgId")) tIn->SetBranchAddress("pdgId", &pdgId);
    if (tIn->GetBranch("mva")) tIn->SetBranchAddress("mva", &mva);
    
    // Vertex vectors
    if (tIn->GetBranch("VtxProb")) tIn->SetBranchAddress("VtxProb", &VtxProb);
    if (tIn->GetBranch("VtxChi2")) tIn->SetBranchAddress("VtxChi2", &VtxChi2);
    if (tIn->GetBranch("alpha3D")) tIn->SetBranchAddress("alpha3D", &alpha3D);
    if (tIn->GetBranch("decayLength3D")) tIn->SetBranchAddress("decayLength3D", &decayLength3D);
    if (tIn->GetBranch("decayLengthSig3D")) tIn->SetBranchAddress("decayLengthSig3D", &decayLengthSig3D);
    
    // Daughter vectors
    if (tIn->GetBranch("pTD")) tIn->SetBranchAddress("pTD", &pTD);
    if (tIn->GetBranch("etaD")) tIn->SetBranchAddress("etaD", &etaD);
    if (tIn->GetBranch("phiD")) tIn->SetBranchAddress("phiD", &phiD);
    if (tIn->GetBranch("chargeD")) tIn->SetBranchAddress("chargeD", &chargeD);
    if (tIn->GetBranch("dedxD")) tIn->SetBranchAddress("dedxD", &dedxD);
    
    // Create output file and tree
    TFile *fOut = new TFile(outputFile, "RECREATE");
    TTree *tOut = new TTree("flatTree", "Flattened candidate tree");
    
    // Output variables (one per candidate)
    UInt_t out_runNb, out_lsNb, out_eventNb;
    Short_t out_nPV, out_centrality;
    Int_t out_Ntrkoffline;
    Float_t out_bestvtxZ;
    UInt_t out_candIdx;
    
    Float_t out_pT, out_eta, out_phi, out_mass, out_y, out_mva;
    Int_t out_pdgId;
    Float_t out_VtxProb, out_VtxChi2, out_alpha3D, out_decayLength3D, out_decayLengthSig3D;
    
    // Daughter info (fixed array for simplicity)
    Float_t out_pTD1, out_pTD2, out_etaD1, out_etaD2, out_phiD1, out_phiD2;
    Short_t out_chargeD1, out_chargeD2;
    Float_t out_dedxD1, out_dedxD2;
    
    // Create output branches
    tOut->Branch("runNb", &out_runNb, "runNb/i");
    tOut->Branch("lsNb", &out_lsNb, "lsNb/i");
    tOut->Branch("eventNb", &out_eventNb, "eventNb/i");
    tOut->Branch("nPV", &out_nPV, "nPV/S");
    tOut->Branch("centrality", &out_centrality, "centrality/S");
    tOut->Branch("Ntrkoffline", &out_Ntrkoffline, "Ntrkoffline/I");
    tOut->Branch("bestvtxZ", &out_bestvtxZ, "bestvtxZ/F");
    tOut->Branch("candIdx", &out_candIdx, "candIdx/i");
    
    tOut->Branch("pT", &out_pT, "pT/F");
    tOut->Branch("eta", &out_eta, "eta/F");
    tOut->Branch("phi", &out_phi, "phi/F");
    tOut->Branch("mass", &out_mass, "mass/F");
    tOut->Branch("y", &out_y, "y/F");
    tOut->Branch("pdgId", &out_pdgId, "pdgId/I");
    tOut->Branch("mva", &out_mva, "mva/F");
    
    tOut->Branch("VtxProb", &out_VtxProb, "VtxProb/F");
    tOut->Branch("VtxChi2", &out_VtxChi2, "VtxChi2/F");
    tOut->Branch("alpha3D", &out_alpha3D, "alpha3D/F");
    tOut->Branch("decayLength3D", &out_decayLength3D, "decayLength3D/F");
    tOut->Branch("decayLengthSig3D", &out_decayLengthSig3D, "decayLengthSig3D/F");
    
    tOut->Branch("pTD1", &out_pTD1, "pTD1/F");
    tOut->Branch("pTD2", &out_pTD2, "pTD2/F");
    tOut->Branch("etaD1", &out_etaD1, "etaD1/F");
    tOut->Branch("etaD2", &out_etaD2, "etaD2/F");
    tOut->Branch("phiD1", &out_phiD1, "phiD1/F");
    tOut->Branch("phiD2", &out_phiD2, "phiD2/F");
    tOut->Branch("chargeD1", &out_chargeD1, "chargeD1/S");
    tOut->Branch("chargeD2", &out_chargeD2, "chargeD2/S");
    tOut->Branch("dedxD1", &out_dedxD1, "dedxD1/F");
    tOut->Branch("dedxD2", &out_dedxD2, "dedxD2/F");
    
    // Loop over events
    Long64_t nEvents = tIn->GetEntries();
    Long64_t totalCands = 0;
    
    for (Long64_t iEvt = 0; iEvt < nEvents; iEvt++) {
        tIn->GetEntry(iEvt);
        
        // Skip events with no candidates
        if (candSize == 0) continue;
        
        // Loop over candidates in this event
        for (UInt_t iCand = 0; iCand < candSize; iCand++) {
            // Event info
            out_runNb = runNb;
            out_lsNb = lsNb;
            out_eventNb = eventNb;
            out_nPV = nPV;
            out_centrality = centrality;
            out_Ntrkoffline = Ntrkoffline;
            out_bestvtxZ = bestvtxZ;
            out_candIdx = iCand;
            
            // Candidate info
            out_pT = (pT && iCand < pT->size()) ? pT->at(iCand) : -99;
            out_eta = (eta && iCand < eta->size()) ? eta->at(iCand) : -99;
            out_phi = (phi && iCand < phi->size()) ? phi->at(iCand) : -99;
            out_mass = (mass && iCand < mass->size()) ? mass->at(iCand) : -99;
            out_y = (y && iCand < y->size()) ? y->at(iCand) : -99;
            out_pdgId = (pdgId && iCand < pdgId->size()) ? pdgId->at(iCand) : 0;
            out_mva = (mva && iCand < mva->size()) ? mva->at(iCand) : -99;
            
            // Vertex info
            out_VtxProb = (VtxProb && iCand < VtxProb->size()) ? VtxProb->at(iCand) : -99;
            out_VtxChi2 = (VtxChi2 && iCand < VtxChi2->size()) ? VtxChi2->at(iCand) : -99;
            out_alpha3D = (alpha3D && iCand < alpha3D->size()) ? alpha3D->at(iCand) : -99;
            out_decayLength3D = (decayLength3D && iCand < decayLength3D->size()) ? decayLength3D->at(iCand) : -99;
            out_decayLengthSig3D = (decayLengthSig3D && iCand < decayLengthSig3D->size()) ? decayLengthSig3D->at(iCand) : -99;
            
            // Daughter info
            out_pTD1 = -99; out_pTD2 = -99;
            out_etaD1 = -99; out_etaD2 = -99;
            out_phiD1 = -99; out_phiD2 = -99;
            out_chargeD1 = 0; out_chargeD2 = 0;
            out_dedxD1 = -999; out_dedxD2 = -999;
            
            if (pTD && iCand < pTD->size()) {
                auto& dauPt = pTD->at(iCand);
                if (dauPt.size() > 0) out_pTD1 = dauPt[0];
                if (dauPt.size() > 1) out_pTD2 = dauPt[1];
            }
            if (etaD && iCand < etaD->size()) {
                auto& dauEta = etaD->at(iCand);
                if (dauEta.size() > 0) out_etaD1 = dauEta[0];
                if (dauEta.size() > 1) out_etaD2 = dauEta[1];
            }
            if (phiD && iCand < phiD->size()) {
                auto& dauPhi = phiD->at(iCand);
                if (dauPhi.size() > 0) out_phiD1 = dauPhi[0];
                if (dauPhi.size() > 1) out_phiD2 = dauPhi[1];
            }
            if (chargeD && iCand < chargeD->size()) {
                auto& dauChg = chargeD->at(iCand);
                if (dauChg.size() > 0) out_chargeD1 = dauChg[0];
                if (dauChg.size() > 1) out_chargeD2 = dauChg[1];
            }
            if (dedxD && iCand < dedxD->size()) {
                auto& dauDedx = dedxD->at(iCand);
                if (dauDedx.size() > 0) out_dedxD1 = dauDedx[0];
                if (dauDedx.size() > 1) out_dedxD2 = dauDedx[1];
            }
            
            tOut->Fill();
            totalCands++;
        }
    }
    
    std::cout << "Processed " << nEvents << " events, wrote " << totalCands << " candidates" << std::endl;
    
    // Write and close
    fOut->cd();
    tOut->Write();
    fOut->Close();
    fIn->Close();
    
    std::cout << "Output written to " << outputFile << std::endl;
}
