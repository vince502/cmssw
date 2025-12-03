//////////////////////////////////////////////////////////
// BDiMuMuFlatProducer
// Simple flat tree producer for B meson analysis
// Converts CMSSW collections into flat ROOT trees
//////////////////////////////////////////////////////////

#ifndef BDIMUMUONFLATPRODUCER_H
#define BDIMUMUONFLATPRODUCER_H

#include <memory>
#include <string>
#include <vector>

// ROOT includes
#include <TTree.h>
#include <TFile.h>
#include <TMath.h>

// CMSSW Framework includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

// Data format includes
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/HeavyIonEvent/interface/Centrality.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

// Constants
const Int_t MAXBMESON_FLAT = 1000;
const Int_t MAXTRACK_FLAT = 10;

class BDiMuMuFlatProducer : public edm::one::EDAnalyzer<edm::one::WatchRuns> {
public:
    explicit BDiMuMuFlatProducer(const edm::ParameterSet&);
    ~BDiMuMuFlatProducer();

private:
    virtual void beginJob() override;
    virtual void beginRun(const edm::Run&, const edm::EventSetup&) override;
    virtual void endRun(const edm::Run&, const edm::EventSetup&) override {};
    virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
    virtual void endJob() override;
    
    void initTree();
    void resetBranches();
    void fillEventInfo(const edm::Event&);
    void fillBCandidates(const edm::Handle<pat::CompositeCandidateCollection>&, int type);
    void fillGenInfo(const edm::Handle<reco::GenParticleCollection>&);

    // Services and tokens
    edm::Service<TFileService> fs;
    TTree* flatTree;
    
    // Input tokens
    edm::EDGetTokenT<pat::CompositeCandidateCollection> bPlusToken_;
    edm::EDGetTokenT<pat::CompositeCandidateCollection> bZeroToken_;
    edm::EDGetTokenT<pat::CompositeCandidateCollection> bcToken_;
    edm::EDGetTokenT<reco::VertexCollection> vertexToken_;
    edm::EDGetTokenT<reco::BeamSpot> beamSpotToken_;
    edm::EDGetTokenT<reco::GenParticleCollection> genToken_;
    edm::EDGetTokenT<reco::Centrality> centralityToken_;
    edm::EDGetTokenT<int> centralityBinToken_;
    
    // Configuration
    bool isMC_;
    bool saveBPlus_;
    bool saveBZero_;
    bool saveBc_;
    
    // Tree branches - Event info
    UInt_t   runNb;
    ULong64_t eventNb;
    UInt_t   lumiSection;
    Short_t  centrality;
    Short_t  nPV;
    Float_t  bestvx;
    Float_t  bestvy;
    Float_t  bestvz;
    
    // Tree branches - B meson candidates
    UShort_t nBMeson;
    Short_t  bMesonType[MAXBMESON_FLAT];        // 0=B+, 1=B0, 2=Bc
    Float_t  bMesonPt[MAXBMESON_FLAT];
    Float_t  bMesonEta[MAXBMESON_FLAT];
    Float_t  bMesonPhi[MAXBMESON_FLAT];
    Float_t  bMesonY[MAXBMESON_FLAT];
    Float_t  bMesonMass[MAXBMESON_FLAT];
    Float_t  bMesonCharge[MAXBMESON_FLAT];
    
    // Vertex quality
    Float_t  bMesonVtxChi2[MAXBMESON_FLAT];
    Float_t  bMesonVtxNdof[MAXBMESON_FLAT];
    Float_t  bMesonVtxProb[MAXBMESON_FLAT];
    
    // Topological variables
    Float_t  bMesonPointingAngle2D[MAXBMESON_FLAT];
    Float_t  bMesonPointingAngle3D[MAXBMESON_FLAT];
    Float_t  bMesonDecayLength2D[MAXBMESON_FLAT];
    Float_t  bMesonDecayLength3D[MAXBMESON_FLAT];
    
    // Dimuon information
    Float_t  dimuonPt[MAXBMESON_FLAT];
    Float_t  dimuonEta[MAXBMESON_FLAT];
    Float_t  dimuonPhi[MAXBMESON_FLAT];
    Float_t  dimuonMass[MAXBMESON_FLAT];
    Float_t  dimuonCharge[MAXBMESON_FLAT];
    
    // Muon information (two muons per dimuon)
    Float_t  muonPt[MAXBMESON_FLAT][2];
    Float_t  muonEta[MAXBMESON_FLAT][2];
    Float_t  muonPhi[MAXBMESON_FLAT][2];
    Float_t  muonCharge[MAXBMESON_FLAT][2];
    Bool_t   muonIsGlobal[MAXBMESON_FLAT][2];
    Bool_t   muonIsTracker[MAXBMESON_FLAT][2];
    Bool_t   muonIsPF[MAXBMESON_FLAT][2];
    
    // Additional track information
    UShort_t nTracks[MAXBMESON_FLAT];
    Float_t  trackPt[MAXBMESON_FLAT][MAXTRACK_FLAT];
    Float_t  trackEta[MAXBMESON_FLAT][MAXTRACK_FLAT];
    Float_t  trackPhi[MAXBMESON_FLAT][MAXTRACK_FLAT];
    Float_t  trackCharge[MAXBMESON_FLAT][MAXTRACK_FLAT];
    Short_t  trackPID[MAXBMESON_FLAT][MAXTRACK_FLAT];  // 211=pion, 321=kaon
    
    // K*0 information (for B0 only)
    Float_t  kstarPt[MAXBMESON_FLAT];
    Float_t  kstarEta[MAXBMESON_FLAT];
    Float_t  kstarPhi[MAXBMESON_FLAT];
    Float_t  kstarMass[MAXBMESON_FLAT];
    
    // Generator information (MC only)
    UShort_t nGenBMeson;
    Short_t  genBMesonType[MAXBMESON_FLAT];
    Float_t  genBMesonPt[MAXBMESON_FLAT];
    Float_t  genBMesonEta[MAXBMESON_FLAT];
    Float_t  genBMesonPhi[MAXBMESON_FLAT];
    Float_t  genBMesonMass[MAXBMESON_FLAT];
    Short_t  genBMesonPdgId[MAXBMESON_FLAT];
};

#endif