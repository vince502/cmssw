// -*- C++ -*-
//
// Package:    VertexCompositeAnalyzer
// Class:      PATCompositeNtupleProducer
//
// Description: Event-based ntuple producer for pat::CompositeCandidate collections
//              One entry per event, vectors for candidate quantities
//              Supports: D0, D04P, DStar, DStar5P, BPlus, BZero, Bc
//
// Author: Soohwan Lee
//

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>

#include <TTree.h>
#include <TVector3.h>
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

#include "CommonTools/UtilAlgos/interface/TFileService.h"

// Candidate type enumeration
enum class CandidateType {
  D0,        // 2-prong: K + pi
  D04P,      // 4-prong: K + 3pi  
  DStar,     // D0(2P) + slow pi
  DStar5P,   // D0(4P) + slow pi
  BPlus,     // J/psi + K
  BZero,     // J/psi + K* (K+pi)
  Bc,        // J/psi + pi
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
  void fillCandidate(const pat::CompositeCandidate& cand, const reco::VertexCollection& vertices, int bestPVIdx);
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

  // Tokens
  edm::EDGetTokenT<reco::BeamSpot> tok_beamSpot_;
  edm::EDGetTokenT<reco::VertexCollection> tok_vertices_;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> tok_candidates_;
  edm::EDGetTokenT<int> tok_centBinLabel_;
  edm::EDGetTokenT<reco::Centrality> tok_centSrc_;

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

  // Daughter info (vectors of vectors)
  std::vector<std::vector<float>> dau_pt_;
  std::vector<std::vector<float>> dau_eta_;
  std::vector<std::vector<float>> dau_phi_;
  std::vector<std::vector<float>> dau_mass_;
  std::vector<std::vector<short>> dau_charge_;
  std::vector<std::vector<float>> dau_dedx_;
  std::vector<std::vector<float>> dau_dzSig_;
  std::vector<std::vector<float>> dau_dxySig_;
  std::vector<std::vector<float>> dau_nhit_;
  std::vector<std::vector<float>> dau_ptErr_;
  std::vector<std::vector<float>> dau_trkChi2_;
  std::vector<std::vector<bool>>  dau_highPurity_;

  // Muon-specific daughter info
  std::vector<std::vector<bool>>  dau_isGlobal_;
  std::vector<std::vector<bool>>  dau_isPF_;
  std::vector<std::vector<bool>>  dau_isSoft_;
  std::vector<std::vector<bool>>  dau_isTight_;
  std::vector<std::vector<bool>>  dau_isHybrid_;
  std::vector<std::vector<short>> dau_nMuonHit_;
  std::vector<std::vector<short>> dau_nMatchedStation_;
  std::vector<std::vector<short>> dau_nTrackerLayer_;
  std::vector<std::vector<short>> dau_nPixelHit_;

  // Grand-daughter info (for two-layer decays)
  std::vector<std::vector<float>> gdau_pt_;
  std::vector<std::vector<float>> gdau_eta_;
  std::vector<std::vector<float>> gdau_phi_;
  std::vector<std::vector<float>> gdau_mass_;
  std::vector<std::vector<short>> gdau_charge_;
  std::vector<std::vector<float>> gdau_dedx_;
  std::vector<std::vector<float>> gdau_dzSig_;
  std::vector<std::vector<float>> gdau_dxySig_;
  std::vector<std::vector<float>> gdau_nhit_;
  std::vector<std::vector<bool>>  gdau_highPurity_;

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
  std::vector<float> bestPVDz_;      // dz to best PV
  std::vector<float> bestPVDzErr_;   // dz error
  std::vector<float> bestPVDxy_;     // dxy to best PV
  std::vector<float> bestPVDxyErr_;  // dxy error
};

PATCompositeNtupleProducer::PATCompositeNtupleProducer(const edm::ParameterSet& iConfig)
  : candType_(iConfig.getParameter<std::string>("candidateType")),
    twoLayerDecay_(iConfig.getUntrackedParameter<bool>("twoLayerDecay", false)),
    doMuon_(iConfig.getUntrackedParameter<bool>("doMuon", false)),
    doElectron_(iConfig.getUntrackedParameter<bool>("doElectron", false)),
    nDau_(iConfig.getUntrackedParameter<unsigned int>("nDaughters", 2)),
    nGDau_(iConfig.getUntrackedParameter<unsigned int>("nGrandDaughters", 0)),
    isCentrality_(iConfig.getUntrackedParameter<bool>("isCentrality", false)),
    saveTree_(iConfig.getUntrackedParameter<bool>("saveTree", true))
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
}

CandidateType PATCompositeNtupleProducer::getCandidateType(const std::string& name) {
  if(name == "D0") return CandidateType::D0;
  if(name == "D04P") return CandidateType::D04P;
  if(name == "DStar") return CandidateType::DStar;
  if(name == "DStar5P") return CandidateType::DStar5P;
  if(name == "BPlus") return CandidateType::BPlus;
  if(name == "BZero") return CandidateType::BZero;
  if(name == "Bc") return CandidateType::Bc;
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

  // Daughter info (vectors of vectors)
  tree_->Branch("pTD", &dau_pt_);
  tree_->Branch("etaD", &dau_eta_);
  tree_->Branch("phiD", &dau_phi_);
  tree_->Branch("massD", &dau_mass_);
  tree_->Branch("chargeD", &dau_charge_);
  tree_->Branch("dedxD", &dau_dedx_);
  tree_->Branch("dzSigD", &dau_dzSig_);
  tree_->Branch("dxySigD", &dau_dxySig_);
  tree_->Branch("nhitD", &dau_nhit_);
  tree_->Branch("ptErrD", &dau_ptErr_);
  tree_->Branch("trkChi2D", &dau_trkChi2_);
  tree_->Branch("highPurityD", &dau_highPurity_);

  // Muon-specific branches
  if(doMuon_) {
    tree_->Branch("isGlobalD", &dau_isGlobal_);
    tree_->Branch("isPFD", &dau_isPF_);
    tree_->Branch("isSoftD", &dau_isSoft_);
    tree_->Branch("isTightD", &dau_isTight_);
    tree_->Branch("isHybridD", &dau_isHybrid_);
    tree_->Branch("nMuonHitD", &dau_nMuonHit_);
    tree_->Branch("nMatchedStationD", &dau_nMatchedStation_);
    tree_->Branch("nTrackerLayerD", &dau_nTrackerLayer_);
    tree_->Branch("nPixelHitD", &dau_nPixelHit_);
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

    tree_->Branch("pTGD", &gdau_pt_);
    tree_->Branch("etaGD", &gdau_eta_);
    tree_->Branch("phiGD", &gdau_phi_);
    tree_->Branch("massGD", &gdau_mass_);
    tree_->Branch("chargeGD", &gdau_charge_);
    tree_->Branch("dedxGD", &gdau_dedx_);
    tree_->Branch("dzSigGD", &gdau_dzSig_);
    tree_->Branch("dxySigGD", &gdau_dxySig_);
    tree_->Branch("nhitGD", &gdau_nhit_);
    tree_->Branch("highPurityGD", &gdau_highPurity_);
  }

  // Best PV info for each candidate
  tree_->Branch("bestPVIdx", &bestPVIdx_);
  tree_->Branch("bestPVDz", &bestPVDz_);
  tree_->Branch("bestPVDzErr", &bestPVDzErr_);
  tree_->Branch("bestPVDxy", &bestPVDxy_);
  tree_->Branch("bestPVDxyErr", &bestPVDxyErr_);
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

  // Daughter info
  dau_pt_.clear();
  dau_eta_.clear();
  dau_phi_.clear();
  dau_mass_.clear();
  dau_charge_.clear();
  dau_dedx_.clear();
  dau_dzSig_.clear();
  dau_dxySig_.clear();
  dau_nhit_.clear();
  dau_ptErr_.clear();
  dau_trkChi2_.clear();
  dau_highPurity_.clear();

  // Muon info
  dau_isGlobal_.clear();
  dau_isPF_.clear();
  dau_isSoft_.clear();
  dau_isTight_.clear();
  dau_isHybrid_.clear();
  dau_nMuonHit_.clear();
  dau_nMatchedStation_.clear();
  dau_nTrackerLayer_.clear();
  dau_nPixelHit_.clear();

  // Grand-daughter info
  gdau_pt_.clear();
  gdau_eta_.clear();
  gdau_phi_.clear();
  gdau_mass_.clear();
  gdau_charge_.clear();
  gdau_dedx_.clear();
  gdau_dzSig_.clear();
  gdau_dxySig_.clear();
  gdau_nhit_.clear();
  gdau_highPurity_.clear();

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
}

void PATCompositeNtupleProducer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Clear all vectors
  clearVectors();
  
  // Get collections
  edm::Handle<reco::BeamSpot> beamSpot;
  iEvent.getByToken(tok_beamSpot_, beamSpot);
  
  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(tok_vertices_, vertices);
  if(!vertices.isValid() || vertices->empty()) return;

  edm::Handle<pat::CompositeCandidateCollection> candidates;
  iEvent.getByToken(tok_candidates_, candidates);
  // Note: candidates may not be valid if producer didn't run, but we still fill event info

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

  // Fill candidates if collection is valid
  candSize_ = 0;
  if(candidates.isValid()) {
    candSize_ = candidates->size();
    for(const auto& cand : *candidates) {
      int bestIdx = findBestPV(cand, *vertices);
      fillCandidate(cand, *vertices, bestIdx);
    }
  }

  // Always fill tree (one entry per event)
  if(saveTree_) tree_->Fill();
}

int PATCompositeNtupleProducer::findBestPV(const pat::CompositeCandidate& cand, const reco::VertexCollection& vertices) {
  // Get candidate decay vertex position
  float candVtxZ = 0;
  if(cand.hasUserData("Vtx")) {
    const reco::Vertex* decayVtx = cand.userData<reco::Vertex>("Vtx");
    if(decayVtx) candVtxZ = decayVtx->z();
  } else {
    candVtxZ = cand.vz();  // fallback to candidate vertex
  }
  
  // Find nearest PV by |dz|
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

void PATCompositeNtupleProducer::fillCandidate(const pat::CompositeCandidate& cand, const reco::VertexCollection& vertices, int bestPVIdx) {
  // Get best PV (or first valid one)
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
  cand_mva_.push_back(cand.hasUserFloat("mva") ? cand.userFloat("mva") : -99.f);

  // Vertex info from userFloat
  float chi2 = cand.hasUserFloat("VtxChi2") ? cand.userFloat("VtxChi2") : -99.f;
  float ndof = cand.hasUserFloat("VtxNdof") ? cand.userFloat("VtxNdof") : -99.f;
  vtxChi2_.push_back(chi2);
  vtxNdof_.push_back(ndof);
  vtxProb_.push_back((chi2 > 0 && ndof > 0) ? TMath::Prob(chi2, ndof) : -99.f);
  
  alpha2D_.push_back(cand.hasUserFloat("alpha2D") ? cand.userFloat("alpha2D") : -99.f);
  alpha3D_.push_back(cand.hasUserFloat("alpha3D") ? cand.userFloat("alpha3D") : -99.f);
  decayLength2D_.push_back(cand.hasUserFloat("decaylength2D") ? cand.userFloat("decaylength2D") : -99.f);
  decayLength3D_.push_back(cand.hasUserFloat("decaylength3D") ? cand.userFloat("decaylength3D") : -99.f);
  decayLengthSig2D_.push_back(cand.hasUserFloat("decaylengthsignif2D") ? cand.userFloat("decaylengthsignif2D") : -99.f);
  decayLengthSig3D_.push_back(cand.hasUserFloat("decaylengthsignif3D") ? cand.userFloat("decaylengthsignif3D") : -99.f);
  dca3D_.push_back(cand.hasUserFloat("dca3D") ? cand.userFloat("dca3D") : -99.f);
  dca3DErr_.push_back(cand.hasUserFloat("dca3DErr") ? cand.userFloat("dca3DErr") : -99.f);
  trackDCA_.push_back(cand.hasUserFloat("track3DDCA") ? cand.userFloat("track3DDCA") : -99.f);
  trackDCAErr_.push_back(cand.hasUserFloat("track3DDCAErr") ? cand.userFloat("track3DDCAErr") : -99.f);

  // DStar specific
  if(candidateType_ == CandidateType::DStar || candidateType_ == CandidateType::DStar5P) {
    deltaM_.push_back(cand.hasUserFloat("deltaM") ? cand.userFloat("deltaM") : -99.f);
  }

  // Get dEdx from userFloat
  float posDauDeDx = cand.hasUserFloat("posDauDeDx") ? cand.userFloat("posDauDeDx") : -999.f;
  float negDauDeDx = cand.hasUserFloat("negDauDeDx") ? cand.userFloat("negDauDeDx") : -999.f;
  float slowPionDeDx = cand.hasUserFloat("slowPionDeDx") ? cand.userFloat("slowPionDeDx") : -999.f;

  // Daughter info - create vectors for this candidate
  std::vector<float> dPt, dEta, dPhi, dMass, dDedx, dDzSig, dDxySig, dNhit, dPtErr, dTrkChi2;
  std::vector<short> dCharge;
  std::vector<bool> dHighPurity;
  std::vector<bool> dIsGlobal, dIsPF, dIsSoft, dIsTight, dIsHybrid;
  std::vector<short> dNMuonHit, dNMatchedStation, dNTrackerLayer, dNPixelHit;

  const ushort nDaughters = std::min(static_cast<ushort>(cand.numberOfDaughters()), nDau_);
  
  for(ushort iDau = 0; iDau < nDaughters; ++iDau) {
    const auto* dau = cand.daughter(iDau);
    if(!dau) continue;

    dPt.push_back(dau->pt());
    dEta.push_back(dau->eta());
    dPhi.push_back(dau->phi());
    dMass.push_back(dau->mass());
    dCharge.push_back(dau->charge());

    // Assign dEdx
    float dedx = -999.f;
    if(candidateType_ == CandidateType::DStar || candidateType_ == CandidateType::DStar5P) {
      dedx = (iDau == 1) ? slowPionDeDx : -999.f;
    } else {
      dedx = (iDau == 0) ? posDauDeDx : negDauDeDx;
    }
    dDedx.push_back(dedx);

    // Track info
    const reco::Track* trk = dau->bestTrack();
    if(trk) {
      const double dz = trk->dz(pvPos);
      const double dxy = trk->dxy(pvPos);
      const double dzErr = std::sqrt(trk->dzError()*trk->dzError() + pv.zError()*pv.zError());
      const double dxyErr = std::sqrt(trk->d0Error()*trk->d0Error() + pv.xError()*pv.yError());
      
      dDzSig.push_back(dz/dzErr);
      dDxySig.push_back(dxy/dxyErr);
      dNhit.push_back(trk->numberOfValidHits());
      dPtErr.push_back(trk->ptError());
      dTrkChi2.push_back(trk->normalizedChi2());
      dHighPurity.push_back(trk->quality(reco::TrackBase::highPurity));
    } else {
      dDzSig.push_back(-99.f);
      dDxySig.push_back(-99.f);
      dNhit.push_back(-1);
      dPtErr.push_back(-99.f);
      dTrkChi2.push_back(-99.f);
      dHighPurity.push_back(false);
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
      dIsGlobal.push_back(isGlobal);
      dIsPF.push_back(isPF);
      dIsSoft.push_back(isSoft);
      dIsTight.push_back(isTight);
      dIsHybrid.push_back(isHybrid);
      dNMuonHit.push_back(nMuonHit);
      dNMatchedStation.push_back(nMatchedStation);
      dNTrackerLayer.push_back(nTrackerLayer);
      dNPixelHit.push_back(nPixelHit);
    }
  }

  // Push daughter vectors
  dau_pt_.push_back(dPt);
  dau_eta_.push_back(dEta);
  dau_phi_.push_back(dPhi);
  dau_mass_.push_back(dMass);
  dau_charge_.push_back(dCharge);
  dau_dedx_.push_back(dDedx);
  dau_dzSig_.push_back(dDzSig);
  dau_dxySig_.push_back(dDxySig);
  dau_nhit_.push_back(dNhit);
  dau_ptErr_.push_back(dPtErr);
  dau_trkChi2_.push_back(dTrkChi2);
  dau_highPurity_.push_back(dHighPurity);

  if(doMuon_) {
    dau_isGlobal_.push_back(dIsGlobal);
    dau_isPF_.push_back(dIsPF);
    dau_isSoft_.push_back(dIsSoft);
    dau_isTight_.push_back(dIsTight);
    dau_isHybrid_.push_back(dIsHybrid);
    dau_nMuonHit_.push_back(dNMuonHit);
    dau_nMatchedStation_.push_back(dNMatchedStation);
    dau_nTrackerLayer_.push_back(dNTrackerLayer);
    dau_nPixelHit_.push_back(dNPixelHit);
  }

  // Two-layer decay: daughter composite + grand-daughters
  if(twoLayerDecay_) {
    float dcMass = -99.f, dcPt = -99.f, dcEta = -99.f, dcVtxProb = -99.f, dcMva = -99.f;
    float dcAlpha3D = -99.f, dcDecLen3D = -99.f, dcDecLenSig3D = -99.f;
    std::vector<float> gPt, gEta, gPhi, gMass, gDedx, gDzSig, gDxySig, gNhit;
    std::vector<short> gCharge;
    std::vector<bool> gHighPurity;

    // Find composite daughter
    for(ushort iDau = 0; iDau < cand.numberOfDaughters(); ++iDau) {
      const auto* dau = cand.daughter(iDau);
      if(!dau || dau->numberOfDaughters() == 0) continue;
      
      const auto* dauCand = dynamic_cast<const pat::CompositeCandidate*>(dau);
      if(dauCand) {
        dcMass = dauCand->mass();
        dcPt = dauCand->pt();
        dcEta = dauCand->eta();
        dcMva = cand.hasUserFloat("D0mva") ? cand.userFloat("D0mva") : -99.f;
        
        float d0Chi2 = dauCand->hasUserFloat("VtxChi2") ? dauCand->userFloat("VtxChi2") : -99.f;
        float d0Ndof = dauCand->hasUserFloat("VtxNdof") ? dauCand->userFloat("VtxNdof") : -99.f;
        dcVtxProb = (d0Chi2 > 0 && d0Ndof > 0) ? TMath::Prob(d0Chi2, d0Ndof) : -99.f;
        dcAlpha3D = dauCand->hasUserFloat("alpha3D") ? dauCand->userFloat("alpha3D") : -99.f;
        dcDecLen3D = dauCand->hasUserFloat("decaylength3D") ? dauCand->userFloat("decaylength3D") : -99.f;
        dcDecLenSig3D = dauCand->hasUserFloat("decaylengthsignif3D") ? dauCand->userFloat("decaylengthsignif3D") : -99.f;

        // Grand-daughters
        const ushort nGDaughters = std::min(static_cast<ushort>(dauCand->numberOfDaughters()), nGDau_);
        for(ushort iGDau = 0; iGDau < nGDaughters; ++iGDau) {
          const auto* gdau = dauCand->daughter(iGDau);
          if(!gdau) continue;

          gPt.push_back(gdau->pt());
          gEta.push_back(gdau->eta());
          gPhi.push_back(gdau->phi());
          gMass.push_back(gdau->mass());
          gCharge.push_back(gdau->charge());

          float gded = -999.f;
          if(iGDau == 0) gded = cand.hasUserFloat("D0posDauDeDx") ? cand.userFloat("D0posDauDeDx") : -999.f;
          else if(iGDau == 1) gded = cand.hasUserFloat("D0negDauDeDx") ? cand.userFloat("D0negDauDeDx") : -999.f;
          gDedx.push_back(gded);

          const reco::Track* gtrk = gdau->bestTrack();
          if(gtrk) {
            const double gdz = gtrk->dz(pvPos);
            const double gdxy = gtrk->dxy(pvPos);
            const double gdzErr = std::sqrt(gtrk->dzError()*gtrk->dzError() + pv.zError()*pv.zError());
            const double gdxyErr = std::sqrt(gtrk->d0Error()*gtrk->d0Error() + pv.xError()*pv.yError());
            
            gDzSig.push_back(gdz/gdzErr);
            gDxySig.push_back(gdxy/gdxyErr);
            gNhit.push_back(gtrk->numberOfValidHits());
            gHighPurity.push_back(gtrk->quality(reco::TrackBase::highPurity));
          } else {
            gDzSig.push_back(-99.f);
            gDxySig.push_back(-99.f);
            gNhit.push_back(-1);
            gHighPurity.push_back(false);
          }
        }
        break; // Only process first composite daughter
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

    gdau_pt_.push_back(gPt);
    gdau_eta_.push_back(gEta);
    gdau_phi_.push_back(gPhi);
    gdau_mass_.push_back(gMass);
    gdau_charge_.push_back(gCharge);
    gdau_dedx_.push_back(gDedx);
    gdau_dzSig_.push_back(gDzSig);
    gdau_dxySig_.push_back(gDxySig);
    gdau_nhit_.push_back(gNhit);
    gdau_highPurity_.push_back(gHighPurity);
  }

  // Best PV info
  bestPVIdx_.push_back(bestPVIdx);
  
  // Calculate dz/dxy to best PV from candidate decay vertex
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
  float dzErr = pv.zError();  // simplified error
  float dxyErr = std::sqrt(pv.xError()*pv.xError() + pv.yError()*pv.yError());
  
  bestPVDz_.push_back(dz);
  bestPVDzErr_.push_back(dzErr);
  bestPVDxy_.push_back(dxy);
  bestPVDxyErr_.push_back(dxyErr);
}

DEFINE_FWK_MODULE(PATCompositeNtupleProducer);
