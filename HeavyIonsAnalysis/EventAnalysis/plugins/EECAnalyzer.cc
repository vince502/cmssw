#include "HeavyIonsAnalysis/EventAnalysis/plugins/EECAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/Math/interface/Vector3D.h"

#include <cmath>
#include <algorithm>

//
// constructors and destructor
//
EECAnalyzer::EECAnalyzer(const edm::ParameterSet& iConfig)
    : jpsiToken_(consumes<pat::CompositeCandidateCollection>(
          iConfig.getParameter<edm::InputTag>("jpsiSrc"))),
      pfCandidateToken_(consumes<pat::PackedCandidateCollection>(
          iConfig.getParameter<edm::InputTag>("pfCandidateSrc"))),
      vertexToken_(consumes<reco::VertexCollection>(
          iConfig.getParameter<edm::InputTag>("vertexSrc"))),
      jpsiMassMin_(iConfig.getParameter<double>("jpsiMassMin")),
      jpsiMassMax_(iConfig.getParameter<double>("jpsiMassMax")),
      sidebandLowMin_(iConfig.getParameter<double>("sidebandLowMin")),
      sidebandLowMax_(iConfig.getParameter<double>("sidebandLowMax")),
      sidebandHighMin_(iConfig.getParameter<double>("sidebandHighMin")),
      sidebandHighMax_(iConfig.getParameter<double>("sidebandHighMax")),
      pfPtMin_(iConfig.getParameter<double>("pfPtMin")),
      pfAbsEtaMax_(iConfig.getParameter<double>("pfAbsEtaMax")),
      pfDzMax_(iConfig.getParameter<double>("pfDzMax")),
      pfDxyMax_(iConfig.getParameter<double>("pfDxyMax")),
      ctauCut_(iConfig.getParameter<double>("ctauCut")),
      jpsiPtBins_(iConfig.getParameter<std::vector<double>>("jpsiPtBins")),
      jpsiRapBins_(iConfig.getParameter<std::vector<double>>("jpsiRapBins")),
      removeJpsiMuons_(iConfig.getParameter<bool>("removeJpsiMuons")),
      separatePromptNonprompt_(iConfig.getParameter<bool>("separatePromptNonprompt")) {}

EECAnalyzer::~EECAnalyzer() {}

//
// member functions
//

void EECAnalyzer::beginJob() {
  // Create tree (one entry per event, like HiOniaAnalyzer)
  tree_ = fs_->make<TTree>("eecTree", "EEC analysis tree");
  
  // Event-level branches
  tree_->Branch("runNb", &runNb_, "runNb/l");
  tree_->Branch("eventNb", &eventNb_, "eventNb/l");
  tree_->Branch("lumiSection", &lumiSection_, "lumiSection/l");
  tree_->Branch("nPV", &nPV_, "nPV/I");
  tree_->Branch("zVtx", &zVtx_, "zVtx/F");
  
  // J/psi branches (arrays of size nJpsi_)
  // Only EEC-specific information; kinematic info available via Jpsi_oniaIdx in onia tree
  tree_->Branch("nJpsi", &nJpsi_, "nJpsi/I");
  tree_->Branch("Jpsi_oniaIdx", &Jpsi_oniaIdx_);      // Index to Reco_QQ_XXX[index] in onia tree
  tree_->Branch("Jpsi_isPrompt", &Jpsi_isPrompt_);    // EEC-specific prompt classification
  tree_->Branch("Jpsi_isSignal", &Jpsi_isSignal_);     // EEC-specific signal window flag
  tree_->Branch("Jpsi_isSideband", &Jpsi_isSideband_); // EEC-specific sideband flag
  tree_->Branch("Jpsi_nPF", &Jpsi_nPF_);              // EEC-specific: number of PF candidates
  
  // PF candidate branches (flattened, with index to J/psi)
  tree_->Branch("nPF", &nPF_, "nPF/I");
  tree_->Branch("PF_jpsiIdx", &PF_jpsiIdx_);
  tree_->Branch("PF_pt", &PF_pt_);
  tree_->Branch("PF_eta", &PF_eta_);
  tree_->Branch("PF_phi", &PF_phi_);
  tree_->Branch("PF_energy", &PF_energy_);
  tree_->Branch("PF_charge", &PF_charge_);
  tree_->Branch("PF_fromPV", &PF_fromPV_);
  tree_->Branch("PF_dz", &PF_dz_);
  tree_->Branch("PF_dxy", &PF_dxy_);
  tree_->Branch("PF_ptStar", &PF_ptStar_);
  tree_->Branch("PF_etaStar", &PF_etaStar_);
  tree_->Branch("PF_phiStar", &PF_phiStar_);
  tree_->Branch("PF_energyStar", &PF_energyStar_);
  tree_->Branch("PF_cosThetaStar", &PF_cosThetaStar_);
  tree_->Branch("PF_weight", &PF_weight_);
}

void EECAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup&) {
  clear();

  // Event-level info
  runNb_ = iEvent.id().run();
  eventNb_ = iEvent.id().event();
  lumiSection_ = iEvent.luminosityBlock();

  edm::Handle<pat::CompositeCandidateCollection> jpsiCandidates;
  iEvent.getByToken(jpsiToken_, jpsiCandidates);

  edm::Handle<pat::PackedCandidateCollection> pfCandidates;
  iEvent.getByToken(pfCandidateToken_, pfCandidates);

  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(vertexToken_, vertices);

  if (vertices->empty()) return;
  
  const reco::Vertex& pv = vertices->at(0);
  nPV_ = vertices->size();
  zVtx_ = pv.z();

  if (jpsiCandidates->empty() || pfCandidates->empty()) {
    tree_->Fill();
    return;
  }

  // Loop over J/psi candidates with index tracking
  int oniaIdx = 0;
  for (const auto& jpsi : *jpsiCandidates) {
    double pt = jpsi.pt();
    double rap = jpsi.rapidity();
    
    // Basic J/psi selection
    if (pt < jpsiPtBins_[0] || pt > jpsiPtBins_.back()) {
      oniaIdx++;
      continue;
    }
    if (std::abs(rap) > 2.4) {
      oniaIdx++;
      continue;
    }
    
    // Fill J/psi info with index to onia tree
    fillJpsiInfo(jpsi, pv, oniaIdx);
    
    // Fill PF candidates for this J/psi
    fillPFCandidates(jpsi, *pfCandidates, pv);
    
    oniaIdx++;
  }

  tree_->Fill();
}

void EECAnalyzer::fillJpsiInfo(const pat::CompositeCandidate& jpsi, const reco::Vertex& pv, int oniaIdx) {
  nJpsi_++;
  
  // Store index to corresponding entry in onia tree (Reco_QQ_XXX[index])
  Jpsi_oniaIdx_.push_back(oniaIdx);
  
  // EEC-specific classification flags
  Jpsi_isPrompt_.push_back(isPromptJpsi(jpsi, pv) ? 1 : 0);
  Jpsi_isSignal_.push_back(isInSignalWindow(jpsi) ? 1 : 0);
  Jpsi_isSideband_.push_back(isInSideband(jpsi) ? 1 : 0);
  
  // Will be filled in fillPFCandidates
  Jpsi_nPF_.push_back(0);
}

void EECAnalyzer::fillPFCandidates(const pat::CompositeCandidate& jpsi,
                                   const pat::PackedCandidateCollection& pfCandidates,
                                   const reco::Vertex& pv) {
  // Build J/psi 4-vector
  TLorentzVector jpsiVec;
  jpsiVec.SetPtEtaPhiM(jpsi.pt(), jpsi.eta(), jpsi.phi(), jpsi.mass());
  
  // J/psi mass for normalization
  double mJpsi = jpsi.mass();
  if (mJpsi < 0.1) mJpsi = 3.096; // Use PDG mass if not available
  
  // Unit vector along J/psi direction in lab frame (helicity axis)
  TVector3 jpsiDirLab = jpsiVec.Vect().Unit();
  
  // Get J/psi muons for removal if needed
  std::vector<const reco::Candidate*> jpsiMuons;
  if (removeJpsiMuons_) {
    if (jpsi.numberOfDaughters() >= 2) {
      jpsiMuons.push_back(jpsi.daughter(0));
      jpsiMuons.push_back(jpsi.daughter(1));
    }
  }
  
  int nPF_thisJpsi = 0;
  int jpsiIdx = nJpsi_ - 1;  // Current J/psi index (0-based)
  
  // Loop over PF candidates
  for (const auto& pfCand : pfCandidates) {
    if (!passPFCandidateSelection(pfCand, jpsi)) continue;
    
    // Check if this is a J/psi muon
    if (removeJpsiMuons_) {
      bool isJpsiMuon = false;
      for (const auto* mu : jpsiMuons) {
        if (mu) {
          double deta = pfCand.eta() - mu->eta();
          double dphi = reco::deltaPhi(pfCand.phi(), mu->phi());
          double dr = std::sqrt(deta*deta + dphi*dphi);
          if (dr < 0.01) {
            isJpsiMuon = true;
            break;
          }
        }
      }
      if (isJpsiMuon) continue;
    }
    
    // Lab frame quantities
    PF_jpsiIdx_.push_back(jpsiIdx);
    PF_pt_.push_back(pfCand.pt());
    PF_eta_.push_back(pfCand.eta());
    PF_phi_.push_back(pfCand.phi());
    PF_energy_.push_back(pfCand.energy());
    PF_charge_.push_back(pfCand.charge());
    PF_fromPV_.push_back(pfCand.fromPV());
    PF_dz_.push_back(pfCand.dz());
    PF_dxy_.push_back(pfCand.dxy());
    
    // Build PF candidate 4-vector
    TLorentzVector pfVec;
    pfVec.SetPtEtaPhiE(pfCand.pt(), pfCand.eta(), pfCand.phi(), pfCand.energy());
    
    // Boost to J/psi rest frame
    TLorentzVector pfVecRest = boostToRestFrame(pfVec, jpsiVec);
    
    // Rest frame quantities
    PF_ptStar_.push_back(pfVecRest.Pt());
    PF_etaStar_.push_back(pfVecRest.Eta());
    PF_phiStar_.push_back(pfVecRest.Phi());
    PF_energyStar_.push_back(pfVecRest.E());
    
    // Compute cos(theta*) = p_i* · p_Jpsi_lab / |p_i*| / |p_Jpsi_lab|
    TVector3 pfDirRest = pfVecRest.Vect().Unit();
    double cosThetaStar = pfDirRest.Dot(jpsiDirLab);
    PF_cosThetaStar_.push_back(cosThetaStar);
    
    // Weight: E_i* / M_J/psi
    double weight = pfVecRest.E() / mJpsi;
    PF_weight_.push_back(weight);
    
    nPF_thisJpsi++;
    nPF_++;
  }
  
  // Update nPF for this J/psi
  Jpsi_nPF_[jpsiIdx] = nPF_thisJpsi;
}

bool EECAnalyzer::isPromptJpsi(const pat::CompositeCandidate& jpsi, const reco::Vertex& pv) {
  double ctau = -999.0;
  if (jpsi.hasUserFloat("ppdlPV")) {
    ctau = jpsi.userFloat("ppdlPV");
  } else if (jpsi.hasUserFloat("ppdlBS")) {
    ctau = jpsi.userFloat("ppdlBS");
  }
  return (ctau < ctauCut_);
}

bool EECAnalyzer::isInSignalWindow(const pat::CompositeCandidate& jpsi) {
  double mass = jpsi.mass();
  return (mass >= jpsiMassMin_ && mass <= jpsiMassMax_);
}

bool EECAnalyzer::isInSideband(const pat::CompositeCandidate& jpsi) {
  double mass = jpsi.mass();
  return ((mass >= sidebandLowMin_ && mass <= sidebandLowMax_) ||
          (mass >= sidebandHighMin_ && mass <= sidebandHighMax_));
}

bool EECAnalyzer::passPFCandidateSelection(const pat::PackedCandidate& pfCand,
                                           const pat::CompositeCandidate& jpsi) {
  // Basic kinematic cuts
  if (pfCand.pt() < pfPtMin_) return false;
  if (std::abs(pfCand.eta()) > pfAbsEtaMax_) return false;
  
  // PV association cuts (for charged candidates) - pileup mitigation
  if (pfCand.charge() != 0) {
    // Strict PV association for charged particles to reduce pileup
    if (std::abs(pfCand.dz()) > pfDzMax_) return false;
    if (std::abs(pfCand.dxy()) > pfDxyMax_) return false;
    
    // Additional: require fromPV flag if available (MiniAOD packed candidates)
    // fromPV() returns: 0=no PV, 1=first PV, 2=any PV, 3=any PV with loose dz
    // We want fromPV >= 1 (associated with primary vertex)
    if (pfCand.fromPV() == 0) return false;
  } else {
    // For neutral particles, we rely on PUPPI weights if available
    // If not using PUPPI, neutrals are more pileup-sensitive
    // For now, we keep them but this should be reviewed
    // TODO: Consider adding PUPPI weight cut or removing neutrals entirely
  }
  
  return true;
}

TLorentzVector EECAnalyzer::boostToRestFrame(const TLorentzVector& vec, 
                                             const TLorentzVector& boostVec) {
  // Boost vector
  TVector3 beta = boostVec.BoostVector();
  
  // Boost the vector
  TLorentzVector boosted = vec;
  boosted.Boost(-beta.X(), -beta.Y(), -beta.Z());
  
  return boosted;
}

void EECAnalyzer::endJob() {}

void EECAnalyzer::clear() {
  // Event-level
  runNb_ = 0;
  eventNb_ = 0;
  lumiSection_ = 0;
  nPV_ = 0;
  zVtx_ = 0.0;
  
  // J/psi
  nJpsi_ = 0;
  Jpsi_oniaIdx_.clear();
  Jpsi_isPrompt_.clear();
  Jpsi_isSignal_.clear();
  Jpsi_isSideband_.clear();
  Jpsi_nPF_.clear();
  
  // PF candidates
  nPF_ = 0;
  PF_jpsiIdx_.clear();
  PF_pt_.clear();
  PF_eta_.clear();
  PF_phi_.clear();
  PF_energy_.clear();
  PF_charge_.clear();
  PF_fromPV_.clear();
  PF_dz_.clear();
  PF_dxy_.clear();
  PF_ptStar_.clear();
  PF_etaStar_.clear();
  PF_phiStar_.clear();
  PF_energyStar_.clear();
  PF_cosThetaStar_.clear();
  PF_weight_.clear();
}

DEFINE_FWK_MODULE(EECAnalyzer);
