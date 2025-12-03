//////////////////////////////////////////////////////////
// FlattenBDiMuMu.C
// Standalone ROOT macro for flattening BDiMuMuNtuplizer output
// Converts nested B meson tree to flat structure (one row per B meson)
// Usage: root -l 'FlattenBDiMuMu.C("input.root", "output.root")'
//////////////////////////////////////////////////////////

#include "../interface/FlattenBDiMuMu.h"

// Constructor (single file)
FlattenBDiMuMu::FlattenBDiMuMu(const char* inputFileName, const char* outputFileName) {
    std::cout << "=== FlattenBDiMuMu ===" << std::endl;
    std::cout << "Input file: " << inputFileName << std::endl;
    std::cout << "Output file: " << outputFileName << std::endl;
    
    // Open input file and get tree
    TFile* inputFile = TFile::Open(inputFileName, "READ");
    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Error: Cannot open input file " << inputFileName << std::endl;
        return;
    }
    
    inputTree = (TTree*)inputFile->Get("bDiMuMuNtuplizer/bDiMuMuTree");
    if (!inputTree) {
        std::cerr << "Error: Cannot find bDiMuMuNtuplizer/bDiMuMuTree in input file" << std::endl;
        return;
    }
    
    std::cout << "Found input tree with " << inputTree->GetEntries() << " events" << std::endl;
    
    // Create output file and tree
    outputFile = TFile::Open(outputFileName, "RECREATE");
    if (!outputFile || outputFile->IsZombie()) {
        std::cerr << "Error: Cannot create output file " << outputFileName << std::endl;
        return;
    }
    
    flatTree = new TTree("bDiMuMuFlatTree", "Flattened B meson tree - one entry per B meson candidate");
    
    // Setup trees
    SetupInputTree();
    SetupOutputTree();
    
    std::cout << "Trees initialized successfully" << std::endl;
}

// Constructor (TChain)
FlattenBDiMuMu::FlattenBDiMuMu(TChain* inputChain, const char* outputFileName) {
    std::cout << "=== FlattenBDiMuMu (TChain) ===" << std::endl;
    std::cout << "Input chain with " << inputChain->GetNtrees() << " files" << std::endl;
    std::cout << "Output file: " << outputFileName << std::endl;
    
    inputTree = inputChain;
    if (!inputTree) {
        std::cerr << "Error: Invalid input chain" << std::endl;
        return;
    }
    
    std::cout << "Found input chain with " << inputTree->GetEntries() << " events" << std::endl;
    
    // Create output file and tree
    outputFile = TFile::Open(outputFileName, "RECREATE");
    if (!outputFile || outputFile->IsZombie()) {
        std::cerr << "Error: Cannot create output file " << outputFileName << std::endl;
        return;
    }
    
    flatTree = new TTree("bDiMuMuFlatTree", "Flattened B meson tree - one entry per B meson candidate");
    
    // Setup trees
    SetupInputTree();
    SetupOutputTree();
    
    std::cout << "Trees initialized successfully" << std::endl;
}

// Destructor
FlattenBDiMuMu::~FlattenBDiMuMu() {
    if (outputFile) {
        outputFile->Close();
        delete outputFile;
    }
}

// Setup input tree branch addresses
void FlattenBDiMuMu::SetupInputTree() {
    // Event information
    inputTree->SetBranchAddress("runNb", &runNb);
    inputTree->SetBranchAddress("eventNb", &eventNb);
    inputTree->SetBranchAddress("lumiSection", &lumiSection);
    inputTree->SetBranchAddress("centrality", &centrality);
    inputTree->SetBranchAddress("Ntrkoffline", &Ntrkoffline);
    inputTree->SetBranchAddress("nPV", &nPV);
    inputTree->SetBranchAddress("bestvx", &bestvx);
    inputTree->SetBranchAddress("bestvy", &bestvy);
    inputTree->SetBranchAddress("bestvz", &bestvz);
    inputTree->SetBranchAddress("bestvxError", &bestvxError);
    inputTree->SetBranchAddress("bestvyError", &bestvyError);
    inputTree->SetBranchAddress("bestvzError", &bestvzError);
    
    // B meson candidate information
    inputTree->SetBranchAddress("nBMeson", &nBMeson);
    inputTree->SetBranchAddress("bMesonType", bMesonType);
    inputTree->SetBranchAddress("bMesonPt", bMesonPt);
    inputTree->SetBranchAddress("bMesonEta", bMesonEta);
    inputTree->SetBranchAddress("bMesonPhi", bMesonPhi);
    inputTree->SetBranchAddress("bMesonY", bMesonY);
    inputTree->SetBranchAddress("bMesonMass", bMesonMass);
    inputTree->SetBranchAddress("bMesonCharge", bMesonCharge);
    inputTree->SetBranchAddress("bMesonMVA", bMesonMVA);
    inputTree->SetBranchAddress("bMesonMatchGEN", bMesonMatchGEN);
    
    // Vertex quality
    inputTree->SetBranchAddress("bMesonVtxChi2", bMesonVtxChi2);
    inputTree->SetBranchAddress("bMesonVtxNdof", bMesonVtxNdof);
    inputTree->SetBranchAddress("bMesonVtxProb", bMesonVtxProb);
    inputTree->SetBranchAddress("bMesonVtxX", bMesonVtxX);
    inputTree->SetBranchAddress("bMesonVtxY", bMesonVtxY);
    inputTree->SetBranchAddress("bMesonVtxZ", bMesonVtxZ);
    inputTree->SetBranchAddress("bMesonVtxXError", bMesonVtxXError);
    inputTree->SetBranchAddress("bMesonVtxYError", bMesonVtxYError);
    inputTree->SetBranchAddress("bMesonVtxZError", bMesonVtxZError);
    
    // Topological variables
    inputTree->SetBranchAddress("bMesonPointingAngle2D", bMesonPointingAngle2D);
    inputTree->SetBranchAddress("bMesonPointingAngle3D", bMesonPointingAngle3D);
    inputTree->SetBranchAddress("bMesonDecayLength2D", bMesonDecayLength2D);
    inputTree->SetBranchAddress("bMesonDecayLength3D", bMesonDecayLength3D);
    inputTree->SetBranchAddress("bMesonDecayLengthSig2D", bMesonDecayLengthSig2D);
    inputTree->SetBranchAddress("bMesonDecayLengthSig3D", bMesonDecayLengthSig3D);
    
    // Dimuon information
    inputTree->SetBranchAddress("dimuonPt", dimuonPt);
    inputTree->SetBranchAddress("dimuonEta", dimuonEta);
    inputTree->SetBranchAddress("dimuonPhi", dimuonPhi);
    inputTree->SetBranchAddress("dimuonY", dimuonY);
    inputTree->SetBranchAddress("dimuonMass", dimuonMass);
    inputTree->SetBranchAddress("dimuonCharge", dimuonCharge);
    
    // Muon information
    inputTree->SetBranchAddress("muonPt", muonPt);
    inputTree->SetBranchAddress("muonEta", muonEta);
    inputTree->SetBranchAddress("muonPhi", muonPhi);
    inputTree->SetBranchAddress("muonCharge", muonCharge);
    inputTree->SetBranchAddress("muonIsGlobal", muonIsGlobal);
    inputTree->SetBranchAddress("muonIsTracker", muonIsTracker);
    inputTree->SetBranchAddress("muonIsPF", muonIsPF);
    inputTree->SetBranchAddress("muonDxy", muonDxy);
    inputTree->SetBranchAddress("muonDz", muonDz);
    inputTree->SetBranchAddress("muonNHits", muonNHits);
    inputTree->SetBranchAddress("muonNPixelHits", muonNPixelHits);
    inputTree->SetBranchAddress("muonNStations", muonNStations);
    
    // Additional track information
    inputTree->SetBranchAddress("nTracks", nTracks);
    inputTree->SetBranchAddress("trackPt", trackPt);
    inputTree->SetBranchAddress("trackEta", trackEta);
    inputTree->SetBranchAddress("trackPhi", trackPhi);
    inputTree->SetBranchAddress("trackCharge", trackCharge);
    inputTree->SetBranchAddress("trackDxy", trackDxy);
    inputTree->SetBranchAddress("trackDz", trackDz);
    inputTree->SetBranchAddress("trackPtError", trackPtError);
    inputTree->SetBranchAddress("trackNHits", trackNHits);
    inputTree->SetBranchAddress("trackNPixelHits", trackNPixelHits);
    inputTree->SetBranchAddress("trackChi2", trackChi2);
    inputTree->SetBranchAddress("trackHighPurity", trackHighPurity);
    inputTree->SetBranchAddress("trackPID", trackPID);
    
    // K*0 information
    inputTree->SetBranchAddress("kstarPt", kstarPt);
    inputTree->SetBranchAddress("kstarEta", kstarEta);
    inputTree->SetBranchAddress("kstarPhi", kstarPhi);
    inputTree->SetBranchAddress("kstarMass", kstarMass);
    
    // Generator information (check if branches exist)
    if (inputTree->GetBranch("nGenBMeson")) {
        inputTree->SetBranchAddress("nGenBMeson", &nGenBMeson);
        inputTree->SetBranchAddress("genBMesonType", genBMesonType);
        inputTree->SetBranchAddress("genBMesonPt", genBMesonPt);
        inputTree->SetBranchAddress("genBMesonEta", genBMesonEta);
        inputTree->SetBranchAddress("genBMesonPhi", genBMesonPhi);
        inputTree->SetBranchAddress("genBMesonY", genBMesonY);
        inputTree->SetBranchAddress("genBMesonMass", genBMesonMass);
        inputTree->SetBranchAddress("genBMesonPdgId", genBMesonPdgId);
    }
}

// Setup output tree branches (flat structure)
void FlattenBDiMuMu::SetupOutputTree() {
    // Event information (duplicated for each B meson)
    flatTree->Branch("runNb", &flat_runNb, "runNb/i");
    flatTree->Branch("eventNb", &flat_eventNb, "eventNb/l");
    flatTree->Branch("lumiSection", &flat_lumiSection, "lumiSection/i");
    flatTree->Branch("centrality", &flat_centrality, "centrality/S");
    flatTree->Branch("Ntrkoffline", &flat_Ntrkoffline, "Ntrkoffline/I");
    flatTree->Branch("nPV", &flat_nPV, "nPV/S");
    flatTree->Branch("bestvx", &flat_bestvx, "bestvx/F");
    flatTree->Branch("bestvy", &flat_bestvy, "bestvy/F");
    flatTree->Branch("bestvz", &flat_bestvz, "bestvz/F");
    flatTree->Branch("bestvxError", &flat_bestvxError, "bestvxError/F");
    flatTree->Branch("bestvyError", &flat_bestvyError, "bestvyError/F");
    flatTree->Branch("bestvzError", &flat_bestvzError, "bestvzError/F");
    
    // B meson information (scalar per candidate)
    flatTree->Branch("bMesonType", &flat_bMesonType, "bMesonType/S");
    flatTree->Branch("bMesonPt", &flat_bMesonPt, "bMesonPt/F");
    flatTree->Branch("bMesonEta", &flat_bMesonEta, "bMesonEta/F");
    flatTree->Branch("bMesonPhi", &flat_bMesonPhi, "bMesonPhi/F");
    flatTree->Branch("bMesonY", &flat_bMesonY, "bMesonY/F");
    flatTree->Branch("bMesonMass", &flat_bMesonMass, "bMesonMass/F");
    flatTree->Branch("bMesonCharge", &flat_bMesonCharge, "bMesonCharge/F");
    flatTree->Branch("bMesonMVA", &flat_bMesonMVA, "bMesonMVA/F");
    flatTree->Branch("bMesonMatchGEN", &flat_bMesonMatchGEN, "bMesonMatchGEN/O");
    
    // Vertex quality
    flatTree->Branch("bMesonVtxChi2", &flat_bMesonVtxChi2, "bMesonVtxChi2/F");
    flatTree->Branch("bMesonVtxNdof", &flat_bMesonVtxNdof, "bMesonVtxNdof/F");
    flatTree->Branch("bMesonVtxProb", &flat_bMesonVtxProb, "bMesonVtxProb/F");
    flatTree->Branch("bMesonVtxX", &flat_bMesonVtxX, "bMesonVtxX/F");
    flatTree->Branch("bMesonVtxY", &flat_bMesonVtxY, "bMesonVtxY/F");
    flatTree->Branch("bMesonVtxZ", &flat_bMesonVtxZ, "bMesonVtxZ/F");
    flatTree->Branch("bMesonVtxXError", &flat_bMesonVtxXError, "bMesonVtxXError/F");
    flatTree->Branch("bMesonVtxYError", &flat_bMesonVtxYError, "bMesonVtxYError/F");
    flatTree->Branch("bMesonVtxZError", &flat_bMesonVtxZError, "bMesonVtxZError/F");
    
    // Topological variables
    flatTree->Branch("bMesonPointingAngle2D", &flat_bMesonPointingAngle2D, "bMesonPointingAngle2D/F");
    flatTree->Branch("bMesonPointingAngle3D", &flat_bMesonPointingAngle3D, "bMesonPointingAngle3D/F");
    flatTree->Branch("bMesonDecayLength2D", &flat_bMesonDecayLength2D, "bMesonDecayLength2D/F");
    flatTree->Branch("bMesonDecayLength3D", &flat_bMesonDecayLength3D, "bMesonDecayLength3D/F");
    flatTree->Branch("bMesonDecayLengthSig2D", &flat_bMesonDecayLengthSig2D, "bMesonDecayLengthSig2D/F");
    flatTree->Branch("bMesonDecayLengthSig3D", &flat_bMesonDecayLengthSig3D, "bMesonDecayLengthSig3D/F");
    
    // Dimuon information
    flatTree->Branch("dimuonPt", &flat_dimuonPt, "dimuonPt/F");
    flatTree->Branch("dimuonEta", &flat_dimuonEta, "dimuonEta/F");
    flatTree->Branch("dimuonPhi", &flat_dimuonPhi, "dimuonPhi/F");
    flatTree->Branch("dimuonY", &flat_dimuonY, "dimuonY/F");
    flatTree->Branch("dimuonMass", &flat_dimuonMass, "dimuonMass/F");
    flatTree->Branch("dimuonCharge", &flat_dimuonCharge, "dimuonCharge/F");
    
    // Individual muon information
    flatTree->Branch("muon1Pt", &flat_muon1Pt, "muon1Pt/F");
    flatTree->Branch("muon1Eta", &flat_muon1Eta, "muon1Eta/F");
    flatTree->Branch("muon1Phi", &flat_muon1Phi, "muon1Phi/F");
    flatTree->Branch("muon1Charge", &flat_muon1Charge, "muon1Charge/F");
    flatTree->Branch("muon1IsGlobal", &flat_muon1IsGlobal, "muon1IsGlobal/O");
    flatTree->Branch("muon1IsTracker", &flat_muon1IsTracker, "muon1IsTracker/O");
    flatTree->Branch("muon1IsPF", &flat_muon1IsPF, "muon1IsPF/O");
    flatTree->Branch("muon1Dxy", &flat_muon1Dxy, "muon1Dxy/F");
    flatTree->Branch("muon1Dz", &flat_muon1Dz, "muon1Dz/F");
    flatTree->Branch("muon1NHits", &flat_muon1NHits, "muon1NHits/S");
    flatTree->Branch("muon1NPixelHits", &flat_muon1NPixelHits, "muon1NPixelHits/S");
    flatTree->Branch("muon1NStations", &flat_muon1NStations, "muon1NStations/S");
    
    flatTree->Branch("muon2Pt", &flat_muon2Pt, "muon2Pt/F");
    flatTree->Branch("muon2Eta", &flat_muon2Eta, "muon2Eta/F");
    flatTree->Branch("muon2Phi", &flat_muon2Phi, "muon2Phi/F");
    flatTree->Branch("muon2Charge", &flat_muon2Charge, "muon2Charge/F");
    flatTree->Branch("muon2IsGlobal", &flat_muon2IsGlobal, "muon2IsGlobal/O");
    flatTree->Branch("muon2IsTracker", &flat_muon2IsTracker, "muon2IsTracker/O");
    flatTree->Branch("muon2IsPF", &flat_muon2IsPF, "muon2IsPF/O");
    flatTree->Branch("muon2Dxy", &flat_muon2Dxy, "muon2Dxy/F");
    flatTree->Branch("muon2Dz", &flat_muon2Dz, "muon2Dz/F");
    flatTree->Branch("muon2NHits", &flat_muon2NHits, "muon2NHits/S");
    flatTree->Branch("muon2NPixelHits", &flat_muon2NPixelHits, "muon2NPixelHits/S");
    flatTree->Branch("muon2NStations", &flat_muon2NStations, "muon2NStations/S");
    
    // Additional track information
    flatTree->Branch("nTracks", &flat_nTracks, "nTracks/s");
    flatTree->Branch("track1Pt", &flat_track1Pt, "track1Pt/F");
    flatTree->Branch("track1Eta", &flat_track1Eta, "track1Eta/F");
    flatTree->Branch("track1Phi", &flat_track1Phi, "track1Phi/F");
    flatTree->Branch("track1Charge", &flat_track1Charge, "track1Charge/F");
    flatTree->Branch("track1Dxy", &flat_track1Dxy, "track1Dxy/F");
    flatTree->Branch("track1Dz", &flat_track1Dz, "track1Dz/F");
    flatTree->Branch("track1PtError", &flat_track1PtError, "track1PtError/F");
    flatTree->Branch("track1NHits", &flat_track1NHits, "track1NHits/S");
    flatTree->Branch("track1NPixelHits", &flat_track1NPixelHits, "track1NPixelHits/S");
    flatTree->Branch("track1Chi2", &flat_track1Chi2, "track1Chi2/F");
    flatTree->Branch("track1HighPurity", &flat_track1HighPurity, "track1HighPurity/O");
    flatTree->Branch("track1PID", &flat_track1PID, "track1PID/S");
    
    flatTree->Branch("track2Pt", &flat_track2Pt, "track2Pt/F");
    flatTree->Branch("track2Eta", &flat_track2Eta, "track2Eta/F");
    flatTree->Branch("track2Phi", &flat_track2Phi, "track2Phi/F");
    flatTree->Branch("track2Charge", &flat_track2Charge, "track2Charge/F");
    flatTree->Branch("track2Dxy", &flat_track2Dxy, "track2Dxy/F");
    flatTree->Branch("track2Dz", &flat_track2Dz, "track2Dz/F");
    flatTree->Branch("track2PtError", &flat_track2PtError, "track2PtError/F");
    flatTree->Branch("track2NHits", &flat_track2NHits, "track2NHits/S");
    flatTree->Branch("track2NPixelHits", &flat_track2NPixelHits, "track2NPixelHits/S");
    flatTree->Branch("track2Chi2", &flat_track2Chi2, "track2Chi2/F");
    flatTree->Branch("track2HighPurity", &flat_track2HighPurity, "track2HighPurity/O");
    flatTree->Branch("track2PID", &flat_track2PID, "track2PID/S");
    
    // K*0 information (for B0 only)
    flatTree->Branch("kstarPt", &flat_kstarPt, "kstarPt/F");
    flatTree->Branch("kstarEta", &flat_kstarEta, "kstarEta/F");
    flatTree->Branch("kstarPhi", &flat_kstarPhi, "kstarPhi/F");
    flatTree->Branch("kstarMass", &flat_kstarMass, "kstarMass/F");
}

// Process all events
void FlattenBDiMuMu::ProcessEvents() {
    Long64_t nEntries = inputTree->GetEntries();
    Long64_t totalBMesons = 0;
    
    std::cout << "Processing " << nEntries << " events..." << std::endl;
    
    for (Long64_t entry = 0; entry < nEntries; entry++) {
        if (entry % 10000 == 0) {
            std::cout << "Processing event " << entry << " / " << nEntries 
                      << " (" << (100.0 * entry / nEntries) << "%)" << std::endl;
        }
        
        inputTree->GetEntry(entry);
        
        // Loop over all B meson candidates in this event
        for (int bmeson = 0; bmeson < nBMeson; bmeson++) {
            FillFlatEntry(bmeson);
            flatTree->Fill();
            totalBMesons++;
        }
    }
    
    std::cout << "Processed " << nEntries << " events" << std::endl;
    std::cout << "Total B meson candidates: " << totalBMesons << std::endl;
    std::cout << "Writing output file..." << std::endl;
    
    outputFile->Write();
    std::cout << "Done!" << std::endl;
}

// Fill flat entry for a specific B meson candidate
void FlattenBDiMuMu::FillFlatEntry(int idx) {
    ResetFlatBranches();
    
    // Event information (same for all B mesons in this event)
    flat_runNb = runNb;
    flat_eventNb = eventNb;
    flat_lumiSection = lumiSection;
    flat_centrality = centrality;
    flat_Ntrkoffline = Ntrkoffline;
    flat_nPV = nPV;
    flat_bestvx = bestvx;
    flat_bestvy = bestvy;
    flat_bestvz = bestvz;
    flat_bestvxError = bestvxError;
    flat_bestvyError = bestvyError;
    flat_bestvzError = bestvzError;
    
    // B meson information
    flat_bMesonType = bMesonType[idx];
    flat_bMesonPt = bMesonPt[idx];
    flat_bMesonEta = bMesonEta[idx];
    flat_bMesonPhi = bMesonPhi[idx];
    flat_bMesonY = bMesonY[idx];
    flat_bMesonMass = bMesonMass[idx];
    flat_bMesonCharge = bMesonCharge[idx];
    flat_bMesonMVA = bMesonMVA[idx];
    flat_bMesonMatchGEN = bMesonMatchGEN[idx];
    
    // Vertex quality
    flat_bMesonVtxChi2 = bMesonVtxChi2[idx];
    flat_bMesonVtxNdof = bMesonVtxNdof[idx];
    flat_bMesonVtxProb = bMesonVtxProb[idx];
    flat_bMesonVtxX = bMesonVtxX[idx];
    flat_bMesonVtxY = bMesonVtxY[idx];
    flat_bMesonVtxZ = bMesonVtxZ[idx];
    flat_bMesonVtxXError = bMesonVtxXError[idx];
    flat_bMesonVtxYError = bMesonVtxYError[idx];
    flat_bMesonVtxZError = bMesonVtxZError[idx];
    
    // Topological variables
    flat_bMesonPointingAngle2D = bMesonPointingAngle2D[idx];
    flat_bMesonPointingAngle3D = bMesonPointingAngle3D[idx];
    flat_bMesonDecayLength2D = bMesonDecayLength2D[idx];
    flat_bMesonDecayLength3D = bMesonDecayLength3D[idx];
    flat_bMesonDecayLengthSig2D = bMesonDecayLengthSig2D[idx];
    flat_bMesonDecayLengthSig3D = bMesonDecayLengthSig3D[idx];
    
    // Dimuon information
    flat_dimuonPt = dimuonPt[idx];
    flat_dimuonEta = dimuonEta[idx];
    flat_dimuonPhi = dimuonPhi[idx];
    flat_dimuonY = dimuonY[idx];
    flat_dimuonMass = dimuonMass[idx];
    flat_dimuonCharge = dimuonCharge[idx];
    
    // Muon information (assign muon1 = higher pT, muon2 = lower pT)
    if (muonPt[idx][0] >= muonPt[idx][1]) {
        // Muon 0 is leading
        flat_muon1Pt = muonPt[idx][0];
        flat_muon1Eta = muonEta[idx][0];
        flat_muon1Phi = muonPhi[idx][0];
        flat_muon1Charge = muonCharge[idx][0];
        flat_muon1IsGlobal = muonIsGlobal[idx][0];
        flat_muon1IsTracker = muonIsTracker[idx][0];
        flat_muon1IsPF = muonIsPF[idx][0];
        flat_muon1Dxy = muonDxy[idx][0];
        flat_muon1Dz = muonDz[idx][0];
        flat_muon1NHits = muonNHits[idx][0];
        flat_muon1NPixelHits = muonNPixelHits[idx][0];
        flat_muon1NStations = muonNStations[idx][0];
        
        flat_muon2Pt = muonPt[idx][1];
        flat_muon2Eta = muonEta[idx][1];
        flat_muon2Phi = muonPhi[idx][1];
        flat_muon2Charge = muonCharge[idx][1];
        flat_muon2IsGlobal = muonIsGlobal[idx][1];
        flat_muon2IsTracker = muonIsTracker[idx][1];
        flat_muon2IsPF = muonIsPF[idx][1];
        flat_muon2Dxy = muonDxy[idx][1];
        flat_muon2Dz = muonDz[idx][1];
        flat_muon2NHits = muonNHits[idx][1];
        flat_muon2NPixelHits = muonNPixelHits[idx][1];
        flat_muon2NStations = muonNStations[idx][1];
    } else {
        // Muon 1 is leading
        flat_muon1Pt = muonPt[idx][1];
        flat_muon1Eta = muonEta[idx][1];
        flat_muon1Phi = muonPhi[idx][1];
        flat_muon1Charge = muonCharge[idx][1];
        flat_muon1IsGlobal = muonIsGlobal[idx][1];
        flat_muon1IsTracker = muonIsTracker[idx][1];
        flat_muon1IsPF = muonIsPF[idx][1];
        flat_muon1Dxy = muonDxy[idx][1];
        flat_muon1Dz = muonDz[idx][1];
        flat_muon1NHits = muonNHits[idx][1];
        flat_muon1NPixelHits = muonNPixelHits[idx][1];
        flat_muon1NStations = muonNStations[idx][1];
        
        flat_muon2Pt = muonPt[idx][0];
        flat_muon2Eta = muonEta[idx][0];
        flat_muon2Phi = muonPhi[idx][0];
        flat_muon2Charge = muonCharge[idx][0];
        flat_muon2IsGlobal = muonIsGlobal[idx][0];
        flat_muon2IsTracker = muonIsTracker[idx][0];
        flat_muon2IsPF = muonIsPF[idx][0];
        flat_muon2Dxy = muonDxy[idx][0];
        flat_muon2Dz = muonDz[idx][0];
        flat_muon2NHits = muonNHits[idx][0];
        flat_muon2NPixelHits = muonNPixelHits[idx][0];
        flat_muon2NStations = muonNStations[idx][0];
    }
    
    // Track information
    flat_nTracks = nTracks[idx];
    
    if (nTracks[idx] >= 1) {
        flat_track1Pt = trackPt[idx][0];
        flat_track1Eta = trackEta[idx][0];
        flat_track1Phi = trackPhi[idx][0];
        flat_track1Charge = trackCharge[idx][0];
        flat_track1Dxy = trackDxy[idx][0];
        flat_track1Dz = trackDz[idx][0];
        flat_track1PtError = trackPtError[idx][0];
        flat_track1NHits = trackNHits[idx][0];
        flat_track1NPixelHits = trackNPixelHits[idx][0];
        flat_track1Chi2 = trackChi2[idx][0];
        flat_track1HighPurity = trackHighPurity[idx][0];
        flat_track1PID = trackPID[idx][0];
    }
    
    if (nTracks[idx] >= 2) {
        flat_track2Pt = trackPt[idx][1];
        flat_track2Eta = trackEta[idx][1];
        flat_track2Phi = trackPhi[idx][1];
        flat_track2Charge = trackCharge[idx][1];
        flat_track2Dxy = trackDxy[idx][1];
        flat_track2Dz = trackDz[idx][1];
        flat_track2PtError = trackPtError[idx][1];
        flat_track2NHits = trackNHits[idx][1];
        flat_track2NPixelHits = trackNPixelHits[idx][1];
        flat_track2Chi2 = trackChi2[idx][1];
        flat_track2HighPurity = trackHighPurity[idx][1];
        flat_track2PID = trackPID[idx][1];
    }
    
    // K*0 information (for B0 only)
    if (IsBZero(idx)) {
        flat_kstarPt = kstarPt[idx];
        flat_kstarEta = kstarEta[idx];
        flat_kstarPhi = kstarPhi[idx];
        flat_kstarMass = kstarMass[idx];
    }
}

// Reset flat branches to default values
void FlattenBDiMuMu::ResetFlatBranches() {
    // Event info
    flat_runNb = 0;
    flat_eventNb = 0;
    flat_lumiSection = 0;
    flat_centrality = -1;
    flat_Ntrkoffline = 0;
    flat_nPV = 0;
    flat_bestvx = -999.;
    flat_bestvy = -999.;
    flat_bestvz = -999.;
    flat_bestvxError = -999.;
    flat_bestvyError = -999.;
    flat_bestvzError = -999.;
    
    // B meson info
    flat_bMesonType = -1;
    flat_bMesonPt = -999.;
    flat_bMesonEta = -999.;
    flat_bMesonPhi = -999.;
    flat_bMesonY = -999.;
    flat_bMesonMass = -999.;
    flat_bMesonCharge = -999.;
    flat_bMesonMVA = -999.;
    flat_bMesonMatchGEN = false;
    
    // Vertex quality
    flat_bMesonVtxChi2 = -999.;
    flat_bMesonVtxNdof = -999.;
    flat_bMesonVtxProb = -999.;
    flat_bMesonVtxX = -999.;
    flat_bMesonVtxY = -999.;
    flat_bMesonVtxZ = -999.;
    flat_bMesonVtxXError = -999.;
    flat_bMesonVtxYError = -999.;
    flat_bMesonVtxZError = -999.;
    
    // Topological variables
    flat_bMesonPointingAngle2D = -999.;
    flat_bMesonPointingAngle3D = -999.;
    flat_bMesonDecayLength2D = -999.;
    flat_bMesonDecayLength3D = -999.;
    flat_bMesonDecayLengthSig2D = -999.;
    flat_bMesonDecayLengthSig3D = -999.;
    
    // Dimuon info
    flat_dimuonPt = -999.;
    flat_dimuonEta = -999.;
    flat_dimuonPhi = -999.;
    flat_dimuonY = -999.;
    flat_dimuonMass = -999.;
    flat_dimuonCharge = -999.;
    
    // Muon info
    flat_muon1Pt = -999.;
    flat_muon1Eta = -999.;
    flat_muon1Phi = -999.;
    flat_muon1Charge = -999.;
    flat_muon1IsGlobal = false;
    flat_muon1IsTracker = false;
    flat_muon1IsPF = false;
    flat_muon1Dxy = -999.;
    flat_muon1Dz = -999.;
    flat_muon1NHits = -1;
    flat_muon1NPixelHits = -1;
    flat_muon1NStations = -1;
    
    flat_muon2Pt = -999.;
    flat_muon2Eta = -999.;
    flat_muon2Phi = -999.;
    flat_muon2Charge = -999.;
    flat_muon2IsGlobal = false;
    flat_muon2IsTracker = false;
    flat_muon2IsPF = false;
    flat_muon2Dxy = -999.;
    flat_muon2Dz = -999.;
    flat_muon2NHits = -1;
    flat_muon2NPixelHits = -1;
    flat_muon2NStations = -1;
    
    // Track info
    flat_nTracks = 0;
    flat_track1Pt = -999.;
    flat_track1Eta = -999.;
    flat_track1Phi = -999.;
    flat_track1Charge = -999.;
    flat_track1Dxy = -999.;
    flat_track1Dz = -999.;
    flat_track1PtError = -999.;
    flat_track1NHits = -1;
    flat_track1NPixelHits = -1;
    flat_track1Chi2 = -999.;
    flat_track1HighPurity = false;
    flat_track1PID = 0;
    
    flat_track2Pt = -999.;
    flat_track2Eta = -999.;
    flat_track2Phi = -999.;
    flat_track2Charge = -999.;
    flat_track2Dxy = -999.;
    flat_track2Dz = -999.;
    flat_track2PtError = -999.;
    flat_track2NHits = -1;
    flat_track2NPixelHits = -1;
    flat_track2Chi2 = -999.;
    flat_track2HighPurity = false;
    flat_track2PID = 0;
    
    // K*0 info
    flat_kstarPt = -999.;
    flat_kstarEta = -999.;
    flat_kstarPhi = -999.;
    flat_kstarMass = -999.;
}

// Main function for standalone execution - directory input with TChain
void FlattenBDiMuMu(const char* inputPath = ".", 
                    const char* outputFileName = "BDiMuMu_Flat.root") {
    
    std::cout << "=== BDiMuMu Tree Flattener ===" << std::endl;
    std::cout << "Converting nested B meson tree to flat structure" << std::endl;
    
    // Check if inputPath is a directory or file
    TString inputStr(inputPath);
    bool isDirectory = gSystem->OpenDirectory(inputPath) != nullptr;
    
    if (isDirectory) {
        std::cout << "Input directory: " << inputPath << std::endl;
        std::cout << "Output: " << outputFileName << std::endl;
        std::cout << std::endl;
        
        // Create TChain and add all ROOT files from directory
        TChain* chain = new TChain("bDiMuMuNtuplizer/bDiMuMuTree");
        TString pattern = TString(inputPath) + "/*.root";
        
        std::cout << "Adding files matching pattern: " << pattern << std::endl;
        Int_t nFiles = chain->Add(pattern);
        
        if (nFiles == 0) {
            std::cerr << "Error: No ROOT files found in " << inputPath << std::endl;
            delete chain;
            return;
        }
        
        std::cout << "Successfully added " << nFiles << " files to chain" << std::endl;
        std::cout << "Total entries: " << chain->GetEntries() << std::endl;
        
        // Process with TChain
        FlattenBDiMuMu* flattener = new FlattenBDiMuMu(chain, outputFileName);
        flattener->ProcessEvents();
        
        delete flattener;
        delete chain;
        
    } else {
        // Single file processing (original behavior)
        std::cout << "Input file: " << inputPath << std::endl;
        std::cout << "Output: " << outputFileName << std::endl;
        std::cout << std::endl;
        
        FlattenBDiMuMu* flattener = new FlattenBDiMuMu(inputPath, outputFileName);
        flattener->ProcessEvents();
        
        delete flattener;
    }
    
    std::cout << std::endl;
    std::cout << "Flattening complete!" << std::endl;
    std::cout << "You can now analyze the flat tree with simple cuts:" << std::endl;
    std::cout << "  root> TFile* f = TFile::Open(\"" << outputFileName << "\");" << std::endl;
    std::cout << "  root> TTree* t = (TTree*)f->Get(\"bDiMuMuFlatTree\");" << std::endl;
    std::cout << "  root> t->Draw(\"bMesonMass\", \"muon1Pt > 4 && track1Pt > 1\");" << std::endl;
}