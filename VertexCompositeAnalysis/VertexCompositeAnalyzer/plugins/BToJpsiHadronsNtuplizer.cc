// -*- C++ -*-
//
// Package:    VertexCompositeAnalyzer
// Class:      BToJpsiHadronsNtuplizer
//
// Description: Unified ntuplizer for BToJpsiHadronsProducer output
//              Produces flat ntuple with uniform schema for all decay modes
//
// Decay modes (decayId):
//   0 = B+  -> J/psi K+    (1 hadron)
//   1 = B+  -> J/psi pi+   (1 hadron)
//   2 = B0  -> J/psi K pi  (2 hadrons)
//   3 = Bs  -> J/psi K K   (2 hadrons)
//   4 = Bc+ -> J/psi pi+   (1 hadron)
//   5 = X   -> J/psi pi pi (2 hadrons)
//
// Author: Auto-generated for CMS B-physics analysis

#include <memory>
#include <vector>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/HeavyIonEvent/interface/Centrality.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include <TTree.h>

class BToJpsiHadronsNtuplizer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit BToJpsiHadronsNtuplizer(const edm::ParameterSet&);
  ~BToJpsiHadronsNtuplizer() override = default;

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override {}

  void initTree();
  void clearVectors();

  // Tokens
  edm::EDGetTokenT<pat::CompositeCandidateCollection> candToken_;
  edm::EDGetTokenT<reco::VertexCollection> pvToken_;
  edm::EDGetTokenT<int> centBinToken_;
  edm::EDGetTokenT<reco::Centrality> centToken_;

  // Configuration
  bool isCentrality_;
  bool saveTree_;

  // Tree
  edm::Service<TFileService> fs_;
  TTree* tree_;

  // Event info
  UInt_t run_, lumi_, event_;
  Short_t nPV_;
  Short_t centrality_;
  Int_t Ntrkoffline_;
  Float_t pvX_, pvY_, pvZ_;
  UInt_t nCand_;

  // Candidate info (vectors - one element per candidate)
  // B candidate
  std::vector<float> B_mass_;
  std::vector<float> B_pt_;
  std::vector<float> B_eta_;
  std::vector<float> B_phi_;
  std::vector<float> B_y_;
  std::vector<int>   B_charge_;
  std::vector<int>   B_decayId_;
  std::vector<int>   B_nTracks_;
  std::vector<int>   B_hasDau2_;
  
  // Vertex info
  std::vector<float> B_vtxX_;
  std::vector<float> B_vtxY_;
  std::vector<float> B_vtxZ_;
  std::vector<float> B_vtxChi2_;
  std::vector<float> B_vtxNdof_;
  std::vector<float> B_vtxProb_;
  std::vector<float> B_lxy_;
  std::vector<float> B_lxySig_;
  std::vector<float> B_l3D_;
  std::vector<float> B_l3DSig_;
  std::vector<float> B_cosAlpha_;
  
  // J/psi info
  std::vector<int>   jpsiIdx_;  // Index for matching with hionia tree
  std::vector<float> Jpsi_mass_;
  std::vector<float> Jpsi_pt_;
  std::vector<float> Jpsi_eta_;
  std::vector<float> Jpsi_phi_;
  std::vector<float> Jpsi_y_;
  
  // Resonance masses (for 2-track modes)
  std::vector<float> resMass_;  // phi, K*, or pipi mass
  
  // Hadron 1 info
  std::vector<float> trk1_pt_;
  std::vector<float> trk1_eta_;
  std::vector<float> trk1_phi_;
  std::vector<int>   trk1_charge_;
  std::vector<float> trk1_massHypo_;
  std::vector<int>   trk1_idx_;
  
  // Hadron 2 info (filled with -999 for 1-track modes)
  std::vector<float> trk2_pt_;
  std::vector<float> trk2_eta_;
  std::vector<float> trk2_phi_;
  std::vector<int>   trk2_charge_;
  std::vector<float> trk2_massHypo_;
  std::vector<int>   trk2_idx_;
};

BToJpsiHadronsNtuplizer::BToJpsiHadronsNtuplizer(const edm::ParameterSet& iConfig)
    : candToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("candidateSrc"))),
      pvToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("pvSrc"))),
      isCentrality_(iConfig.getUntrackedParameter<bool>("isCentrality", false)),
      saveTree_(iConfig.getUntrackedParameter<bool>("saveTree", true))
{
  usesResource("TFileService");
  
  if (isCentrality_) {
    centBinToken_ = consumes<int>(iConfig.getParameter<edm::InputTag>("centralityBinLabel"));
    centToken_ = consumes<reco::Centrality>(iConfig.getParameter<edm::InputTag>("centralitySrc"));
  }
}

void BToJpsiHadronsNtuplizer::beginJob() {
  if (saveTree_) initTree();
}

void BToJpsiHadronsNtuplizer::initTree() {
  tree_ = fs_->make<TTree>("bTree", "B meson candidates");
  
  // Event info
  tree_->Branch("run", &run_);
  tree_->Branch("lumi", &lumi_);
  tree_->Branch("event", &event_);
  tree_->Branch("nPV", &nPV_);
  tree_->Branch("pvX", &pvX_);
  tree_->Branch("pvY", &pvY_);
  tree_->Branch("pvZ", &pvZ_);
  tree_->Branch("nCand", &nCand_);
  
  if (isCentrality_) {
    tree_->Branch("centrality", &centrality_);
    tree_->Branch("Ntrkoffline", &Ntrkoffline_);
  }
  
  // B candidate info
  tree_->Branch("B_mass", &B_mass_);
  tree_->Branch("B_pt", &B_pt_);
  tree_->Branch("B_eta", &B_eta_);
  tree_->Branch("B_phi", &B_phi_);
  tree_->Branch("B_y", &B_y_);
  tree_->Branch("B_charge", &B_charge_);
  tree_->Branch("B_decayId", &B_decayId_);
  tree_->Branch("B_nTracks", &B_nTracks_);
  tree_->Branch("B_hasDau2", &B_hasDau2_);
  
  // Vertex info
  tree_->Branch("B_vtxX", &B_vtxX_);
  tree_->Branch("B_vtxY", &B_vtxY_);
  tree_->Branch("B_vtxZ", &B_vtxZ_);
  tree_->Branch("B_vtxChi2", &B_vtxChi2_);
  tree_->Branch("B_vtxNdof", &B_vtxNdof_);
  tree_->Branch("B_vtxProb", &B_vtxProb_);
  tree_->Branch("B_lxy", &B_lxy_);
  tree_->Branch("B_lxySig", &B_lxySig_);
  tree_->Branch("B_l3D", &B_l3D_);
  tree_->Branch("B_l3DSig", &B_l3DSig_);
  tree_->Branch("B_cosAlpha", &B_cosAlpha_);
  
  // J/psi info
  tree_->Branch("jpsiIdx", &jpsiIdx_);
  tree_->Branch("Jpsi_mass", &Jpsi_mass_);
  tree_->Branch("Jpsi_pt", &Jpsi_pt_);
  tree_->Branch("Jpsi_eta", &Jpsi_eta_);
  tree_->Branch("Jpsi_phi", &Jpsi_phi_);
  tree_->Branch("Jpsi_y", &Jpsi_y_);
  
  // Resonance mass
  tree_->Branch("resMass", &resMass_);
  
  // Hadron 1
  tree_->Branch("trk1_pt", &trk1_pt_);
  tree_->Branch("trk1_eta", &trk1_eta_);
  tree_->Branch("trk1_phi", &trk1_phi_);
  tree_->Branch("trk1_charge", &trk1_charge_);
  tree_->Branch("trk1_massHypo", &trk1_massHypo_);
  tree_->Branch("trk1_idx", &trk1_idx_);
  
  // Hadron 2
  tree_->Branch("trk2_pt", &trk2_pt_);
  tree_->Branch("trk2_eta", &trk2_eta_);
  tree_->Branch("trk2_phi", &trk2_phi_);
  tree_->Branch("trk2_charge", &trk2_charge_);
  tree_->Branch("trk2_massHypo", &trk2_massHypo_);
  tree_->Branch("trk2_idx", &trk2_idx_);
}

void BToJpsiHadronsNtuplizer::clearVectors() {
  B_mass_.clear();
  B_pt_.clear();
  B_eta_.clear();
  B_phi_.clear();
  B_y_.clear();
  B_charge_.clear();
  B_decayId_.clear();
  B_nTracks_.clear();
  B_hasDau2_.clear();
  
  B_vtxX_.clear();
  B_vtxY_.clear();
  B_vtxZ_.clear();
  B_vtxChi2_.clear();
  B_vtxNdof_.clear();
  B_vtxProb_.clear();
  B_lxy_.clear();
  B_lxySig_.clear();
  B_l3D_.clear();
  B_l3DSig_.clear();
  B_cosAlpha_.clear();
  
  jpsiIdx_.clear();
  Jpsi_mass_.clear();
  Jpsi_pt_.clear();
  Jpsi_eta_.clear();
  Jpsi_phi_.clear();
  Jpsi_y_.clear();
  
  resMass_.clear();
  
  trk1_pt_.clear();
  trk1_eta_.clear();
  trk1_phi_.clear();
  trk1_charge_.clear();
  trk1_massHypo_.clear();
  trk1_idx_.clear();
  
  trk2_pt_.clear();
  trk2_eta_.clear();
  trk2_phi_.clear();
  trk2_charge_.clear();
  trk2_massHypo_.clear();
  trk2_idx_.clear();
}

void BToJpsiHadronsNtuplizer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  clearVectors();
  
  // Event info
  run_ = iEvent.id().run();
  lumi_ = iEvent.luminosityBlock();
  event_ = iEvent.id().event();
  
  // Primary vertex
  edm::Handle<reco::VertexCollection> pvHandle;
  iEvent.getByToken(pvToken_, pvHandle);
  
  if (!pvHandle.isValid() || pvHandle->empty()) {
    if (saveTree_) tree_->Fill();
    return;
  }
  
  nPV_ = pvHandle->size();
  const auto& pv = pvHandle->front();
  pvX_ = pv.x();
  pvY_ = pv.y();
  pvZ_ = pv.z();
  
  // Centrality
  centrality_ = -1;
  Ntrkoffline_ = -1;
  if (isCentrality_) {
    edm::Handle<int> centBin;
    iEvent.getByToken(centBinToken_, centBin);
    if (centBin.isValid()) centrality_ = *centBin;
    
    edm::Handle<reco::Centrality> cent;
    iEvent.getByToken(centToken_, cent);
    if (cent.isValid()) Ntrkoffline_ = cent->Ntracks();
  }
  
  // Get B candidates
  edm::Handle<pat::CompositeCandidateCollection> candHandle;
  iEvent.getByToken(candToken_, candHandle);
  
  nCand_ = 0;
  if (candHandle.isValid()) {
    nCand_ = candHandle->size();
    
    for (const auto& cand : *candHandle) {
      // B candidate kinematics
      B_mass_.push_back(cand.mass());
      B_pt_.push_back(cand.pt());
      B_eta_.push_back(cand.eta());
      B_phi_.push_back(cand.phi());
      B_y_.push_back(cand.rapidity());
      B_charge_.push_back(cand.charge());
      
      // Decay mode info
      B_decayId_.push_back(cand.hasUserInt("decayId") ? cand.userInt("decayId") : -1);
      B_nTracks_.push_back(cand.hasUserInt("nTracks") ? cand.userInt("nTracks") : -1);
      B_hasDau2_.push_back(cand.hasUserInt("hasDaughter2") ? cand.userInt("hasDaughter2") : -1);
      
      // Vertex info
      B_vtxX_.push_back(cand.hasUserFloat("vtxX") ? cand.userFloat("vtxX") : -99.f);
      B_vtxY_.push_back(cand.hasUserFloat("vtxY") ? cand.userFloat("vtxY") : -99.f);
      B_vtxZ_.push_back(cand.hasUserFloat("vtxZ") ? cand.userFloat("vtxZ") : -99.f);
      B_vtxChi2_.push_back(cand.hasUserFloat("vtxChi2") ? cand.userFloat("vtxChi2") : -99.f);
      B_vtxNdof_.push_back(cand.hasUserFloat("vtxNdof") ? cand.userFloat("vtxNdof") : -99.f);
      B_vtxProb_.push_back(cand.hasUserFloat("vtxProb") ? cand.userFloat("vtxProb") : -99.f);
      B_lxy_.push_back(cand.hasUserFloat("lxy") ? cand.userFloat("lxy") : -99.f);
      B_lxySig_.push_back(cand.hasUserFloat("lxySig") ? cand.userFloat("lxySig") : -99.f);
      B_l3D_.push_back(cand.hasUserFloat("l3D") ? cand.userFloat("l3D") : -99.f);
      B_l3DSig_.push_back(cand.hasUserFloat("l3DSig") ? cand.userFloat("l3DSig") : -99.f);
      B_cosAlpha_.push_back(cand.hasUserFloat("cosAlpha") ? cand.userFloat("cosAlpha") : -99.f);
      
      // J/psi info (stored as userFloats in producer for reliability)
      jpsiIdx_.push_back(cand.hasUserInt("jpsiIdx") ? cand.userInt("jpsiIdx") : -1);
      Jpsi_mass_.push_back(cand.hasUserFloat("jpsiMass") ? cand.userFloat("jpsiMass") : -99.f);
      Jpsi_pt_.push_back(cand.hasUserFloat("jpsiPt") ? cand.userFloat("jpsiPt") : -99.f);
      Jpsi_eta_.push_back(cand.hasUserFloat("jpsiEta") ? cand.userFloat("jpsiEta") : -99.f);
      Jpsi_phi_.push_back(cand.hasUserFloat("jpsiPhi") ? cand.userFloat("jpsiPhi") : -99.f);
      Jpsi_y_.push_back(cand.hasUserFloat("jpsiY") ? cand.userFloat("jpsiY") : -99.f);
      
      // Resonance mass (phi, K*, pipi)
      float resMass = -99.f;
      if (cand.hasUserFloat("phiMass")) resMass = cand.userFloat("phiMass");
      else if (cand.hasUserFloat("kstarMass")) resMass = cand.userFloat("kstarMass");
      else if (cand.hasUserFloat("pipiMass")) resMass = cand.userFloat("pipiMass");
      resMass_.push_back(resMass);
      
      // Hadron 1 (stored as userFloats in producer)
      trk1_pt_.push_back(cand.hasUserFloat("trk1Pt") ? cand.userFloat("trk1Pt") : -99.f);
      trk1_eta_.push_back(cand.hasUserFloat("trk1Eta") ? cand.userFloat("trk1Eta") : -99.f);
      trk1_phi_.push_back(cand.hasUserFloat("trk1Phi") ? cand.userFloat("trk1Phi") : -99.f);
      trk1_charge_.push_back(cand.hasUserInt("trk1Charge") ? cand.userInt("trk1Charge") : 0);
      trk1_massHypo_.push_back(cand.hasUserFloat("trk1Mass") ? cand.userFloat("trk1Mass") : -99.f);
      trk1_idx_.push_back(cand.hasUserInt("trk1Idx") ? cand.userInt("trk1Idx") : -1);
      
      // Hadron 2 (filled with -999 for 1-track modes)
      trk2_pt_.push_back(cand.hasUserFloat("trk2Pt") ? cand.userFloat("trk2Pt") : -999.f);
      trk2_eta_.push_back(cand.hasUserFloat("trk2Eta") ? cand.userFloat("trk2Eta") : -999.f);
      trk2_phi_.push_back(cand.hasUserFloat("trk2Phi") ? cand.userFloat("trk2Phi") : -999.f);
      trk2_charge_.push_back(cand.hasUserInt("trk2Charge") ? cand.userInt("trk2Charge") : 0);
      trk2_massHypo_.push_back(cand.hasUserFloat("trk2Mass") ? cand.userFloat("trk2Mass") : -999.f);
      trk2_idx_.push_back(cand.hasUserInt("trk2Idx") ? cand.userInt("trk2Idx") : -1);
    }
  }
  
  if (saveTree_) tree_->Fill();
}

DEFINE_FWK_MODULE(BToJpsiHadronsNtuplizer);
