//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Jul 30 2025 by ROOT version 6.26/06
// Based on BDiMuMuNtuplizer output for B meson analysis
//////////////////////////////////////////////////////////

#ifndef BDiMuMuNtuple_h
#define BDiMuMuNtuple_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

// Header file for the classes stored in the TTree if any.

namespace DataFormat {

   const Int_t MAXBMESON = 1000;
   const Int_t MAXTRACK = 10;

   class BDiMuMuNtuple {
   public :
      TTree          *fChain;   //!pointer to the analyzed TTree or TChain
      Int_t           fCurrent; //!current Tree number in a TChain

      // Declaration of leaf types
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
      
      // B meson candidate information
      UShort_t        nBMeson;
      Short_t         bMesonType[MAXBMESON];
      Float_t         bMesonPt[MAXBMESON];
      Float_t         bMesonEta[MAXBMESON];
      Float_t         bMesonPhi[MAXBMESON];
      Float_t         bMesonY[MAXBMESON];
      Float_t         bMesonMass[MAXBMESON];
      Float_t         bMesonCharge[MAXBMESON];
      Float_t         bMesonMVA[MAXBMESON];
      Bool_t          bMesonMatchGEN[MAXBMESON];
      
      // Vertex quality
      Float_t         bMesonVtxChi2[MAXBMESON];
      Float_t         bMesonVtxNdof[MAXBMESON];
      Float_t         bMesonVtxProb[MAXBMESON];
      Float_t         bMesonVtxX[MAXBMESON];
      Float_t         bMesonVtxY[MAXBMESON];
      Float_t         bMesonVtxZ[MAXBMESON];
      Float_t         bMesonVtxXError[MAXBMESON];
      Float_t         bMesonVtxYError[MAXBMESON];
      Float_t         bMesonVtxZError[MAXBMESON];
      
      // Topological variables
      Float_t         bMesonPointingAngle2D[MAXBMESON];
      Float_t         bMesonPointingAngle3D[MAXBMESON];
      Float_t         bMesonDecayLength2D[MAXBMESON];
      Float_t         bMesonDecayLength3D[MAXBMESON];
      Float_t         bMesonDecayLengthSig2D[MAXBMESON];
      Float_t         bMesonDecayLengthSig3D[MAXBMESON];
      
      // Dimuon information
      Float_t         dimuonPt[MAXBMESON];
      Float_t         dimuonEta[MAXBMESON];
      Float_t         dimuonPhi[MAXBMESON];
      Float_t         dimuonY[MAXBMESON];
      Float_t         dimuonMass[MAXBMESON];
      Float_t         dimuonCharge[MAXBMESON];
      
      // Muon information
      Float_t         muonPt[MAXBMESON][2];
      Float_t         muonEta[MAXBMESON][2];
      Float_t         muonPhi[MAXBMESON][2];
      Float_t         muonCharge[MAXBMESON][2];
      Bool_t          muonIsGlobal[MAXBMESON][2];
      Bool_t          muonIsTracker[MAXBMESON][2];
      Bool_t          muonIsPF[MAXBMESON][2];
      Float_t         muonDxy[MAXBMESON][2];
      Float_t         muonDz[MAXBMESON][2];
      Short_t         muonNHits[MAXBMESON][2];
      Short_t         muonNPixelHits[MAXBMESON][2];
      Short_t         muonNStations[MAXBMESON][2];
      
      // Additional track information
      UShort_t        nTracks[MAXBMESON];
      Float_t         trackPt[MAXBMESON][MAXTRACK];
      Float_t         trackEta[MAXBMESON][MAXTRACK];
      Float_t         trackPhi[MAXBMESON][MAXTRACK];
      Float_t         trackCharge[MAXBMESON][MAXTRACK];
      Float_t         trackDxy[MAXBMESON][MAXTRACK];
      Float_t         trackDz[MAXBMESON][MAXTRACK];
      Float_t         trackPtError[MAXBMESON][MAXTRACK];
      Short_t         trackNHits[MAXBMESON][MAXTRACK];
      Short_t         trackNPixelHits[MAXBMESON][MAXTRACK];
      Float_t         trackChi2[MAXBMESON][MAXTRACK];
      Bool_t          trackHighPurity[MAXBMESON][MAXTRACK];
      Short_t         trackPID[MAXBMESON][MAXTRACK];
      
      // K*0 information (for B0 only)
      Float_t         kstarPt[MAXBMESON];
      Float_t         kstarEta[MAXBMESON];
      Float_t         kstarPhi[MAXBMESON];
      Float_t         kstarMass[MAXBMESON];
      
      // Generator information (MC only)
      UShort_t        nGenBMeson;
      Short_t         genBMesonType[MAXBMESON];
      Float_t         genBMesonPt[MAXBMESON];
      Float_t         genBMesonEta[MAXBMESON];
      Float_t         genBMesonPhi[MAXBMESON];
      Float_t         genBMesonY[MAXBMESON];
      Float_t         genBMesonMass[MAXBMESON];
      Short_t         genBMesonPdgId[MAXBMESON];

      // List of branches
      TBranch        *b_runNb;   //!
      TBranch        *b_eventNb;   //!
      TBranch        *b_lumiSection;   //!
      TBranch        *b_centrality;   //!
      TBranch        *b_Ntrkoffline;   //!
      TBranch        *b_nPV;   //!
      TBranch        *b_bestvx;   //!
      TBranch        *b_bestvy;   //!
      TBranch        *b_bestvz;   //!
      TBranch        *b_bestvxError;   //!
      TBranch        *b_bestvyError;   //!
      TBranch        *b_bestvzError;   //!
      TBranch        *b_nBMeson;   //!
      TBranch        *b_bMesonType;   //!
      TBranch        *b_bMesonPt;   //!
      TBranch        *b_bMesonEta;   //!
      TBranch        *b_bMesonPhi;   //!
      TBranch        *b_bMesonY;   //!
      TBranch        *b_bMesonMass;   //!
      TBranch        *b_bMesonCharge;   //!
      TBranch        *b_bMesonMVA;   //!
      TBranch        *b_bMesonMatchGEN;   //!
      TBranch        *b_bMesonVtxChi2;   //!
      TBranch        *b_bMesonVtxNdof;   //!
      TBranch        *b_bMesonVtxProb;   //!
      TBranch        *b_bMesonVtxX;   //!
      TBranch        *b_bMesonVtxY;   //!
      TBranch        *b_bMesonVtxZ;   //!
      TBranch        *b_bMesonVtxXError;   //!
      TBranch        *b_bMesonVtxYError;   //!
      TBranch        *b_bMesonVtxZError;   //!
      TBranch        *b_bMesonPointingAngle2D;   //!
      TBranch        *b_bMesonPointingAngle3D;   //!
      TBranch        *b_bMesonDecayLength2D;   //!
      TBranch        *b_bMesonDecayLength3D;   //!
      TBranch        *b_bMesonDecayLengthSig2D;   //!
      TBranch        *b_bMesonDecayLengthSig3D;   //!
      TBranch        *b_dimuonPt;   //!
      TBranch        *b_dimuonEta;   //!
      TBranch        *b_dimuonPhi;   //!
      TBranch        *b_dimuonY;   //!
      TBranch        *b_dimuonMass;   //!
      TBranch        *b_dimuonCharge;   //!
      TBranch        *b_muonPt;   //!
      TBranch        *b_muonEta;   //!
      TBranch        *b_muonPhi;   //!
      TBranch        *b_muonCharge;   //!
      TBranch        *b_muonIsGlobal;   //!
      TBranch        *b_muonIsTracker;   //!
      TBranch        *b_muonIsPF;   //!
      TBranch        *b_muonDxy;   //!
      TBranch        *b_muonDz;   //!
      TBranch        *b_muonNHits;   //!
      TBranch        *b_muonNPixelHits;   //!
      TBranch        *b_muonNStations;   //!
      TBranch        *b_nTracks;   //!
      TBranch        *b_trackPt;   //!
      TBranch        *b_trackEta;   //!
      TBranch        *b_trackPhi;   //!
      TBranch        *b_trackCharge;   //!
      TBranch        *b_trackDxy;   //!
      TBranch        *b_trackDz;   //!
      TBranch        *b_trackPtError;   //!
      TBranch        *b_trackNHits;   //!
      TBranch        *b_trackNPixelHits;   //!
      TBranch        *b_trackChi2;   //!
      TBranch        *b_trackHighPurity;   //!
      TBranch        *b_trackPID;   //!
      TBranch        *b_kstarPt;   //!
      TBranch        *b_kstarEta;   //!
      TBranch        *b_kstarPhi;   //!
      TBranch        *b_kstarMass;   //!
      TBranch        *b_nGenBMeson;   //!
      TBranch        *b_genBMesonType;   //!
      TBranch        *b_genBMesonPt;   //!
      TBranch        *b_genBMesonEta;   //!
      TBranch        *b_genBMesonPhi;   //!
      TBranch        *b_genBMesonY;   //!
      TBranch        *b_genBMesonMass;   //!
      TBranch        *b_genBMesonPdgId;   //!

      BDiMuMuNtuple(TTree *tree=0);
      virtual ~BDiMuMuNtuple();
      virtual Int_t    Cut(Long64_t entry);
      virtual Int_t    GetEntry(Long64_t entry);
      virtual Long64_t LoadTree(Long64_t entry);
      virtual void     Init(TTree *tree);
      virtual void     Loop();
      virtual Bool_t   Notify();
      virtual void     Show(Long64_t entry = -1);
      
      // Helper functions for analysis
      virtual Bool_t   IsBPlus(Int_t idx) { return bMesonType[idx] == 0; }
      virtual Bool_t   IsBZero(Int_t idx) { return bMesonType[idx] == 1; }
      virtual Bool_t   IsBc(Int_t idx) { return bMesonType[idx] == 2; }
      virtual Float_t  GetBMesonMass(Int_t type) { 
         if (type == 0) return 5.27932; // B+
         if (type == 1) return 5.27964; // B0
         if (type == 2) return 6.2756;  // Bc
         return 0; 
      }
   };
}

#endif

#ifdef BDiMuMuNtuple_cxx

namespace DataFormat {

BDiMuMuNtuple::BDiMuMuNtuple(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("BDiMuMu_Ntuple.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("BDiMuMu_Ntuple.root");
      }
      f->GetObject("bDiMuMuTree",tree);

   }
   Init(tree);
}

BDiMuMuNtuple::~BDiMuMuNtuple()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t BDiMuMuNtuple::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}

Long64_t BDiMuMuNtuple::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void BDiMuMuNtuple::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("runNb", &runNb, &b_runNb);
   fChain->SetBranchAddress("eventNb", &eventNb, &b_eventNb);
   fChain->SetBranchAddress("lumiSection", &lumiSection, &b_lumiSection);
   fChain->SetBranchAddress("centrality", &centrality, &b_centrality);
   fChain->SetBranchAddress("Ntrkoffline", &Ntrkoffline, &b_Ntrkoffline);
   fChain->SetBranchAddress("nPV", &nPV, &b_nPV);
   fChain->SetBranchAddress("bestvx", &bestvx, &b_bestvx);
   fChain->SetBranchAddress("bestvy", &bestvy, &b_bestvy);
   fChain->SetBranchAddress("bestvz", &bestvz, &b_bestvz);
   fChain->SetBranchAddress("bestvxError", &bestvxError, &b_bestvxError);
   fChain->SetBranchAddress("bestvyError", &bestvyError, &b_bestvyError);
   fChain->SetBranchAddress("bestvzError", &bestvzError, &b_bestvzError);
   fChain->SetBranchAddress("nBMeson", &nBMeson, &b_nBMeson);
   fChain->SetBranchAddress("bMesonType", bMesonType, &b_bMesonType);
   fChain->SetBranchAddress("bMesonPt", bMesonPt, &b_bMesonPt);
   fChain->SetBranchAddress("bMesonEta", bMesonEta, &b_bMesonEta);
   fChain->SetBranchAddress("bMesonPhi", bMesonPhi, &b_bMesonPhi);
   fChain->SetBranchAddress("bMesonY", bMesonY, &b_bMesonY);
   fChain->SetBranchAddress("bMesonMass", bMesonMass, &b_bMesonMass);
   fChain->SetBranchAddress("bMesonCharge", bMesonCharge, &b_bMesonCharge);
   fChain->SetBranchAddress("bMesonMVA", bMesonMVA, &b_bMesonMVA);
   fChain->SetBranchAddress("bMesonMatchGEN", bMesonMatchGEN, &b_bMesonMatchGEN);
   fChain->SetBranchAddress("bMesonVtxChi2", bMesonVtxChi2, &b_bMesonVtxChi2);
   fChain->SetBranchAddress("bMesonVtxNdof", bMesonVtxNdof, &b_bMesonVtxNdof);
   fChain->SetBranchAddress("bMesonVtxProb", bMesonVtxProb, &b_bMesonVtxProb);
   fChain->SetBranchAddress("bMesonVtxX", bMesonVtxX, &b_bMesonVtxX);
   fChain->SetBranchAddress("bMesonVtxY", bMesonVtxY, &b_bMesonVtxY);
   fChain->SetBranchAddress("bMesonVtxZ", bMesonVtxZ, &b_bMesonVtxZ);
   fChain->SetBranchAddress("bMesonVtxXError", bMesonVtxXError, &b_bMesonVtxXError);
   fChain->SetBranchAddress("bMesonVtxYError", bMesonVtxYError, &b_bMesonVtxYError);
   fChain->SetBranchAddress("bMesonVtxZError", bMesonVtxZError, &b_bMesonVtxZError);
   fChain->SetBranchAddress("bMesonPointingAngle2D", bMesonPointingAngle2D, &b_bMesonPointingAngle2D);
   fChain->SetBranchAddress("bMesonPointingAngle3D", bMesonPointingAngle3D, &b_bMesonPointingAngle3D);
   fChain->SetBranchAddress("bMesonDecayLength2D", bMesonDecayLength2D, &b_bMesonDecayLength2D);
   fChain->SetBranchAddress("bMesonDecayLength3D", bMesonDecayLength3D, &b_bMesonDecayLength3D);
   fChain->SetBranchAddress("bMesonDecayLengthSig2D", bMesonDecayLengthSig2D, &b_bMesonDecayLengthSig2D);
   fChain->SetBranchAddress("bMesonDecayLengthSig3D", bMesonDecayLengthSig3D, &b_bMesonDecayLengthSig3D);
   fChain->SetBranchAddress("dimuonPt", dimuonPt, &b_dimuonPt);
   fChain->SetBranchAddress("dimuonEta", dimuonEta, &b_dimuonEta);
   fChain->SetBranchAddress("dimuonPhi", dimuonPhi, &b_dimuonPhi);
   fChain->SetBranchAddress("dimuonY", dimuonY, &b_dimuonY);
   fChain->SetBranchAddress("dimuonMass", dimuonMass, &b_dimuonMass);
   fChain->SetBranchAddress("dimuonCharge", dimuonCharge, &b_dimuonCharge);
   fChain->SetBranchAddress("muonPt", muonPt, &b_muonPt);
   fChain->SetBranchAddress("muonEta", muonEta, &b_muonEta);
   fChain->SetBranchAddress("muonPhi", muonPhi, &b_muonPhi);
   fChain->SetBranchAddress("muonCharge", muonCharge, &b_muonCharge);
   fChain->SetBranchAddress("muonIsGlobal", muonIsGlobal, &b_muonIsGlobal);
   fChain->SetBranchAddress("muonIsTracker", muonIsTracker, &b_muonIsTracker);
   fChain->SetBranchAddress("muonIsPF", muonIsPF, &b_muonIsPF);
   fChain->SetBranchAddress("muonDxy", muonDxy, &b_muonDxy);
   fChain->SetBranchAddress("muonDz", muonDz, &b_muonDz);
   fChain->SetBranchAddress("muonNHits", muonNHits, &b_muonNHits);
   fChain->SetBranchAddress("muonNPixelHits", muonNPixelHits, &b_muonNPixelHits);
   fChain->SetBranchAddress("muonNStations", muonNStations, &b_muonNStations);
   fChain->SetBranchAddress("nTracks", nTracks, &b_nTracks);
   fChain->SetBranchAddress("trackPt", trackPt, &b_trackPt);
   fChain->SetBranchAddress("trackEta", trackEta, &b_trackEta);
   fChain->SetBranchAddress("trackPhi", trackPhi, &b_trackPhi);
   fChain->SetBranchAddress("trackCharge", trackCharge, &b_trackCharge);
   fChain->SetBranchAddress("trackDxy", trackDxy, &b_trackDxy);
   fChain->SetBranchAddress("trackDz", trackDz, &b_trackDz);
   fChain->SetBranchAddress("trackPtError", trackPtError, &b_trackPtError);
   fChain->SetBranchAddress("trackNHits", trackNHits, &b_trackNHits);
   fChain->SetBranchAddress("trackNPixelHits", trackNPixelHits, &b_trackNPixelHits);
   fChain->SetBranchAddress("trackChi2", trackChi2, &b_trackChi2);
   fChain->SetBranchAddress("trackHighPurity", trackHighPurity, &b_trackHighPurity);
   fChain->SetBranchAddress("trackPID", trackPID, &b_trackPID);
   fChain->SetBranchAddress("kstarPt", kstarPt, &b_kstarPt);
   fChain->SetBranchAddress("kstarEta", kstarEta, &b_kstarEta);
   fChain->SetBranchAddress("kstarPhi", kstarPhi, &b_kstarPhi);
   fChain->SetBranchAddress("kstarMass", kstarMass, &b_kstarMass);
   fChain->SetBranchAddress("nGenBMeson", &nGenBMeson, &b_nGenBMeson);
   fChain->SetBranchAddress("genBMesonType", genBMesonType, &b_genBMesonType);
   fChain->SetBranchAddress("genBMesonPt", genBMesonPt, &b_genBMesonPt);
   fChain->SetBranchAddress("genBMesonEta", genBMesonEta, &b_genBMesonEta);
   fChain->SetBranchAddress("genBMesonPhi", genBMesonPhi, &b_genBMesonPhi);
   fChain->SetBranchAddress("genBMesonY", genBMesonY, &b_genBMesonY);
   fChain->SetBranchAddress("genBMesonMass", genBMesonMass, &b_genBMesonMass);
   fChain->SetBranchAddress("genBMesonPdgId", genBMesonPdgId, &b_genBMesonPdgId);
   Notify();
}

Bool_t BDiMuMuNtuple::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void BDiMuMuNtuple::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}

Int_t BDiMuMuNtuple::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}

void BDiMuMuNtuple::Loop()
{
//   In a ROOT session, you can do:
//      root> .L BDiMuMuNtuple.C
//      root> BDiMuMuNtuple t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    if (fChain->GetEntry(jentry) <= 0) break;
// by
//    if (fChain->GetEntry(jentry, getall) <= 0) break;

   if (fChain == 0) return;

   Long64_t nentries = fChain->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
      
      // User analysis code goes here
      // Example: Loop over B meson candidates
      for (int i = 0; i < nBMeson; i++) {
         // Analysis for each B meson candidate
         // std::cout << "B meson " << i << " pT: " << bMesonPt[i] << " mass: " << bMesonMass[i] << std::endl;
      }
   }
}

} // end namespace DataFormat

#endif // #ifdef BDiMuMuNtuple_cxx