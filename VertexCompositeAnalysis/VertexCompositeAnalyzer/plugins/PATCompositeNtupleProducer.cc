// -*- C++ -*-
//
// Package:    VertexCompositeAnalyzer
// Class:      PATCompositeNtupleProducer
//
// Description: Event-based ntuple producer for pat::CompositeCandidate collections
//              One entry per event, vectors for candidate quantities
//              Supports: D0, D04P, DStar, DStar5P, BPlus, BZero, Bc
//              Uses flat vectors (pTD1, pTD2, ...) instead of nested vectors for efficiency
//
// Author: Soohwan Lee
//

#include <memory>
#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <cmath>

#include <TTree.h>
#include <TVector3.h>
#include <TVector2.h>
#include <TMath.h>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/HeavyIonEvent/interface/Centrality.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"

// Max daughters: 5 (for D04P + slow pion = DStar5P)
// Max grand-daughters: 4 (for D04P inside DStar5P)
constexpr int MAXDAU = 5;
constexpr int MAXGDAU = 4;
constexpr float DUMMY = -999.f;

// Helper structure for gen decay tree (defined before class)
struct GenDecay {
  const reco::GenParticle* main;           // D0, D*, B, etc.
  std::vector<const reco::GenParticle*> daughters;  // Direct daughters
  std::vector<const reco::GenParticle*> grandDaughters;  // Grand-daughters (for two-layer)
};

// Candidate type enumeration
enum class CandidateType {
  D0,        // 2-prong: K + pi
  D04P,      // 4-prong: K + 3pi  
  DStar,     // D0(2P) + slow pi
  DStar5P,   // D0(4P) + slow pi
  BPlus,     // J/psi + K
  BZero,     // J/psi + K* (K+pi)
  Bc,        // J/psi + pi
  X3872,     // J/psi + pi+ pi-
  ChiC,      // J/psi + gamma(e+e-)
  Unknown
};

class PATCompositeNtupleProducer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit PATCompositeNtupleProducer(const edm::ParameterSet&);
  ~PATCompositeNtupleProducer() override = default;

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override {}
  
  void initTree();
  void clearVectors();
  void fillCandidate(const pat::CompositeCandidate& cand, const reco::VertexCollection& vertices, int bestPVIdx, const reco::GenParticleCollection* genParticles, const std::vector<GenDecay>& genDecays);
  int findBestPV(const pat::CompositeCandidate& cand, const reco::VertexCollection& vertices);
  CandidateType getCandidateType(const std::string& name);

  // Configuration
  edm::Service<TFileService> fs;
  
  std::string candType_;
  CandidateType candidateType_;
  bool twoLayerDecay_;
  bool doMuon_;
  bool doElectron_;
  ushort nDau_;
  ushort nGDau_;
  
  bool isCentrality_;
  bool saveTree_;
  bool genealogyInfo_;  // Enable gen matching for MC
  
  // Gen matching criteria (configurable)
  double genMatchDRMax_;        // Maximum ΔR for candidate matching (default: 0.1)
  double genMatchMassWindow_;   // Maximum |Δm| for candidate matching, GeV (default: auto by type)
  double genTrackMatchDRMax_;   // Maximum ΔR for track matching (default: 0.03)
  double genTrackMatchPtRatio_; // Maximum pT ratio difference for track matching (default: 0.5)

  // Tokens
  edm::EDGetTokenT<reco::BeamSpot> tok_beamSpot_;
  edm::EDGetTokenT<reco::VertexCollection> tok_vertices_;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> tok_candidates_;
  edm::EDGetTokenT<int> tok_centBinLabel_;
  edm::EDGetTokenT<reco::Centrality> tok_centSrc_;
  edm::EDGetTokenT<reco::GenParticleCollection> tok_genParticles_;

  // Tree
  TTree* tree_;

  // Event info (scalars - one per event)
  uint runNb_;
  uint eventNb_;
  uint lsNb_;
  short nPV_;
  short centrality_;
  int Ntrkoffline_;
  float bestvx_;
  float bestvy_;
  float bestvz_;
  uint candSize_;

  // Candidate info (vectors - one element per candidate)
  std::vector<float> cand_pt_;
  std::vector<float> cand_eta_;
  std::vector<float> cand_phi_;
  std::vector<float> cand_mass_;
  std::vector<float> cand_y_;
  std::vector<int>   cand_pdgId_;
  std::vector<float> cand_mva_;

  // Vertex info
  std::vector<float> vtxChi2_;
  std::vector<float> vtxNdof_;
  std::vector<float> vtxProb_;
  std::vector<float> alpha2D_;
  std::vector<float> alpha3D_;
  std::vector<float> decayLength2D_;
  std::vector<float> decayLength3D_;
  std::vector<float> decayLengthSig2D_;
  std::vector<float> decayLengthSig3D_;
  std::vector<float> dca3D_;
  std::vector<float> dca3DErr_;
  std::vector<float> trackDCA_;
  std::vector<float> trackDCAErr_;

  // DStar specific
  std::vector<float> deltaM_;

  // Daughter info - FLAT vectors (one per daughter index)
  // D1 = 1st daughter, D2 = 2nd, etc. Use DUMMY (-999) if not applicable
  std::array<std::vector<float>, MAXDAU> dau_pt_;
  std::array<std::vector<float>, MAXDAU> dau_eta_;
  std::array<std::vector<float>, MAXDAU> dau_phi_;
  std::array<std::vector<float>, MAXDAU> dau_mass_;
  std::array<std::vector<short>, MAXDAU> dau_charge_;
  std::array<std::vector<float>, MAXDAU> dau_dedx_;
  std::array<std::vector<float>, MAXDAU> dau_dzSig_;
  std::array<std::vector<float>, MAXDAU> dau_dxySig_;
  std::array<std::vector<float>, MAXDAU> dau_nhit_;
  std::array<std::vector<float>, MAXDAU> dau_ptErr_;
  std::array<std::vector<float>, MAXDAU> dau_trkChi2_;
  std::array<std::vector<bool>, MAXDAU>  dau_highPurity_;

  // Muon-specific daughter info
  std::array<std::vector<bool>, MAXDAU>  dau_isGlobal_;
  std::array<std::vector<bool>, MAXDAU>  dau_isPF_;
  std::array<std::vector<bool>, MAXDAU>  dau_isSoft_;
  std::array<std::vector<bool>, MAXDAU>  dau_isTight_;
  std::array<std::vector<bool>, MAXDAU>  dau_isHybrid_;
  std::array<std::vector<short>, MAXDAU> dau_nMuonHit_;
  std::array<std::vector<short>, MAXDAU> dau_nMatchedStation_;
  std::array<std::vector<short>, MAXDAU> dau_nTrackerLayer_;
  std::array<std::vector<short>, MAXDAU> dau_nPixelHit_;

  // Grand-daughter info - FLAT vectors (for two-layer decays like DStar)
  std::array<std::vector<float>, MAXGDAU> gdau_pt_;
  std::array<std::vector<float>, MAXGDAU> gdau_eta_;
  std::array<std::vector<float>, MAXGDAU> gdau_phi_;
  std::array<std::vector<float>, MAXGDAU> gdau_mass_;
  std::array<std::vector<short>, MAXGDAU> gdau_charge_;
  std::array<std::vector<float>, MAXGDAU> gdau_dedx_;
  std::array<std::vector<float>, MAXGDAU> gdau_dzSig_;
  std::array<std::vector<float>, MAXGDAU> gdau_dxySig_;
  std::array<std::vector<float>, MAXGDAU> gdau_nhit_;
  std::array<std::vector<bool>, MAXGDAU>  gdau_highPurity_;

  // Daughter composite info (for DStar, B mesons)
  std::vector<float> dauCand_mass_;
  std::vector<float> dauCand_pt_;
  std::vector<float> dauCand_eta_;
  std::vector<float> dauCand_vtxProb_;
  std::vector<float> dauCand_mva_;
  std::vector<float> dauCand_alpha3D_;
  std::vector<float> dauCand_decayLength3D_;
  std::vector<float> dauCand_decayLengthSig3D_;

  // Best matching PV index for each candidate
  std::vector<int> bestPVIdx_;
  std::vector<float> bestPVDz_;
  std::vector<float> bestPVDzErr_;
  std::vector<float> bestPVDxy_;
  std::vector<float> bestPVDxyErr_;

  // J/psi index for B mesons (reference to hionia tree)
  std::vector<int> jpsiIdx_;

  // ============ Generator-level info (MC only) ============
  // Gen decay tree structure - stores ALL gen particles in the decay chain
  // For D0: genD0, genDau1 (K), genDau2 (π)
  // For D*: genDStar, genSlowPi, genD0, genDau1 (K), genDau2 (π)
  
  // Main gen particle (D0, D*, B, etc.)
  std::vector<bool> genMatched_;      // Boolean flag: true if reco matched to gen
  std::vector<float> genPt_;
  std::vector<float> genEta_;
  std::vector<float> genPhi_;
  std::vector<float> genMass_;
  std::vector<float> genY_;
  std::vector<int> genPdgId_;
  std::vector<int> genStatus_;
  std::vector<float> genVx_;
  std::vector<float> genVy_;
  std::vector<float> genVz_;
  std::vector<float> genCtau_;        // cτ in mm
  std::vector<float> genCtau3D_;      // 3D cτ in mm
  
  // Gen daughters (for all decay types)
  std::array<std::vector<float>, MAXDAU> genDauPt_;
  std::array<std::vector<float>, MAXDAU> genDauEta_;
  std::array<std::vector<float>, MAXDAU> genDauPhi_;
  std::array<std::vector<float>, MAXDAU> genDauMass_;
  std::array<std::vector<int>, MAXDAU> genDauPdgId_;
  std::array<std::vector<int>, MAXDAU> genDauStatus_;
  
  // Gen grand-daughters (for D*, B mesons)
  std::array<std::vector<float>, MAXGDAU> genGDauPt_;
  std::array<std::vector<float>, MAXGDAU> genGDauEta_;
  std::array<std::vector<float>, MAXGDAU> genGDauPhi_;
  std::array<std::vector<float>, MAXGDAU> genGDauMass_;
  std::array<std::vector<int>, MAXGDAU> genGDauPdgId_;
  std::array<std::vector<int>, MAXGDAU> genGDauStatus_;
  
  // D0 swap flag (K↔π swapped)
  std::vector<bool> isSwap_;
  
  // Helper functions for gen matching
  std::vector<GenDecay> findAllGenDecays(const reco::GenParticleCollection& genParticles);
  int findGenMatch(const pat::CompositeCandidate& cand, const std::vector<GenDecay>& genDecays);
  bool isSwapD0(const pat::CompositeCandidate& cand, const GenDecay& genDecay);
  void fillGenDecayTree(const GenDecay& genDecay);
};

PATCompositeNtupleProducer::PATCompositeNtupleProducer(const edm::ParameterSet& iConfig)
  : candType_(iConfig.getParameter<std::string>("candidateType")),
    twoLayerDecay_(iConfig.getUntrackedParameter<bool>("twoLayerDecay", false)),
    doMuon_(iConfig.getUntrackedParameter<bool>("doMuon", false)),
    doElectron_(iConfig.getUntrackedParameter<bool>("doElectron", false)),
    nDau_(iConfig.getUntrackedParameter<unsigned int>("nDaughters", 2)),
    nGDau_(iConfig.getUntrackedParameter<unsigned int>("nGrandDaughters", 0)),
    isCentrality_(iConfig.getUntrackedParameter<bool>("isCentrality", false)),
    saveTree_(iConfig.getUntrackedParameter<bool>("saveTree", true)),
    genealogyInfo_(iConfig.getUntrackedParameter<bool>("genealogyInfo", false)),
    // Gen matching criteria (optimized defaults)
    genMatchDRMax_(iConfig.getUntrackedParameter<double>("genMatchDRMax", 0.1)),
    genMatchMassWindow_(iConfig.getUntrackedParameter<double>("genMatchMassWindow", -1.0)),  // -1 = auto by type
    genTrackMatchDRMax_(iConfig.getUntrackedParameter<double>("genTrackMatchDRMax", 0.03)),
    genTrackMatchPtRatio_(iConfig.getUntrackedParameter<double>("genTrackMatchPtRatio", 0.5))
{
  usesResource("TFileService");
  
  candidateType_ = getCandidateType(candType_);
  
  // Auto-configure based on candidate type
  switch(candidateType_) {
    case CandidateType::D0:
      nDau_ = 2; twoLayerDecay_ = false; nGDau_ = 0;
      break;
    case CandidateType::D04P:
      nDau_ = 4; twoLayerDecay_ = false; nGDau_ = 0;
      break;
    case CandidateType::DStar:
      nDau_ = 2; twoLayerDecay_ = true; nGDau_ = 2;
      break;
    case CandidateType::DStar5P:
      nDau_ = 2; twoLayerDecay_ = true; nGDau_ = 4;
      break;
    case CandidateType::BPlus:
    case CandidateType::Bc:
      nDau_ = 2; twoLayerDecay_ = true; nGDau_ = 2; doMuon_ = true;
      break;
    case CandidateType::X3872:
      // J/psi + pi+ pi-: daughters are onia(composite), piPlus, piMinus
      // Store oniaIdx to reference hionia tree, skip muon branches
      nDau_ = 3; twoLayerDecay_ = false; nGDau_ = 0; doMuon_ = false;
      break;
    case CandidateType::ChiC:
      // J/psi + gamma(e+e-): daughters are onia(composite), photon(composite with e+e-)
      // Store oniaIdx to reference hionia tree, skip muon branches
      nDau_ = 2; twoLayerDecay_ = false; nGDau_ = 0; doMuon_ = false;
      break;
    case CandidateType::BZero:
      nDau_ = 3; twoLayerDecay_ = true; nGDau_ = 2; doMuon_ = true;
      break;
    default:
      break;
  }

  // Tokens
  tok_beamSpot_ = consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotSrc"));
  tok_vertices_ = consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertexSrc"));
  tok_candidates_ = consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("candidateSrc"));
  
  if(isCentrality_) {
    tok_centBinLabel_ = consumes<int>(iConfig.getParameter<edm::InputTag>("centralityBinLabel"));
    tok_centSrc_ = consumes<reco::Centrality>(iConfig.getParameter<edm::InputTag>("centralitySrc"));
  }
  
  // Gen particles token (for MC gen matching)
  if(genealogyInfo_) {
    tok_genParticles_ = consumes<reco::GenParticleCollection>(
        iConfig.getUntrackedParameter<edm::InputTag>("genParticles", edm::InputTag("prunedGenParticles")));
  }
}

CandidateType PATCompositeNtupleProducer::getCandidateType(const std::string& name) {
  if(name == "D0") return CandidateType::D0;
  if(name == "D04P") return CandidateType::D04P;
  if(name == "DStar") return CandidateType::DStar;
  if(name == "DStar5P") return CandidateType::DStar5P;
  if(name == "BPlus") return CandidateType::BPlus;
  if(name == "BZero") return CandidateType::BZero;
  if(name == "Bc") return CandidateType::Bc;
  if(name == "X3872") return CandidateType::X3872;
  if(name == "ChiC") return CandidateType::ChiC;
  return CandidateType::Unknown;
}

void PATCompositeNtupleProducer::beginJob() {
  if(saveTree_) initTree();
}

void PATCompositeNtupleProducer::initTree() {
  tree_ = fs->make<TTree>("candTree", Form("%s candidates", candType_.c_str()));

  // Event info (scalars)
  tree_->Branch("RunNb", &runNb_, "RunNb/i");
  tree_->Branch("LSNb", &lsNb_, "LSNb/i");
  tree_->Branch("EventNb", &eventNb_, "EventNb/i");
  tree_->Branch("nPV", &nPV_, "nPV/S");
  tree_->Branch("bestvtxX", &bestvx_, "bestvtxX/F");
  tree_->Branch("bestvtxY", &bestvy_, "bestvtxY/F");
  tree_->Branch("bestvtxZ", &bestvz_, "bestvtxZ/F");
  tree_->Branch("candSize", &candSize_, "candSize/i");

  if(isCentrality_) {
    tree_->Branch("centrality", &centrality_, "centrality/S");
    tree_->Branch("Ntrkoffline", &Ntrkoffline_, "Ntrkoffline/I");
  }

  // Candidate kinematics (vectors)
  tree_->Branch("pT", &cand_pt_);
  tree_->Branch("eta", &cand_eta_);
  tree_->Branch("phi", &cand_phi_);
  tree_->Branch("mass", &cand_mass_);
  tree_->Branch("y", &cand_y_);
  tree_->Branch("pdgId", &cand_pdgId_);
  tree_->Branch("mva", &cand_mva_);

  // Vertex info (vectors)
  tree_->Branch("VtxChi2", &vtxChi2_);
  tree_->Branch("VtxNdof", &vtxNdof_);
  tree_->Branch("VtxProb", &vtxProb_);
  tree_->Branch("alpha2D", &alpha2D_);
  tree_->Branch("alpha3D", &alpha3D_);
  tree_->Branch("decayLength2D", &decayLength2D_);
  tree_->Branch("decayLength3D", &decayLength3D_);
  tree_->Branch("decayLengthSig2D", &decayLengthSig2D_);
  tree_->Branch("decayLengthSig3D", &decayLengthSig3D_);
  tree_->Branch("dca3D", &dca3D_);
  tree_->Branch("dca3DErr", &dca3DErr_);
  tree_->Branch("trackDCA", &trackDCA_);
  tree_->Branch("trackDCAErr", &trackDCAErr_);

  // DStar specific
  if(candidateType_ == CandidateType::DStar || candidateType_ == CandidateType::DStar5P) {
    tree_->Branch("deltaM", &deltaM_);
  }

  // Daughter info - FLAT branches (pTD1, pTD2, ... up to nDau_)
  const char* dauNames[MAXDAU] = {"D1", "D2", "D3", "D4", "D5"};
  for(int i = 0; i < nDau_; ++i) {
    tree_->Branch(Form("pT%s", dauNames[i]), &dau_pt_[i]);
    tree_->Branch(Form("eta%s", dauNames[i]), &dau_eta_[i]);
    tree_->Branch(Form("phi%s", dauNames[i]), &dau_phi_[i]);
    tree_->Branch(Form("mass%s", dauNames[i]), &dau_mass_[i]);
    tree_->Branch(Form("charge%s", dauNames[i]), &dau_charge_[i]);
    tree_->Branch(Form("dedx%s", dauNames[i]), &dau_dedx_[i]);
    tree_->Branch(Form("dzSig%s", dauNames[i]), &dau_dzSig_[i]);
    tree_->Branch(Form("dxySig%s", dauNames[i]), &dau_dxySig_[i]);
    tree_->Branch(Form("nhit%s", dauNames[i]), &dau_nhit_[i]);
    tree_->Branch(Form("ptErr%s", dauNames[i]), &dau_ptErr_[i]);
    tree_->Branch(Form("trkChi2%s", dauNames[i]), &dau_trkChi2_[i]);
    tree_->Branch(Form("highPurity%s", dauNames[i]), &dau_highPurity_[i]);

    // Muon-specific branches
    if(doMuon_) {
      tree_->Branch(Form("isGlobal%s", dauNames[i]), &dau_isGlobal_[i]);
      tree_->Branch(Form("isPF%s", dauNames[i]), &dau_isPF_[i]);
      tree_->Branch(Form("isSoft%s", dauNames[i]), &dau_isSoft_[i]);
      tree_->Branch(Form("isTight%s", dauNames[i]), &dau_isTight_[i]);
      tree_->Branch(Form("isHybrid%s", dauNames[i]), &dau_isHybrid_[i]);
      tree_->Branch(Form("nMuonHit%s", dauNames[i]), &dau_nMuonHit_[i]);
      tree_->Branch(Form("nMatchedStation%s", dauNames[i]), &dau_nMatchedStation_[i]);
      tree_->Branch(Form("nTrackerLayer%s", dauNames[i]), &dau_nTrackerLayer_[i]);
      tree_->Branch(Form("nPixelHit%s", dauNames[i]), &dau_nPixelHit_[i]);
    }
  }

  // For X3872/ChiC: store oniaIdx to reference hionia tree (no two-layer decay branches needed)
  bool isOniaDaughter = (candidateType_ == CandidateType::X3872 || candidateType_ == CandidateType::ChiC);
  if(isOniaDaughter) {
    tree_->Branch("oniaIdx", &jpsiIdx_);  // Reuse jpsiIdx_ vector for oniaIdx
  }

  // Two-layer decay: daughter composite + grand-daughters
  if(twoLayerDecay_) {
    tree_->Branch("dauCand_mass", &dauCand_mass_);
    tree_->Branch("dauCand_pt", &dauCand_pt_);
    tree_->Branch("dauCand_eta", &dauCand_eta_);
    tree_->Branch("dauCand_vtxProb", &dauCand_vtxProb_);
    tree_->Branch("dauCand_mva", &dauCand_mva_);
    tree_->Branch("dauCand_alpha3D", &dauCand_alpha3D_);
    tree_->Branch("dauCand_decayLength3D", &dauCand_decayLength3D_);
    tree_->Branch("dauCand_decayLengthSig3D", &dauCand_decayLengthSig3D_);

    // For B mesons: store jpsiIdx to reference hionia tree
    bool isBMeson = (candidateType_ == CandidateType::BPlus || candidateType_ == CandidateType::BZero || candidateType_ == CandidateType::Bc);
    if(isBMeson) {
      tree_->Branch("jpsiIdx", &jpsiIdx_);
    }

    // Grand-daughter info: only for D* (B mesons use jpsiIdx)
    if(!isBMeson && nGDau_ > 0) {
      const char* gdauNames[MAXGDAU] = {"GD1", "GD2", "GD3", "GD4"};
      for(int i = 0; i < nGDau_; ++i) {
        tree_->Branch(Form("pT%s", gdauNames[i]), &gdau_pt_[i]);
        tree_->Branch(Form("eta%s", gdauNames[i]), &gdau_eta_[i]);
        tree_->Branch(Form("phi%s", gdauNames[i]), &gdau_phi_[i]);
        tree_->Branch(Form("mass%s", gdauNames[i]), &gdau_mass_[i]);
        tree_->Branch(Form("charge%s", gdauNames[i]), &gdau_charge_[i]);
        tree_->Branch(Form("dedx%s", gdauNames[i]), &gdau_dedx_[i]);
        tree_->Branch(Form("dzSig%s", gdauNames[i]), &gdau_dzSig_[i]);
        tree_->Branch(Form("dxySig%s", gdauNames[i]), &gdau_dxySig_[i]);
        tree_->Branch(Form("nhit%s", gdauNames[i]), &gdau_nhit_[i]);
        tree_->Branch(Form("highPurity%s", gdauNames[i]), &gdau_highPurity_[i]);
      }
    }
  }

  // Best PV info for each candidate
  tree_->Branch("bestPVIdx", &bestPVIdx_);
  tree_->Branch("bestPVDz", &bestPVDz_);
  tree_->Branch("bestPVDzErr", &bestPVDzErr_);
  tree_->Branch("bestPVDxy", &bestPVDxy_);
  tree_->Branch("bestPVDxyErr", &bestPVDxyErr_);

  // Gen matching info (MC only) - full decay tree structure
  if(genealogyInfo_) {
    tree_->Branch("genMatched", &genMatched_);  // Boolean flag: true if matched
    tree_->Branch("genPt", &genPt_);
    tree_->Branch("genEta", &genEta_);
    tree_->Branch("genPhi", &genPhi_);
    tree_->Branch("genMass", &genMass_);
    tree_->Branch("genY", &genY_);
    tree_->Branch("genPdgId", &genPdgId_);
    tree_->Branch("genStatus", &genStatus_);
    tree_->Branch("genVx", &genVx_);
    tree_->Branch("genVy", &genVy_);
    tree_->Branch("genVz", &genVz_);
    tree_->Branch("genCtau", &genCtau_);
    tree_->Branch("genCtau3D", &genCtau3D_);
    
    // Gen daughters (all decay types)
    const char* dauNames[MAXDAU] = {"D1", "D2", "D3", "D4", "D5"};
    for(int i = 0; i < nDau_; ++i) {
      tree_->Branch(Form("gen%sPt", dauNames[i]), &genDauPt_[i]);
      tree_->Branch(Form("gen%sEta", dauNames[i]), &genDauEta_[i]);
      tree_->Branch(Form("gen%sPhi", dauNames[i]), &genDauPhi_[i]);
      tree_->Branch(Form("gen%sMass", dauNames[i]), &genDauMass_[i]);
      tree_->Branch(Form("gen%sPdgId", dauNames[i]), &genDauPdgId_[i]);
      tree_->Branch(Form("gen%sStatus", dauNames[i]), &genDauStatus_[i]);
    }
    
    // Gen grand-daughters (for D*, B mesons)
    if(twoLayerDecay_ && nGDau_ > 0) {
      const char* gdauNames[MAXGDAU] = {"GD1", "GD2", "GD3", "GD4"};
      for(int i = 0; i < nGDau_; ++i) {
        tree_->Branch(Form("gen%sPt", gdauNames[i]), &genGDauPt_[i]);
        tree_->Branch(Form("gen%sEta", gdauNames[i]), &genGDauEta_[i]);
        tree_->Branch(Form("gen%sPhi", gdauNames[i]), &genGDauPhi_[i]);
        tree_->Branch(Form("gen%sMass", gdauNames[i]), &genGDauMass_[i]);
        tree_->Branch(Form("gen%sPdgId", gdauNames[i]), &genGDauPdgId_[i]);
        tree_->Branch(Form("gen%sStatus", gdauNames[i]), &genGDauStatus_[i]);
      }
    }
    
    // D0 swap flag
    if(candidateType_ == CandidateType::D0) {
      tree_->Branch("isSwap", &isSwap_);
    }
  }
}

void PATCompositeNtupleProducer::clearVectors() {
  // Candidate info
  cand_pt_.clear();
  cand_eta_.clear();
  cand_phi_.clear();
  cand_mass_.clear();
  cand_y_.clear();
  cand_pdgId_.clear();
  cand_mva_.clear();

  // Vertex info
  vtxChi2_.clear();
  vtxNdof_.clear();
  vtxProb_.clear();
  alpha2D_.clear();
  alpha3D_.clear();
  decayLength2D_.clear();
  decayLength3D_.clear();
  decayLengthSig2D_.clear();
  decayLengthSig3D_.clear();
  dca3D_.clear();
  dca3DErr_.clear();
  trackDCA_.clear();
  trackDCAErr_.clear();
  deltaM_.clear();

  // Daughter info - clear all arrays
  for(int i = 0; i < MAXDAU; ++i) {
    dau_pt_[i].clear();
    dau_eta_[i].clear();
    dau_phi_[i].clear();
    dau_mass_[i].clear();
    dau_charge_[i].clear();
    dau_dedx_[i].clear();
    dau_dzSig_[i].clear();
    dau_dxySig_[i].clear();
    dau_nhit_[i].clear();
    dau_ptErr_[i].clear();
    dau_trkChi2_[i].clear();
    dau_highPurity_[i].clear();
    dau_isGlobal_[i].clear();
    dau_isPF_[i].clear();
    dau_isSoft_[i].clear();
    dau_isTight_[i].clear();
    dau_isHybrid_[i].clear();
    dau_nMuonHit_[i].clear();
    dau_nMatchedStation_[i].clear();
    dau_nTrackerLayer_[i].clear();
    dau_nPixelHit_[i].clear();
  }

  // Grand-daughter info
  for(int i = 0; i < MAXGDAU; ++i) {
    gdau_pt_[i].clear();
    gdau_eta_[i].clear();
    gdau_phi_[i].clear();
    gdau_mass_[i].clear();
    gdau_charge_[i].clear();
    gdau_dedx_[i].clear();
    gdau_dzSig_[i].clear();
    gdau_dxySig_[i].clear();
    gdau_nhit_[i].clear();
    gdau_highPurity_[i].clear();
  }

  // Daughter composite
  dauCand_mass_.clear();
  dauCand_pt_.clear();
  dauCand_eta_.clear();
  dauCand_vtxProb_.clear();
  dauCand_mva_.clear();
  dauCand_alpha3D_.clear();
  dauCand_decayLength3D_.clear();
  dauCand_decayLengthSig3D_.clear();

  // Best PV
  bestPVIdx_.clear();
  bestPVDz_.clear();
  bestPVDzErr_.clear();
  bestPVDxy_.clear();
  bestPVDxyErr_.clear();

  // J/psi index for B mesons
  jpsiIdx_.clear();

  // Gen matching info (MC only)
  if(genealogyInfo_) {
    genMatched_.clear();
    genPt_.clear();
    genEta_.clear();
    genPhi_.clear();
    genMass_.clear();
    genY_.clear();
    genPdgId_.clear();
    genStatus_.clear();
    genVx_.clear();
    genVy_.clear();
    genVz_.clear();
    genCtau_.clear();
    genCtau3D_.clear();
    isSwap_.clear();
    
    // Clear gen daughters
    for(int i = 0; i < MAXDAU; ++i) {
      genDauPt_[i].clear();
      genDauEta_[i].clear();
      genDauPhi_[i].clear();
      genDauMass_[i].clear();
      genDauPdgId_[i].clear();
      genDauStatus_[i].clear();
    }
    
    // Clear gen grand-daughters
    for(int i = 0; i < MAXGDAU; ++i) {
      genGDauPt_[i].clear();
      genGDauEta_[i].clear();
      genGDauPhi_[i].clear();
      genGDauMass_[i].clear();
      genGDauPdgId_[i].clear();
      genGDauStatus_[i].clear();
    }
  }
}

void PATCompositeNtupleProducer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  clearVectors();
  
  // Get collections
  edm::Handle<reco::BeamSpot> beamSpot;
  iEvent.getByToken(tok_beamSpot_, beamSpot);
  
  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(tok_vertices_, vertices);
  if(!vertices.isValid() || vertices->empty()) return;

  edm::Handle<pat::CompositeCandidateCollection> candidates;
  iEvent.getByToken(tok_candidates_, candidates);

  // Event info
  runNb_ = iEvent.id().run();
  eventNb_ = iEvent.id().event();
  lsNb_ = iEvent.luminosityBlock();
  nPV_ = vertices->size();

  // Primary vertex
  const auto& pv = vertices->front();
  const bool isPV = (!pv.isFake() && pv.tracksSize() >= 2);
  const auto bs = reco::Vertex(beamSpot->position(), beamSpot->covariance3D());
  const reco::Vertex& vtx = isPV ? pv : bs;
  bestvx_ = vtx.x();
  bestvy_ = vtx.y();
  bestvz_ = vtx.z();

  // Centrality
  centrality_ = -1;
  Ntrkoffline_ = -1;
  if(isCentrality_) {
    edm::Handle<int> centBin;
    iEvent.getByToken(tok_centBinLabel_, centBin);
    if(centBin.isValid()) centrality_ = *centBin;
    
    edm::Handle<reco::Centrality> cent;
    iEvent.getByToken(tok_centSrc_, cent);
    if(cent.isValid()) Ntrkoffline_ = cent->Ntracks();
  }

  // Get gen particles and find all gen decays (MC only)
  std::vector<GenDecay> genDecays;
  const reco::GenParticleCollection* genParticles = nullptr;
  if(genealogyInfo_) {
    edm::Handle<reco::GenParticleCollection> genHandle;
    iEvent.getByToken(tok_genParticles_, genHandle);
    if(genHandle.isValid()) {
      genParticles = genHandle.product();
      genDecays = findAllGenDecays(*genParticles);
    }
  }

  // Fill candidates
  candSize_ = 0;
  if(candidates.isValid()) {
    candSize_ = candidates->size();
    for(const auto& cand : *candidates) {
      int bestIdx = findBestPV(cand, *vertices);
      fillCandidate(cand, *vertices, bestIdx, genParticles, genDecays);
    }
  }

  if(saveTree_) tree_->Fill();
}

int PATCompositeNtupleProducer::findBestPV(const pat::CompositeCandidate& cand, const reco::VertexCollection& vertices) {
  float candVtxZ = 0;
  if(cand.hasUserData("Vtx")) {
    const reco::Vertex* decayVtx = cand.userData<reco::Vertex>("Vtx");
    if(decayVtx) candVtxZ = decayVtx->z();
  } else {
    candVtxZ = cand.vz();
  }
  
  int bestIdx = -1;
  float minDz = 999.f;
  int goodVtxIdx = 0;
  
  for(size_t iv = 0; iv < vertices.size(); ++iv) {
    const reco::Vertex& vtx = vertices[iv];
    if(vtx.isFake() || !vtx.isValid()) continue;
    
    float dz = std::abs(candVtxZ - vtx.z());
    if(dz < minDz) {
      minDz = dz;
      bestIdx = goodVtxIdx;
    }
    goodVtxIdx++;
  }
  
  return bestIdx;
}

void PATCompositeNtupleProducer::fillCandidate(const pat::CompositeCandidate& cand, const reco::VertexCollection& vertices, int bestPVIdx, const reco::GenParticleCollection* genParticles, const std::vector<GenDecay>& genDecays) {
  // Get best PV
  const reco::Vertex* bestPV = nullptr;
  int goodVtxIdx = 0;
  for(const auto& vtx : vertices) {
    if(vtx.isFake() || !vtx.isValid()) continue;
    if(goodVtxIdx == bestPVIdx) {
      bestPV = &vtx;
      break;
    }
    goodVtxIdx++;
  }
  if(!bestPV && !vertices.empty()) bestPV = &vertices.front();
  
  const reco::Vertex& pv = *bestPV;
  const math::XYZPoint pvPos(pv.x(), pv.y(), pv.z());
  
  // Candidate kinematics
  cand_pt_.push_back(cand.pt());
  cand_eta_.push_back(cand.eta());
  cand_phi_.push_back(cand.phi());
  cand_mass_.push_back(cand.mass());
  cand_y_.push_back(cand.rapidity());
  cand_pdgId_.push_back(cand.pdgId());
  cand_mva_.push_back(cand.hasUserFloat("mva") ? cand.userFloat("mva") : DUMMY);

  // Vertex info
  float chi2 = cand.hasUserFloat("VtxChi2") ? cand.userFloat("VtxChi2") : DUMMY;
  float ndof = cand.hasUserFloat("VtxNdof") ? cand.userFloat("VtxNdof") : DUMMY;
  vtxChi2_.push_back(chi2);
  vtxNdof_.push_back(ndof);
  vtxProb_.push_back((chi2 > 0 && ndof > 0) ? TMath::Prob(chi2, ndof) : DUMMY);
  
  alpha2D_.push_back(cand.hasUserFloat("alpha2D") ? cand.userFloat("alpha2D") : DUMMY);
  alpha3D_.push_back(cand.hasUserFloat("alpha3D") ? cand.userFloat("alpha3D") : DUMMY);
  decayLength2D_.push_back(cand.hasUserFloat("decaylength2D") ? cand.userFloat("decaylength2D") : DUMMY);
  decayLength3D_.push_back(cand.hasUserFloat("decaylength3D") ? cand.userFloat("decaylength3D") : DUMMY);
  decayLengthSig2D_.push_back(cand.hasUserFloat("decaylengthsignif2D") ? cand.userFloat("decaylengthsignif2D") : DUMMY);
  decayLengthSig3D_.push_back(cand.hasUserFloat("decaylengthsignif3D") ? cand.userFloat("decaylengthsignif3D") : DUMMY);
  dca3D_.push_back(cand.hasUserFloat("dca3D") ? cand.userFloat("dca3D") : DUMMY);
  dca3DErr_.push_back(cand.hasUserFloat("dca3DErr") ? cand.userFloat("dca3DErr") : DUMMY);
  trackDCA_.push_back(cand.hasUserFloat("track3DDCA") ? cand.userFloat("track3DDCA") : DUMMY);
  trackDCAErr_.push_back(cand.hasUserFloat("track3DDCAErr") ? cand.userFloat("track3DDCAErr") : DUMMY);

  // DStar specific
  if(candidateType_ == CandidateType::DStar || candidateType_ == CandidateType::DStar5P) {
    deltaM_.push_back(cand.hasUserFloat("deltaM") ? cand.userFloat("deltaM") : DUMMY);
  }

  // Get dEdx from userFloat
  float posDauDeDx = cand.hasUserFloat("posDauDeDx") ? cand.userFloat("posDauDeDx") : DUMMY;
  float negDauDeDx = cand.hasUserFloat("negDauDeDx") ? cand.userFloat("negDauDeDx") : DUMMY;
  float slowPionDeDx = cand.hasUserFloat("slowPionDeDx") ? cand.userFloat("slowPionDeDx") : DUMMY;
  

  // Fill daughter info for each daughter index
  const ushort nDaughters = std::min(static_cast<ushort>(cand.numberOfDaughters()), nDau_);
  
  for(int iDau = 0; iDau < nDau_; ++iDau) {
    if(iDau < nDaughters) {
      const auto* dau = cand.daughter(iDau);
      if(dau) {
        dau_pt_[iDau].push_back(dau->pt());
        dau_eta_[iDau].push_back(dau->eta());
        dau_phi_[iDau].push_back(dau->phi());
        dau_mass_[iDau].push_back(dau->mass());
        dau_charge_[iDau].push_back(dau->charge());

        // Assign dEdx
        float dedx = DUMMY;
        if(candidateType_ == CandidateType::DStar || candidateType_ == CandidateType::DStar5P) {
          dedx = (iDau == 1) ? slowPionDeDx : DUMMY;
        } else if(candidateType_ == CandidateType::X3872) {
          // X3872: D0=onia(composite), D1=piPlus, D2=piMinus
          if(iDau == 1) dedx = posDauDeDx;       // piPlus
          else if(iDau == 2) dedx = negDauDeDx;  // piMinus
          // iDau==0 (onia) has no dEdx
        } else if(candidateType_ == CandidateType::ChiC) {
          // ChiC: D0=onia(composite), D1=photon(composite with e+e-)
          // dEdx for electrons stored separately - leave as DUMMY for composite daughter
        } else {
          dedx = (iDau == 0) ? posDauDeDx : negDauDeDx;
        }
        dau_dedx_[iDau].push_back(dedx);

        // Track info
        const reco::Track* trk = dau->bestTrack();
        if(trk) {
          const double dz = trk->dz(pvPos);
          const double dxy = trk->dxy(pvPos);
          const double dzErr = std::sqrt(trk->dzError()*trk->dzError() + pv.zError()*pv.zError());
          const double dxyErr = std::sqrt(trk->d0Error()*trk->d0Error() + pv.xError()*pv.yError());
          
          dau_dzSig_[iDau].push_back(dz/dzErr);
          dau_dxySig_[iDau].push_back(dxy/dxyErr);
          dau_nhit_[iDau].push_back(trk->numberOfValidHits());
          dau_ptErr_[iDau].push_back(trk->ptError());
          dau_trkChi2_[iDau].push_back(trk->normalizedChi2());
          dau_highPurity_[iDau].push_back(trk->quality(reco::TrackBase::highPurity));
        } else {
          dau_dzSig_[iDau].push_back(DUMMY);
          dau_dxySig_[iDau].push_back(DUMMY);
          dau_nhit_[iDau].push_back(-1);
          dau_ptErr_[iDau].push_back(DUMMY);
          dau_trkChi2_[iDau].push_back(DUMMY);
          dau_highPurity_[iDau].push_back(false);
        }

        // Muon info
        if(doMuon_) {
          bool isGlobal = false, isPF = false, isSoft = false, isTight = false, isHybrid = false;
          short nMuonHit = -1, nMatchedStation = -1, nTrackerLayer = -1, nPixelHit = -1;
          
          if(dau->isMuon()) {
            const auto* muon = dynamic_cast<const pat::Muon*>(dau);
            if(muon) {
              isGlobal = muon->isGlobalMuon();
              isPF = muon->isPFMuon();
              isSoft = muon->isSoftMuon(pv);
              isTight = muon->isTightMuon(pv);
              
              if(muon->globalTrack().isNonnull()) {
                nMuonHit = muon->globalTrack()->hitPattern().numberOfValidMuonHits();
              }
              nMatchedStation = muon->numberOfMatchedStations();
              if(muon->innerTrack().isNonnull()) {
                nTrackerLayer = muon->innerTrack()->hitPattern().trackerLayersWithMeasurement();
                nPixelHit = muon->innerTrack()->hitPattern().numberOfValidPixelHits();
              }
              isHybrid = isGlobal && nTrackerLayer > 5 && nPixelHit > 0;
            }
          }
          dau_isGlobal_[iDau].push_back(isGlobal);
          dau_isPF_[iDau].push_back(isPF);
          dau_isSoft_[iDau].push_back(isSoft);
          dau_isTight_[iDau].push_back(isTight);
          dau_isHybrid_[iDau].push_back(isHybrid);
          dau_nMuonHit_[iDau].push_back(nMuonHit);
          dau_nMatchedStation_[iDau].push_back(nMatchedStation);
          dau_nTrackerLayer_[iDau].push_back(nTrackerLayer);
          dau_nPixelHit_[iDau].push_back(nPixelHit);
        }
      } else {
        // Null daughter - fill dummy
        dau_pt_[iDau].push_back(DUMMY);
        dau_eta_[iDau].push_back(DUMMY);
        dau_phi_[iDau].push_back(DUMMY);
        dau_mass_[iDau].push_back(DUMMY);
        dau_charge_[iDau].push_back(0);
        dau_dedx_[iDau].push_back(DUMMY);
        dau_dzSig_[iDau].push_back(DUMMY);
        dau_dxySig_[iDau].push_back(DUMMY);
        dau_nhit_[iDau].push_back(-1);
        dau_ptErr_[iDau].push_back(DUMMY);
        dau_trkChi2_[iDau].push_back(DUMMY);
        dau_highPurity_[iDau].push_back(false);
        if(doMuon_) {
          dau_isGlobal_[iDau].push_back(false);
          dau_isPF_[iDau].push_back(false);
          dau_isSoft_[iDau].push_back(false);
          dau_isTight_[iDau].push_back(false);
          dau_isHybrid_[iDau].push_back(false);
          dau_nMuonHit_[iDau].push_back(-1);
          dau_nMatchedStation_[iDau].push_back(-1);
          dau_nTrackerLayer_[iDau].push_back(-1);
          dau_nPixelHit_[iDau].push_back(-1);
        }
      }
    }
  }

  // Two-layer decay: daughter composite + grand-daughters
  if(twoLayerDecay_) {
    float dcMass = DUMMY, dcPt = DUMMY, dcEta = DUMMY, dcVtxProb = DUMMY, dcMva = DUMMY;
    float dcAlpha3D = DUMMY, dcDecLen3D = DUMMY, dcDecLenSig3D = DUMMY;

    // Find composite daughter
    const pat::CompositeCandidate* dauCand = nullptr;
    for(ushort iDau = 0; iDau < cand.numberOfDaughters(); ++iDau) {
      const auto* dau = cand.daughter(iDau);
      if(!dau || dau->numberOfDaughters() == 0) continue;
      dauCand = dynamic_cast<const pat::CompositeCandidate*>(dau);
      if(dauCand) break;
    }

    if(dauCand) {
      dcMass = dauCand->mass();
      dcPt = dauCand->pt();
      dcEta = dauCand->eta();
      dcMva = cand.hasUserFloat("D0mva") ? cand.userFloat("D0mva") : DUMMY;
      
      float d0Chi2 = dauCand->hasUserFloat("VtxChi2") ? dauCand->userFloat("VtxChi2") : DUMMY;
      float d0Ndof = dauCand->hasUserFloat("VtxNdof") ? dauCand->userFloat("VtxNdof") : DUMMY;
      dcVtxProb = (d0Chi2 > 0 && d0Ndof > 0) ? TMath::Prob(d0Chi2, d0Ndof) : DUMMY;
      dcAlpha3D = dauCand->hasUserFloat("alpha3D") ? dauCand->userFloat("alpha3D") : DUMMY;
      dcDecLen3D = dauCand->hasUserFloat("decaylength3D") ? dauCand->userFloat("decaylength3D") : DUMMY;
      dcDecLenSig3D = dauCand->hasUserFloat("decaylengthsignif3D") ? dauCand->userFloat("decaylengthsignif3D") : DUMMY;

      // Grand-daughters (only for D*, not B mesons)
      bool isBMeson = (candidateType_ == CandidateType::BPlus || candidateType_ == CandidateType::BZero || candidateType_ == CandidateType::Bc);
      if(!isBMeson && nGDau_ > 0) {
        const ushort nGDaughters = std::min(static_cast<ushort>(dauCand->numberOfDaughters()), nGDau_);
        
        for(int iGDau = 0; iGDau < nGDau_; ++iGDau) {
          if(iGDau < nGDaughters) {
            const auto* gdau = dauCand->daughter(iGDau);
            if(gdau) {
              gdau_pt_[iGDau].push_back(gdau->pt());
              gdau_eta_[iGDau].push_back(gdau->eta());
              gdau_phi_[iGDau].push_back(gdau->phi());
              gdau_mass_[iGDau].push_back(gdau->mass());
              gdau_charge_[iGDau].push_back(gdau->charge());

              float gded = DUMMY;
              if(iGDau == 0) gded = cand.hasUserFloat("D0posDauDeDx") ? cand.userFloat("D0posDauDeDx") : DUMMY;
              else if(iGDau == 1) gded = cand.hasUserFloat("D0negDauDeDx") ? cand.userFloat("D0negDauDeDx") : DUMMY;
              gdau_dedx_[iGDau].push_back(gded);

              const reco::Track* gtrk = gdau->bestTrack();
              if(gtrk) {
                const double gdz = gtrk->dz(pvPos);
                const double gdxy = gtrk->dxy(pvPos);
                const double gdzErr = std::sqrt(gtrk->dzError()*gtrk->dzError() + pv.zError()*pv.zError());
                const double gdxyErr = std::sqrt(gtrk->d0Error()*gtrk->d0Error() + pv.xError()*pv.yError());
                
                gdau_dzSig_[iGDau].push_back(gdz/gdzErr);
                gdau_dxySig_[iGDau].push_back(gdxy/gdxyErr);
                gdau_nhit_[iGDau].push_back(gtrk->numberOfValidHits());
                gdau_highPurity_[iGDau].push_back(gtrk->quality(reco::TrackBase::highPurity));
              } else {
                gdau_dzSig_[iGDau].push_back(DUMMY);
                gdau_dxySig_[iGDau].push_back(DUMMY);
                gdau_nhit_[iGDau].push_back(-1);
                gdau_highPurity_[iGDau].push_back(false);
              }
            } else {
              // Null grand-daughter
              gdau_pt_[iGDau].push_back(DUMMY);
              gdau_eta_[iGDau].push_back(DUMMY);
              gdau_phi_[iGDau].push_back(DUMMY);
              gdau_mass_[iGDau].push_back(DUMMY);
              gdau_charge_[iGDau].push_back(0);
              gdau_dedx_[iGDau].push_back(DUMMY);
              gdau_dzSig_[iGDau].push_back(DUMMY);
              gdau_dxySig_[iGDau].push_back(DUMMY);
              gdau_nhit_[iGDau].push_back(-1);
              gdau_highPurity_[iGDau].push_back(false);
            }
          }
        }
      }
    } else {
      // No composite daughter found - fill grand-daughter dummies
      bool isBMeson = (candidateType_ == CandidateType::BPlus || candidateType_ == CandidateType::BZero || candidateType_ == CandidateType::Bc);
      if(!isBMeson && nGDau_ > 0) {
        for(int iGDau = 0; iGDau < nGDau_; ++iGDau) {
          gdau_pt_[iGDau].push_back(DUMMY);
          gdau_eta_[iGDau].push_back(DUMMY);
          gdau_phi_[iGDau].push_back(DUMMY);
          gdau_mass_[iGDau].push_back(DUMMY);
          gdau_charge_[iGDau].push_back(0);
          gdau_dedx_[iGDau].push_back(DUMMY);
          gdau_dzSig_[iGDau].push_back(DUMMY);
          gdau_dxySig_[iGDau].push_back(DUMMY);
          gdau_nhit_[iGDau].push_back(-1);
          gdau_highPurity_[iGDau].push_back(false);
        }
      }
    }

    dauCand_mass_.push_back(dcMass);
    dauCand_pt_.push_back(dcPt);
    dauCand_eta_.push_back(dcEta);
    dauCand_vtxProb_.push_back(dcVtxProb);
    dauCand_mva_.push_back(dcMva);
    dauCand_alpha3D_.push_back(dcAlpha3D);
    dauCand_decayLength3D_.push_back(dcDecLen3D);
    dauCand_decayLengthSig3D_.push_back(dcDecLenSig3D);

    // For B mesons: store jpsiIdx
    bool isBMeson = (candidateType_ == CandidateType::BPlus || candidateType_ == CandidateType::BZero || candidateType_ == CandidateType::Bc);
    if(isBMeson) {
      int jpsiIdx = cand.hasUserInt("jpsiIdx") ? cand.userInt("jpsiIdx") : -1;
      jpsiIdx_.push_back(jpsiIdx);
    }
  }

  // For X3872/ChiC: store oniaIdx
  bool isOniaDaughter = (candidateType_ == CandidateType::X3872 || candidateType_ == CandidateType::ChiC);
  if(isOniaDaughter) {
    int oniaIdx = cand.hasUserInt("oniaIdx") ? cand.userInt("oniaIdx") : -1;
    jpsiIdx_.push_back(oniaIdx);  // Reuse jpsiIdx_ vector
  }

  // Gen matching (MC only) - match to full decay tree
  if(genealogyInfo_ && !genDecays.empty()) {
    int matchedDecayIdx = findGenMatch(cand, genDecays);
    
    if(matchedDecayIdx >= 0 && matchedDecayIdx < (int)genDecays.size()) {
      const GenDecay& matchedDecay = genDecays[matchedDecayIdx];
      genMatched_.push_back(true);
      
      // Fill full gen decay tree (main particle + all daughters + grand-daughters)
      fillGenDecayTree(matchedDecay);
      
      // D0 swap detection
      if(candidateType_ == CandidateType::D0) {
        bool swap = isSwapD0(cand, matchedDecay);
        isSwap_.push_back(swap);
      } else {
        // For non-D0, push false (or skip if not D0)
      }
    } else {
      // No match found - fill with dummy values
      genMatched_.push_back(false);
      genPt_.push_back(DUMMY);
      genEta_.push_back(DUMMY);
      genPhi_.push_back(DUMMY);
      genMass_.push_back(DUMMY);
      genY_.push_back(DUMMY);
      genPdgId_.push_back(0);
      genStatus_.push_back(-1);
      genVx_.push_back(DUMMY);
      genVy_.push_back(DUMMY);
      genVz_.push_back(DUMMY);
      genCtau_.push_back(DUMMY);
      genCtau3D_.push_back(DUMMY);
      
      // Fill dummy daughters
      for(int i = 0; i < nDau_; ++i) {
        genDauPt_[i].push_back(DUMMY);
        genDauEta_[i].push_back(DUMMY);
        genDauPhi_[i].push_back(DUMMY);
        genDauMass_[i].push_back(DUMMY);
        genDauPdgId_[i].push_back(0);
        genDauStatus_[i].push_back(-1);
      }
      
      // Fill dummy grand-daughters
      if(twoLayerDecay_ && nGDau_ > 0) {
        for(int i = 0; i < nGDau_; ++i) {
          genGDauPt_[i].push_back(DUMMY);
          genGDauEta_[i].push_back(DUMMY);
          genGDauPhi_[i].push_back(DUMMY);
          genGDauMass_[i].push_back(DUMMY);
          genGDauPdgId_[i].push_back(0);
          genGDauStatus_[i].push_back(-1);
        }
      }
      
      if(candidateType_ == CandidateType::D0) {
        isSwap_.push_back(false);
      }
    }
  }

  // Best PV info
  bestPVIdx_.push_back(bestPVIdx);
  
  float candVtxZ = cand.vz();
  float candVtxX = cand.vx();
  float candVtxY = cand.vy();
  if(cand.hasUserData("Vtx")) {
    const reco::Vertex* decayVtx = cand.userData<reco::Vertex>("Vtx");
    if(decayVtx) {
      candVtxX = decayVtx->x();
      candVtxY = decayVtx->y();
      candVtxZ = decayVtx->z();
    }
  }
  
  float dz = candVtxZ - pv.z();
  float dx = candVtxX - pv.x();
  float dy = candVtxY - pv.y();
  float dxy = std::sqrt(dx*dx + dy*dy);
  float dzErr = pv.zError();
  float dxyErr = std::sqrt(pv.xError()*pv.xError() + pv.yError()*pv.yError());
  
  bestPVDz_.push_back(dz);
  bestPVDzErr_.push_back(dzErr);
  bestPVDxy_.push_back(dxy);
  bestPVDxyErr_.push_back(dxyErr);
}

// Gen matching helper functions - find all gen decays matching candidate type
std::vector<GenDecay> PATCompositeNtupleProducer::findAllGenDecays(const reco::GenParticleCollection& genParticles) {
  std::vector<GenDecay> genDecays;
  
  // NOTE: prunedGenParticles has a pT cut (typically ~0.5-1 GeV), so low pT daughters
  // might not be in the collection. However, gen.daughter(d) still returns valid
  // pointers to daughter particles even if they're not in prunedGenParticles.
  // We can still access their kinematics (pt, eta, phi, pdgId) for matching purposes.
  // For complete decay trees without pT cuts, consider using 'genParticles' instead.
  
  // Expected PDG IDs for main particle
  std::vector<int> expectedPdgIds;
  switch(candidateType_) {
    case CandidateType::D0:
      expectedPdgIds = {421, -421};  // D0 and anti-D0
      break;
    case CandidateType::D04P:
      expectedPdgIds = {421, -421};
      break;
    case CandidateType::DStar:
      expectedPdgIds = {413, -413};  // D*+ and D*-
      break;
    case CandidateType::DStar5P:
      expectedPdgIds = {413, -413};
      break;
    case CandidateType::BPlus:
      expectedPdgIds = {521, -521};  // B+ and B-
      break;
    case CandidateType::BZero:
      expectedPdgIds = {511, -511};  // B0 and anti-B0
      break;
    case CandidateType::Bc:
      expectedPdgIds = {541, -541};  // Bc+ and Bc-
      break;
    case CandidateType::X3872:
      expectedPdgIds = {9920443};  // X(3872)
      break;
    case CandidateType::ChiC:
      expectedPdgIds = {10441, 20443, 445};  // chi_c0, chi_c1, chi_c2
      break;
    default:
      return genDecays;
  }
  
  // Loop over all gen particles to find decays
  for(size_t i = 0; i < genParticles.size(); ++i) {
    const auto& gen = genParticles[i];
    int absPdgId = std::abs(gen.pdgId());
    
    // Check if PDG ID matches
    bool pdgMatch = false;
    for(int expPdg : expectedPdgIds) {
      if(std::abs(expPdg) == absPdgId) {
        pdgMatch = true;
        break;
      }
    }
    if(!pdgMatch) continue;
    
    // Check status (2 = decayed, 1 = stable)
    if(gen.status() != 2 && gen.status() != 1) continue;
    
    GenDecay decay;
    decay.main = &gen;
    
    // Extract daughters based on decay type
    // Note: gen.daughter(d) returns valid pointers even if daughter is not in prunedGenParticles
    // (due to pT cuts). We can still access kinematics for matching.
    if(candidateType_ == CandidateType::D0 || candidateType_ == CandidateType::D04P) {
      // D0 -> K π (or K 3π for D04P)
      for(size_t d = 0; d < gen.numberOfDaughters(); ++d) {
        const reco::GenParticle* dau = dynamic_cast<const reco::GenParticle*>(gen.daughter(d));
        if(dau) {
          // Store daughter pointer (valid even if not in prunedGenParticles collection)
          decay.daughters.push_back(dau);
        }
      }
    } else if(candidateType_ == CandidateType::DStar || candidateType_ == CandidateType::DStar5P) {
      // D* -> D0 π
      const reco::GenParticle* genD0 = nullptr;
      const reco::GenParticle* genSlowPi = nullptr;
      
      for(size_t d = 0; d < gen.numberOfDaughters(); ++d) {
        const reco::GenParticle* dau = dynamic_cast<const reco::GenParticle*>(gen.daughter(d));
        if(!dau) continue;
        int dauPdgId = std::abs(dau->pdgId());
        if(dauPdgId == 421) {
          genD0 = dau;
        } else if(dauPdgId == 211) {
          genSlowPi = dau;
        }
      }
      
      if(genD0 && genSlowPi) {
        decay.daughters.push_back(genD0);      // D0 daughter
        decay.daughters.push_back(genSlowPi);  // Slow π daughter
        
        // Extract D0 daughters (grand-daughters: K and π)
        // Note: genD0->daughter(d) returns valid pointers even if not in prunedGenParticles
        for(size_t d = 0; d < genD0->numberOfDaughters(); ++d) {
          const reco::GenParticle* gdau = dynamic_cast<const reco::GenParticle*>(genD0->daughter(d));
          if(gdau) {
            decay.grandDaughters.push_back(gdau);
          }
        }
      }
    } else if(candidateType_ == CandidateType::BPlus || candidateType_ == CandidateType::BZero || candidateType_ == CandidateType::Bc) {
      // B -> J/ψ + hadron
      const reco::GenParticle* genJpsi = nullptr;
      const reco::GenParticle* genHadron = nullptr;
      
      for(size_t d = 0; d < gen.numberOfDaughters(); ++d) {
        const reco::GenParticle* dau = dynamic_cast<const reco::GenParticle*>(gen.daughter(d));
        if(!dau) continue;
        int dauPdgId = std::abs(dau->pdgId());
        if(dauPdgId == 443) {
          genJpsi = dau;
        } else {
          genHadron = dau;
        }
      }
      
      if(genJpsi) {
        decay.daughters.push_back(genJpsi);
        if(genHadron) decay.daughters.push_back(genHadron);
        
        // Extract J/ψ daughters (grand-daughters: μ+ μ-)
        // Note: genJpsi->daughter(d) returns valid pointers even if not in prunedGenParticles
        for(size_t d = 0; d < genJpsi->numberOfDaughters(); ++d) {
          const reco::GenParticle* gdau = dynamic_cast<const reco::GenParticle*>(genJpsi->daughter(d));
          if(gdau && std::abs(gdau->pdgId()) == 13) {  // Muon
            decay.grandDaughters.push_back(gdau);
          }
        }
      }
    } else if(candidateType_ == CandidateType::X3872) {
      // X(3872) -> J/ψ π+ π-
      const reco::GenParticle* genJpsi = nullptr;
      std::vector<const reco::GenParticle*> genPions;
      
      for(size_t d = 0; d < gen.numberOfDaughters(); ++d) {
        const reco::GenParticle* dau = dynamic_cast<const reco::GenParticle*>(gen.daughter(d));
        if(!dau) continue;
        int dauPdgId = std::abs(dau->pdgId());
        if(dauPdgId == 443) {
          genJpsi = dau;
        } else if(dauPdgId == 211) {
          genPions.push_back(dau);
        }
      }
      
      if(genJpsi) decay.daughters.push_back(genJpsi);
      for(auto pi : genPions) decay.daughters.push_back(pi);
    } else if(candidateType_ == CandidateType::ChiC) {
      // Chi_c -> J/ψ γ
      const reco::GenParticle* genJpsi = nullptr;
      const reco::GenParticle* genGamma = nullptr;
      
      for(size_t d = 0; d < gen.numberOfDaughters(); ++d) {
        const reco::GenParticle* dau = dynamic_cast<const reco::GenParticle*>(gen.daughter(d));
        if(!dau) continue;
        int dauPdgId = std::abs(dau->pdgId());
        if(dauPdgId == 443) {
          genJpsi = dau;
        } else if(dauPdgId == 22) {
          genGamma = dau;
        }
      }
      
      if(genJpsi) decay.daughters.push_back(genJpsi);
      if(genGamma) decay.daughters.push_back(genGamma);
    }
    
    // Only add if we found the expected decay structure
    if(decay.main && decay.daughters.size() >= nDau_) {
      genDecays.push_back(decay);
    }
  }
  
  return genDecays;
}

int PATCompositeNtupleProducer::findGenMatch(const pat::CompositeCandidate& cand, const std::vector<GenDecay>& genDecays) {
  double candEta = cand.eta();
  double candPhi = cand.phi();
  
  int bestMatchIdx = -1;
  double bestDR = 999.0;
  
  // Match reco candidate to gen decay by ΔR of main particle
  for(size_t i = 0; i < genDecays.size(); ++i) {
    const GenDecay& decay = genDecays[i];
    if(!decay.main) continue;
    
    // Calculate ΔR
    double dEta = decay.main->eta() - candEta;
    double dPhi = TVector2::Phi_mpi_pi(decay.main->phi() - candPhi);
    double dR = std::sqrt(dEta*dEta + dPhi*dPhi);
    
    // Matching criteria: ΔR < genMatchDRMax_
    if(dR < genMatchDRMax_) {
      // Prefer closer matches (smaller ΔR)
      if(dR < bestDR) {
        bestDR = dR;
        bestMatchIdx = i;
      }
    }
  }
  
  return bestMatchIdx;
}

void PATCompositeNtupleProducer::fillGenDecayTree(const GenDecay& genDecay) {
  if(!genDecay.main) return;
  
  const reco::GenParticle* main = genDecay.main;
  
  // Fill main gen particle
  genPt_.push_back(main->pt());
  genEta_.push_back(main->eta());
  genPhi_.push_back(main->phi());
  genMass_.push_back(main->mass());
  genY_.push_back(main->rapidity());
  genPdgId_.push_back(main->pdgId());
  genStatus_.push_back(main->status());
  genVx_.push_back(main->vx());
  genVy_.push_back(main->vy());
  genVz_.push_back(main->vz());
  
  // Calculate cτ
  float ctau = 0.f;
  float ctau3D = 0.f;
  if(main->numberOfMothers() > 0) {
    const reco::Candidate* mom = main->mother();
    if(mom) {
      TVector3 prodVtx(mom->vx(), mom->vy(), mom->vz());
      TVector3 decayVtx(main->vx(), main->vy(), main->vz());
      TVector3 momentum(main->px(), main->py(), main->pz());
      TVector3 diff = decayVtx - prodVtx;
      
      if(momentum.Perp() > 0)
        ctau = static_cast<float>(diff.Perp() * main->mass() / momentum.Perp());
      if(momentum.Mag() > 0)
        ctau3D = static_cast<float>(diff.Mag() * main->mass() / momentum.Mag());
    }
  }
  genCtau_.push_back(ctau * 10.0f);  // Convert to mm
  genCtau3D_.push_back(ctau3D * 10.0f);
  
  // Fill gen daughters
  for(int i = 0; i < nDau_; ++i) {
    if(i < (int)genDecay.daughters.size() && genDecay.daughters[i]) {
      const reco::GenParticle* dau = genDecay.daughters[i];
      genDauPt_[i].push_back(dau->pt());
      genDauEta_[i].push_back(dau->eta());
      genDauPhi_[i].push_back(dau->phi());
      genDauMass_[i].push_back(dau->mass());
      genDauPdgId_[i].push_back(dau->pdgId());
      genDauStatus_[i].push_back(dau->status());
    } else {
      genDauPt_[i].push_back(DUMMY);
      genDauEta_[i].push_back(DUMMY);
      genDauPhi_[i].push_back(DUMMY);
      genDauMass_[i].push_back(DUMMY);
      genDauPdgId_[i].push_back(0);
      genDauStatus_[i].push_back(-1);
    }
  }
  
  // Fill gen grand-daughters (for D*, B mesons)
  if(twoLayerDecay_ && nGDau_ > 0) {
    for(int i = 0; i < nGDau_; ++i) {
      if(i < (int)genDecay.grandDaughters.size() && genDecay.grandDaughters[i]) {
        const reco::GenParticle* gdau = genDecay.grandDaughters[i];
        genGDauPt_[i].push_back(gdau->pt());
        genGDauEta_[i].push_back(gdau->eta());
        genGDauPhi_[i].push_back(gdau->phi());
        genGDauMass_[i].push_back(gdau->mass());
        genGDauPdgId_[i].push_back(gdau->pdgId());
        genGDauStatus_[i].push_back(gdau->status());
      } else {
        genGDauPt_[i].push_back(DUMMY);
        genGDauEta_[i].push_back(DUMMY);
        genGDauPhi_[i].push_back(DUMMY);
        genGDauMass_[i].push_back(DUMMY);
        genGDauPdgId_[i].push_back(0);
        genGDauStatus_[i].push_back(-1);
      }
    }
  }
}

bool PATCompositeNtupleProducer::isSwapD0(const pat::CompositeCandidate& cand, const GenDecay& genDecay) {
  // Check if K and π are swapped by comparing assigned mass hypothesis to gen PDG ID
  if(!genDecay.main || genDecay.daughters.size() < 2) return false;
  if(cand.numberOfDaughters() < 2) return false;
  
  // Get gen daughter particles (K and π)
  const reco::GenParticle* genDau1 = genDecay.daughters[0];
  const reco::GenParticle* genDau2 = genDecay.daughters[1];
  if(!genDau1 || !genDau2) return false;
  
  int genDau1PdgId = genDau1->pdgId();
  int genDau2PdgId = genDau2->pdgId();
  
  // Get reco daughter tracks
  const reco::Candidate* recoDau1 = cand.daughter(0);
  const reco::Candidate* recoDau2 = cand.daughter(1);
  if(!recoDau1 || !recoDau2) return false;
  
  const reco::Track* trk1 = recoDau1->bestTrack();
  const reco::Track* trk2 = recoDau2->bestTrack();
  if(!trk1 || !trk2) return false;
  
  // Match reco tracks to gen particles by kinematics
  // Calculate ΔR and pT ratio between reco tracks and gen daughters
  double dEta1_1 = trk1->eta() - genDau1->eta();
  double dPhi1_1 = TVector2::Phi_mpi_pi(trk1->phi() - genDau1->phi());
  double dR1_1 = std::sqrt(dEta1_1*dEta1_1 + dPhi1_1*dPhi1_1);
  double ptRatio1_1 = std::abs(genDau1->pt() - trk1->pt()) / trk1->pt();
  
  double dEta1_2 = trk1->eta() - genDau2->eta();
  double dPhi1_2 = TVector2::Phi_mpi_pi(trk1->phi() - genDau2->phi());
  double dR1_2 = std::sqrt(dEta1_2*dEta1_2 + dPhi1_2*dPhi1_2);
  double ptRatio1_2 = std::abs(genDau2->pt() - trk1->pt()) / trk1->pt();
  
  double dEta2_1 = trk2->eta() - genDau1->eta();
  double dPhi2_1 = TVector2::Phi_mpi_pi(trk2->phi() - genDau1->phi());
  double dR2_1 = std::sqrt(dEta2_1*dEta2_1 + dPhi2_1*dPhi2_1);
  double ptRatio2_1 = std::abs(genDau1->pt() - trk2->pt()) / trk2->pt();
  
  double dEta2_2 = trk2->eta() - genDau2->eta();
  double dPhi2_2 = TVector2::Phi_mpi_pi(trk2->phi() - genDau2->phi());
  double dR2_2 = std::sqrt(dEta2_2*dEta2_2 + dPhi2_2*dPhi2_2);
  double ptRatio2_2 = std::abs(genDau2->pt() - trk2->pt()) / trk2->pt();
  
  // Match tracks to gen particles (require charge match, ΔR < threshold, and pT ratio < threshold)
  bool trk1MatchesGen1 = (trk1->charge() == genDau1->charge() && dR1_1 < genTrackMatchDRMax_ && ptRatio1_1 < genTrackMatchPtRatio_);
  bool trk1MatchesGen2 = (trk1->charge() == genDau2->charge() && dR1_2 < genTrackMatchDRMax_ && ptRatio1_2 < genTrackMatchPtRatio_);
  bool trk2MatchesGen1 = (trk2->charge() == genDau1->charge() && dR2_1 < genTrackMatchDRMax_ && ptRatio2_1 < genTrackMatchPtRatio_);
  bool trk2MatchesGen2 = (trk2->charge() == genDau2->charge() && dR2_2 < genTrackMatchDRMax_ && ptRatio2_2 < genTrackMatchPtRatio_);
  
  // Determine which gen particle matches which reco track
  int genPdgIdForTrk1 = 0;
  int genPdgIdForTrk2 = 0;
  
  if(trk1MatchesGen1 && !trk1MatchesGen2) {
    genPdgIdForTrk1 = genDau1PdgId;
  } else if(trk1MatchesGen2 && !trk1MatchesGen1) {
    genPdgIdForTrk1 = genDau2PdgId;
  } else if(trk1MatchesGen1 && trk1MatchesGen2) {
    // Both match, use closer one
    genPdgIdForTrk1 = (dR1_1 < dR1_2) ? genDau1PdgId : genDau2PdgId;
  }
  
  if(trk2MatchesGen1 && !trk2MatchesGen2) {
    genPdgIdForTrk2 = genDau1PdgId;
  } else if(trk2MatchesGen2 && !trk2MatchesGen1) {
    genPdgIdForTrk2 = genDau2PdgId;
  } else if(trk2MatchesGen1 && trk2MatchesGen2) {
    // Both match, use closer one
    genPdgIdForTrk2 = (dR2_1 < dR2_2) ? genDau1PdgId : genDau2PdgId;
  }
  
  // If we couldn't match tracks, can't determine swap
  if(genPdgIdForTrk1 == 0 || genPdgIdForTrk2 == 0) return false;
  
  // Get the mass hypothesis used for each reco track
  // For D0, tracks are assigned pion or kaon mass in the kinematic fit
  // Check the mass stored in the candidate daughter
  double recoDau1Mass = recoDau1->mass();
  double recoDau2Mass = recoDau2->mass();
  
  // Expected masses: K = 0.493677, π = 0.139570
  const double kaonMass = 0.493677;
  const double pionMass = 0.139570;
  const double massTolerance = 0.05;  // 50 MeV tolerance
  
  // Determine what mass hypothesis was used for each track
  bool trk1AssignedKaonMass = (std::abs(recoDau1Mass - kaonMass) < massTolerance);
  bool trk1AssignedPionMass = (std::abs(recoDau1Mass - pionMass) < massTolerance);
  bool trk2AssignedKaonMass = (std::abs(recoDau2Mass - kaonMass) < massTolerance);
  bool trk2AssignedPionMass = (std::abs(recoDau2Mass - pionMass) < massTolerance);
  
  // Check if gen PDG ID matches assigned mass hypothesis
  bool trk1IsKaon = (std::abs(genPdgIdForTrk1) == 321);
  bool trk1IsPion = (std::abs(genPdgIdForTrk1) == 211);
  bool trk2IsKaon = (std::abs(genPdgIdForTrk2) == 321);
  bool trk2IsPion = (std::abs(genPdgIdForTrk2) == 211);
  
  // Check if mass assignment matches gen PDG ID
  bool trk1Correct = (trk1IsKaon && trk1AssignedKaonMass) || (trk1IsPion && trk1AssignedPionMass);
  bool trk2Correct = (trk2IsKaon && trk2AssignedKaonMass) || (trk2IsPion && trk2AssignedPionMass);
  
  // If both are correct, not swapped
  // If both are wrong (opposite), it's swapped
  // If one is wrong, it's partially swapped (shouldn't happen for D0)
  
  return !trk1Correct || !trk2Correct;
}

DEFINE_FWK_MODULE(PATCompositeNtupleProducer);
