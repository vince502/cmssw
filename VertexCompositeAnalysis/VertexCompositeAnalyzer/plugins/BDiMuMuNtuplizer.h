// -*- C++ -*-
//
// Package:    VertexCompositeAnalyzer
// Class:      BDiMuMuNtuplizer
// 
/**\class BDiMuMuNtuplizer BDiMuMuNtuplizer.h VertexCompositeAnalysis/VertexCompositeAnalyzer/plugins/BDiMuMuNtuplizer.h

 Description: EDAnalyzer for creating ROOT trees from B and Bc meson candidates

 Implementation:
     Reads B+, B0, and Bc collections from BDiMuMuProducer and creates comprehensive ROOT trees
     for offline analysis including kinematics, vertex quality, topological variables, and event info
*/
//
// Original Author:  Based on PATCompositeTreeProducer, specialized for B meson analysis
//
//

#ifndef VertexCompositeAnalysis__BDIMUMUONTUPLIZER_H
#define VertexCompositeAnalysis__BDIMUMUONTUPLIZER_H

// system includes
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <math.h>

// ROOT includes
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include <TFile.h>
#include <TVector3.h>
#include <TMath.h>
#include <Math/Functions.h>
#include <Math/SVector.h>
#include <Math/SMatrix.h>

// CMSSW Framework includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Common/interface/TriggerNames.h"

// Data format includes
#include "DataFormats/TrackReco/interface/DeDxData.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/RecoCandidate/interface/RecoCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/HeavyIonEvent/interface/CentralityBins.h"
#include "DataFormats/HeavyIonEvent/interface/Centrality.h"
#include "DataFormats/HeavyIonEvent/interface/EvtPlane.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/Math/interface/deltaR.h"

// Services includes
#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

// Constants
#define PI 3.1416
#define MAXBMESON 1000
#define MAXTRACK 10
#define MAXTRG 1024
#define MAXSEL 100

typedef ROOT::Math::SMatrix<double, 3, 3, ROOT::Math::MatRepSym<double, 3>> SMatrixSym3D;
typedef ROOT::Math::SVector<double, 3> SVector3;

class BDiMuMuNtuplizer : public edm::one::EDAnalyzer<edm::one::WatchRuns> {
public:
  explicit BDiMuMuNtuplizer(const edm::ParameterSet&);
  ~BDiMuMuNtuplizer();

  using MVACollection = std::vector<float>;

private:
  virtual void beginJob() override;
  virtual void beginRun(const edm::Run&, const edm::EventSetup&) override;
  virtual void endRun(const edm::Run&, const edm::EventSetup&) override {};
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  virtual void initTree();
  virtual void resetBranches();
  virtual void fillEventInfo(const edm::Event&, const edm::EventSetup&);
  virtual void fillBMesons(const edm::Event&, const edm::EventSetup&);
  virtual void fillDaughterInfo(const pat::CompositeCandidate& bmeson, int bIndex);
  virtual void fillGenInfo(const edm::Event&, const edm::EventSetup&);

  reco::GenParticleRef findLastParticle(const reco::GenParticleRef&);
  reco::GenParticleRef findMother(const reco::GenParticleRef&);
  bool matchToGen(const pat::CompositeCandidate& bmeson, const edm::Handle<reco::GenParticleCollection>& genHandle, double deltaR = 0.3);

  // ----------member data ---------------------------
  edm::Service<TFileService> fs;
  TTree* bDiMuMuTree;

  // Input tokens
  edm::EDGetTokenT<pat::CompositeCandidateCollection> bPlusToken_;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> bZeroToken_;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> bcToken_;
  edm::EDGetTokenT<MVACollection> bPlusMVAToken_;
  edm::EDGetTokenT<MVACollection> bZeroMVAToken_;
  edm::EDGetTokenT<MVACollection> bcMVAToken_;
  edm::EDGetTokenT<reco::VertexCollection> vertexToken_;
  edm::EDGetTokenT<reco::BeamSpot> beamSpotToken_;
  edm::EDGetTokenT<reco::GenParticleCollection> genToken_;
  edm::EDGetTokenT<edm::TriggerResults> triggerToken_;
  edm::EDGetTokenT<reco::Centrality> centralityToken_;
  edm::EDGetTokenT<int> centralityBinToken_;

  // Configuration options
  bool doGenMatching_;
  bool saveBPlus_;
  bool saveBZero_;
  bool saveBc_;
  bool saveFullInfo_;
  bool isMC_;
  double genMatchingDeltaR_;

  // Tree branches
  // Event information
  UInt_t   runNb;
  ULong64_t eventNb;
  UInt_t   lumiSection;
  Short_t  centrality;
  Int_t    Ntrkoffline;
  Short_t  nPV;
  Float_t  bestvx;
  Float_t  bestvy;
  Float_t  bestvz;
  Float_t  bestvxError;
  Float_t  bestvyError;
  Float_t  bestvzError;
  Bool_t   trigHLT[MAXTRG];
  Bool_t   evtSel[MAXSEL];

  // B meson candidate information
  UShort_t nBMeson;
  Short_t  bMesonType[MAXBMESON];     // 0=B+, 1=B0, 2=Bc
  Float_t  bMesonPt[MAXBMESON];
  Float_t  bMesonEta[MAXBMESON];
  Float_t  bMesonPhi[MAXBMESON];
  Float_t  bMesonY[MAXBMESON];
  Float_t  bMesonMass[MAXBMESON];
  Float_t  bMesonCharge[MAXBMESON];
  Float_t  bMesonMVA[MAXBMESON];
  Bool_t   bMesonMatchGEN[MAXBMESON];

  // Vertex quality
  Float_t  bMesonVtxChi2[MAXBMESON];
  Float_t  bMesonVtxNdof[MAXBMESON];
  Float_t  bMesonVtxProb[MAXBMESON];
  Float_t  bMesonVtxX[MAXBMESON];
  Float_t  bMesonVtxY[MAXBMESON];
  Float_t  bMesonVtxZ[MAXBMESON];
  Float_t  bMesonVtxXError[MAXBMESON];
  Float_t  bMesonVtxYError[MAXBMESON];
  Float_t  bMesonVtxZError[MAXBMESON];

  // Topological variables
  Float_t  bMesonPointingAngle2D[MAXBMESON];
  Float_t  bMesonPointingAngle3D[MAXBMESON];
  Float_t  bMesonDecayLength2D[MAXBMESON];
  Float_t  bMesonDecayLength3D[MAXBMESON];
  Float_t  bMesonDecayLengthError2D[MAXBMESON];
  Float_t  bMesonDecayLengthError3D[MAXBMESON];
  Float_t  bMesonDecayLengthSig2D[MAXBMESON];
  Float_t  bMesonDecayLengthSig3D[MAXBMESON];

  // Dimuon daughter information
  Float_t  dimuonPt[MAXBMESON];
  Float_t  dimuonEta[MAXBMESON];
  Float_t  dimuonPhi[MAXBMESON];
  Float_t  dimuonY[MAXBMESON];
  Float_t  dimuonMass[MAXBMESON];
  Float_t  dimuonCharge[MAXBMESON];
  Float_t  dimuonVtxChi2[MAXBMESON];
  Float_t  dimuonVtxNdof[MAXBMESON];
  Float_t  dimuonVtxProb[MAXBMESON];

  // Individual muon information (candidate first, then muon index)
  Float_t  muonPt[MAXBMESON][2];
  Float_t  muonEta[MAXBMESON][2];
  Float_t  muonPhi[MAXBMESON][2];
  Float_t  muonCharge[MAXBMESON][2];
  Bool_t   muonIsGlobal[MAXBMESON][2];
  Bool_t   muonIsTracker[MAXBMESON][2];
  Bool_t   muonIsPF[MAXBMESON][2];
  Bool_t   muonIsTight[MAXBMESON][2];
  Bool_t   muonIsSoft[MAXBMESON][2];
  Float_t  muonDxy[MAXBMESON][2];
  Float_t  muonDz[MAXBMESON][2];
  Float_t  muonDxyError[MAXBMESON][2];
  Float_t  muonDzError[MAXBMESON][2];
  Short_t  muonNHits[MAXBMESON][2];
  Short_t  muonNPixelHits[MAXBMESON][2];
  Short_t  muonNStations[MAXBMESON][2];

  // Additional track information (candidate first, then track index)
  UShort_t nTracks[MAXBMESON];        // Number of additional tracks (1 for B+/Bc, 2 for B0)
  Float_t  trackPt[MAXBMESON][MAXTRACK];
  Float_t  trackEta[MAXBMESON][MAXTRACK];
  Float_t  trackPhi[MAXBMESON][MAXTRACK];
  Float_t  trackCharge[MAXBMESON][MAXTRACK];
  Float_t  trackDxy[MAXBMESON][MAXTRACK];
  Float_t  trackDz[MAXBMESON][MAXTRACK];
  Float_t  trackDxyError[MAXBMESON][MAXTRACK];
  Float_t  trackDzError[MAXBMESON][MAXTRACK];
  Float_t  trackPtError[MAXBMESON][MAXTRACK];
  Short_t  trackNHits[MAXBMESON][MAXTRACK];
  Short_t  trackNPixelHits[MAXBMESON][MAXTRACK];
  Float_t  trackChi2[MAXBMESON][MAXTRACK];
  Short_t  trackNdof[MAXBMESON][MAXTRACK];
  Bool_t   trackHighPurity[MAXBMESON][MAXTRACK];
  Float_t  trackDedx[MAXBMESON][MAXTRACK];
  Short_t  trackPID[MAXBMESON][MAXTRACK];  // PDG ID assumption (211 for pion, 321 for kaon)

  // K*0 information (for B0 only)
  Float_t  kstarPt[MAXBMESON];
  Float_t  kstarEta[MAXBMESON];
  Float_t  kstarPhi[MAXBMESON];
  Float_t  kstarMass[MAXBMESON];

  // Generator information (MC only)
  UShort_t nGenBMeson;
  Short_t  genBMesonType[MAXBMESON];
  Float_t  genBMesonPt[MAXBMESON];
  Float_t  genBMesonEta[MAXBMESON];
  Float_t  genBMesonPhi[MAXBMESON];
  Float_t  genBMesonY[MAXBMESON];
  Float_t  genBMesonMass[MAXBMESON];
  Short_t  genBMesonPdgId[MAXBMESON];
  Short_t  genBMesonMotherId[MAXBMESON];
  Float_t  genDimuonPt[MAXBMESON];
  Float_t  genDimuonEta[MAXBMESON];
  Float_t  genDimuonPhi[MAXBMESON];
  Float_t  genDimuonMass[MAXBMESON];
  Float_t  genMuonPt[MAXBMESON][2];
  Float_t  genMuonEta[MAXBMESON][2];
  Float_t  genMuonPhi[MAXBMESON][2];
  Float_t  genTrackPt[MAXBMESON][MAXTRACK];
  Float_t  genTrackEta[MAXBMESON][MAXTRACK];
  Float_t  genTrackPhi[MAXBMESON][MAXTRACK];
  Short_t  genTrackPdgId[MAXBMESON][MAXTRACK];
};

#endif