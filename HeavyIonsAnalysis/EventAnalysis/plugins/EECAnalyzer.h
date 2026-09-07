#ifndef EECAnalyzer_h
#define EECAnalyzer_h

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "TTree.h"
#include "TLorentzVector.h"
#include "TVector3.h"

#include <vector>
#include <string>
#include <map>

class EECAnalyzer : public edm::one::EDAnalyzer<> {
public:
  explicit EECAnalyzer(const edm::ParameterSet&);
  ~EECAnalyzer() override;

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  void clear();
  void fillJpsiInfo(const pat::CompositeCandidate& jpsi, const reco::Vertex& pv, int oniaIdx);
  void fillPFCandidates(const pat::CompositeCandidate& jpsi,
                       const pat::PackedCandidateCollection& pfCandidates,
                       const reco::Vertex& pv);
  
  bool isPromptJpsi(const pat::CompositeCandidate& jpsi, const reco::Vertex& pv);
  bool isInSignalWindow(const pat::CompositeCandidate& jpsi);
  bool isInSideband(const pat::CompositeCandidate& jpsi);
  bool passPFCandidateSelection(const pat::PackedCandidate& pfCand, 
                                 const pat::CompositeCandidate& jpsi);
  TLorentzVector boostToRestFrame(const TLorentzVector& vec, const TLorentzVector& boostVec);

private:
  // ----------member data ---------------------------
  edm::Service<TFileService> fs_;

  edm::EDGetTokenT<pat::CompositeCandidateCollection> jpsiToken_;
  edm::EDGetTokenT<pat::PackedCandidateCollection> pfCandidateToken_;
  edm::EDGetTokenT<reco::VertexCollection> vertexToken_;

  // Configuration parameters
  double jpsiMassMin_;
  double jpsiMassMax_;
  double sidebandLowMin_;
  double sidebandLowMax_;
  double sidebandHighMin_;
  double sidebandHighMax_;
  double pfPtMin_;
  double pfAbsEtaMax_;
  double pfDzMax_;
  double pfDxyMax_;
  double ctauCut_;
  std::vector<double> jpsiPtBins_;
  std::vector<double> jpsiRapBins_;
  bool removeJpsiMuons_;
  bool separatePromptNonprompt_;

  // Tree (one entry per event, like HiOniaAnalyzer)
  TTree* tree_;
  
  // Event-level variables
  ULong64_t runNb_;
  ULong64_t eventNb_;
  ULong64_t lumiSection_;
  int nPV_;
  float zVtx_;
  
  // Per-J/psi variables (arrays of size nJpsi_)
  int nJpsi_;
  std::vector<int> Jpsi_oniaIdx_;      // Index to corresponding entry in onia tree (Reco_QQ_XXX[index])
  std::vector<int> Jpsi_isPrompt_;    // EEC-specific: prompt classification
  std::vector<int> Jpsi_isSignal_;     // EEC-specific: signal window flag
  std::vector<int> Jpsi_isSideband_;   // EEC-specific: sideband flag
  std::vector<int> Jpsi_nPF_;          // EEC-specific: Number of PF candidates per J/psi
  
  // PF candidate variables (flattened, with index to J/psi)
  // Total number of PF candidates across all J/psi in this event
  int nPF_;
  std::vector<int> PF_jpsiIdx_;      // Index to which J/psi this PF belongs (0 to nJpsi_-1)
  std::vector<float> PF_pt_;         // Lab frame
  std::vector<float> PF_eta_;
  std::vector<float> PF_phi_;
  std::vector<float> PF_energy_;
  std::vector<int> PF_charge_;
  std::vector<int> PF_fromPV_;
  std::vector<float> PF_dz_;
  std::vector<float> PF_dxy_;
  std::vector<float> PF_ptStar_;     // Rest frame (boosted to J/psi rest frame)
  std::vector<float> PF_etaStar_;
  std::vector<float> PF_phiStar_;
  std::vector<float> PF_energyStar_;
  std::vector<float> PF_cosThetaStar_;  // cos(θ*) relative to helicity axis
  std::vector<float> PF_weight_;        // E*/M_J/psi (weight for EEC)
};

#endif
