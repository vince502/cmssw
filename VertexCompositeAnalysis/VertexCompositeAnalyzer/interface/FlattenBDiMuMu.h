//////////////////////////////////////////////////////////
// FlattenBDiMuMu.h
// Standalone ROOT header for flattening BDiMuMuNtuplizer output
// Converts nested B meson tree to flat structure (one row per B meson)
// Usage: root -l 'FlattenBDiMuMu.C("input.root", "output.root")'
//////////////////////////////////////////////////////////

#ifndef FlattenBDiMuMu_h
#define FlattenBDiMuMu_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TMath.h>
#include <iostream>

class FlattenBDiMuMu {
public:
    TTree* inputTree;
    TFile* outputFile;
    TTree* flatTree;

    // Input tree variables (from BDiMuMuNtuplizer)
    UInt_t          runNb;
    ULong64_t       eventNb;
    UInt_t          lumiSection;
    Short_t         centrality;
    Int_t           Ntrkoffline;
    Short_t         nPV;
    Float_t         bestvx;
    Float_t         bestvy;
    Float_t         bestvz;
    Float_t         bestvxError;
    Float_t         bestvyError;
    Float_t         bestvzError;
    
    // B meson candidate information (arrays)
    UShort_t        nBMeson;
    Short_t         bMesonType[1000];
    Float_t         bMesonPt[1000];
    Float_t         bMesonEta[1000];
    Float_t         bMesonPhi[1000];
    Float_t         bMesonY[1000];
    Float_t         bMesonMass[1000];
    Float_t         bMesonCharge[1000];
    Float_t         bMesonMVA[1000];
    Bool_t          bMesonMatchGEN[1000];
    
    // Vertex quality
    Float_t         bMesonVtxChi2[1000];
    Float_t         bMesonVtxNdof[1000];
    Float_t         bMesonVtxProb[1000];
    Float_t         bMesonVtxX[1000];
    Float_t         bMesonVtxY[1000];
    Float_t         bMesonVtxZ[1000];
    Float_t         bMesonVtxXError[1000];
    Float_t         bMesonVtxYError[1000];
    Float_t         bMesonVtxZError[1000];
    
    // Topological variables
    Float_t         bMesonPointingAngle2D[1000];
    Float_t         bMesonPointingAngle3D[1000];
    Float_t         bMesonDecayLength2D[1000];
    Float_t         bMesonDecayLength3D[1000];
    Float_t         bMesonDecayLengthSig2D[1000];
    Float_t         bMesonDecayLengthSig3D[1000];
    
    // Dimuon information
    Float_t         dimuonPt[1000];
    Float_t         dimuonEta[1000];
    Float_t         dimuonPhi[1000];
    Float_t         dimuonY[1000];
    Float_t         dimuonMass[1000];
    Float_t         dimuonCharge[1000];
    
    // Muon information
    Float_t         muonPt[1000][2];
    Float_t         muonEta[1000][2];
    Float_t         muonPhi[1000][2];
    Float_t         muonCharge[1000][2];
    Bool_t          muonIsGlobal[1000][2];
    Bool_t          muonIsTracker[1000][2];
    Bool_t          muonIsPF[1000][2];
    Float_t         muonDxy[1000][2];
    Float_t         muonDz[1000][2];
    Short_t         muonNHits[1000][2];
    Short_t         muonNPixelHits[1000][2];
    Short_t         muonNStations[1000][2];
    
    // Additional track information
    UShort_t        nTracks[1000];
    Float_t         trackPt[1000][10];
    Float_t         trackEta[1000][10];
    Float_t         trackPhi[1000][10];
    Float_t         trackCharge[1000][10];
    Float_t         trackDxy[1000][10];
    Float_t         trackDz[1000][10];
    Float_t         trackPtError[1000][10];
    Short_t         trackNHits[1000][10];
    Short_t         trackNPixelHits[1000][10];
    Float_t         trackChi2[1000][10];
    Bool_t          trackHighPurity[1000][10];
    Short_t         trackPID[1000][10];
    
    // K*0 information (for B0 only)
    Float_t         kstarPt[1000];
    Float_t         kstarEta[1000];
    Float_t         kstarPhi[1000];
    Float_t         kstarMass[1000];
    
    // Generator information (MC only)
    UShort_t        nGenBMeson;
    Short_t         genBMesonType[1000];
    Float_t         genBMesonPt[1000];
    Float_t         genBMesonEta[1000];
    Float_t         genBMesonPhi[1000];
    Float_t         genBMesonY[1000];
    Float_t         genBMesonMass[1000];
    Short_t         genBMesonPdgId[1000];

    // Flat tree variables (scalars - one per B meson candidate)
    // Event information (duplicated for each B meson in the event)
    UInt_t          flat_runNb;
    ULong64_t       flat_eventNb;
    UInt_t          flat_lumiSection;
    Short_t         flat_centrality;
    Int_t           flat_Ntrkoffline;
    Short_t         flat_nPV;
    Float_t         flat_bestvx;
    Float_t         flat_bestvy;
    Float_t         flat_bestvz;
    Float_t         flat_bestvxError;
    Float_t         flat_bestvyError;
    Float_t         flat_bestvzError;
    
    // B meson information (scalar per candidate)
    Short_t         flat_bMesonType;        // 0=B+, 1=B0, 2=Bc
    Float_t         flat_bMesonPt;
    Float_t         flat_bMesonEta;
    Float_t         flat_bMesonPhi;
    Float_t         flat_bMesonY;
    Float_t         flat_bMesonMass;
    Float_t         flat_bMesonCharge;
    Float_t         flat_bMesonMVA;
    Bool_t          flat_bMesonMatchGEN;
    
    // Vertex quality
    Float_t         flat_bMesonVtxChi2;
    Float_t         flat_bMesonVtxNdof;
    Float_t         flat_bMesonVtxProb;
    Float_t         flat_bMesonVtxX;
    Float_t         flat_bMesonVtxY;
    Float_t         flat_bMesonVtxZ;
    Float_t         flat_bMesonVtxXError;
    Float_t         flat_bMesonVtxYError;
    Float_t         flat_bMesonVtxZError;
    
    // Topological variables
    Float_t         flat_bMesonPointingAngle2D;
    Float_t         flat_bMesonPointingAngle3D;
    Float_t         flat_bMesonDecayLength2D;
    Float_t         flat_bMesonDecayLength3D;
    Float_t         flat_bMesonDecayLengthSig2D;
    Float_t         flat_bMesonDecayLengthSig3D;
    
    // Dimuon information
    Float_t         flat_dimuonPt;
    Float_t         flat_dimuonEta;
    Float_t         flat_dimuonPhi;
    Float_t         flat_dimuonY;
    Float_t         flat_dimuonMass;
    Float_t         flat_dimuonCharge;
    
    // Individual muon information (muon1 = leading, muon2 = subleading)
    Float_t         flat_muon1Pt;
    Float_t         flat_muon1Eta;
    Float_t         flat_muon1Phi;
    Float_t         flat_muon1Charge;
    Bool_t          flat_muon1IsGlobal;
    Bool_t          flat_muon1IsTracker;
    Bool_t          flat_muon1IsPF;
    Float_t         flat_muon1Dxy;
    Float_t         flat_muon1Dz;
    Short_t         flat_muon1NHits;
    Short_t         flat_muon1NPixelHits;
    Short_t         flat_muon1NStations;
    
    Float_t         flat_muon2Pt;
    Float_t         flat_muon2Eta;
    Float_t         flat_muon2Phi;
    Float_t         flat_muon2Charge;
    Bool_t          flat_muon2IsGlobal;
    Bool_t          flat_muon2IsTracker;
    Bool_t          flat_muon2IsPF;
    Float_t         flat_muon2Dxy;
    Float_t         flat_muon2Dz;
    Short_t         flat_muon2NHits;
    Short_t         flat_muon2NPixelHits;
    Short_t         flat_muon2NStations;
    
    // Additional track information (for B+ = track1, for B0 = track1+track2, for Bc = track1)
    UShort_t        flat_nTracks;
    Float_t         flat_track1Pt;
    Float_t         flat_track1Eta;
    Float_t         flat_track1Phi;
    Float_t         flat_track1Charge;
    Float_t         flat_track1Dxy;
    Float_t         flat_track1Dz;
    Float_t         flat_track1PtError;
    Short_t         flat_track1NHits;
    Short_t         flat_track1NPixelHits;
    Float_t         flat_track1Chi2;
    Bool_t          flat_track1HighPurity;
    Short_t         flat_track1PID;
    
    Float_t         flat_track2Pt;
    Float_t         flat_track2Eta;
    Float_t         flat_track2Phi;
    Float_t         flat_track2Charge;
    Float_t         flat_track2Dxy;
    Float_t         flat_track2Dz;
    Float_t         flat_track2PtError;
    Short_t         flat_track2NHits;
    Short_t         flat_track2NPixelHits;
    Float_t         flat_track2Chi2;
    Bool_t          flat_track2HighPurity;
    Short_t         flat_track2PID;
    
    // K*0 information (for B0 only)
    Float_t         flat_kstarPt;
    Float_t         flat_kstarEta;
    Float_t         flat_kstarPhi;
    Float_t         flat_kstarMass;
    
    // Constructor and methods
    FlattenBDiMuMu(const char* inputFileName, const char* outputFileName);
    FlattenBDiMuMu(TChain* inputChain, const char* outputFileName);
    ~FlattenBDiMuMu();
    
    void SetupInputTree();
    void SetupOutputTree();
    void ProcessEvents();
    void FillFlatEntry(int bmeson_idx);
    void ResetFlatBranches();
    
    // Helper functions
    bool IsBPlus(int idx) { return bMesonType[idx] == 0; }
    bool IsBZero(int idx) { return bMesonType[idx] == 1; }
    bool IsBc(int idx) { return bMesonType[idx] == 2; }
    
    const char* GetBMesonName(int type) {
        if (type == 0) return "B+";
        if (type == 1) return "B0";
        if (type == 2) return "Bc";
        return "Unknown";
    }
};

#endif