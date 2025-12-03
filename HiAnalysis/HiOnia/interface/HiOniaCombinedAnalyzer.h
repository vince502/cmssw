#ifndef HIANALYSIS_HIONIA_HIONIACOMBINEDANALYZER_H
#define HIANALYSIS_HIONIA_HIONIACOMBINEDANALYZER_H

// Combined analyzer for dimuon and dielectron

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <TLorentzVector.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TVector3.h>
#include <TRegexp.h>

// Common includes
#include "CommonTools/Egamma/interface/ConversionTools.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/Candidate/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HeavyIonEvent/interface/Centrality.h"
#include "DataFormats/HeavyIonEvent/interface/EvtPlane.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "Math/GenVector/VectorUtil.h"
#include "Math/Point3D.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

// Muon-specific includes
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"

// Electron-specific includes
#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/PatCandidates/interface/Electron.h"

class HiOniaCombinedAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources, edm::one::WatchRuns> {
public:
  explicit HiOniaCombinedAnalyzer(const edm::ParameterSet&);
  ~HiOniaCombinedAnalyzer() override;

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;
  void beginRun(const edm::Run&, const edm::EventSetup&) override;
  void endRun(const edm::Run&, const edm::EventSetup&) override;

  void InitTree();
  void InitEvent();

  // Muon-related methods
  void fillRecoMuons(const edm::Event&);
  void fillRecoDimuons(const edm::Event&);
  void fillGenMuons(const edm::Event&);
  int IndexOfThisMuon(TLorentzVector* v1, bool isGen = false);
  bool selGlobalMuon(const pat::Muon* aMuon);
  bool selTrackerMuon(const pat::Muon* aMuon);
  bool selGlobalOrTrackerMuon(const pat::Muon* aMuon);
  bool isMuonInAccept(const pat::Muon* aMuon, std::string muonType);
  bool isSoftMuonBase(const pat::Muon* aMuon);
  bool isHybridSoftMuon(const pat::Muon* aMuon);
  void checkMuonTriggers(const pat::CompositeCandidate* aJpsiCand);
  TLorentzVector lorentzMomentum(const reco::Candidate::LorentzVector& p);
  long int FloatToIntkey(float v);

  // Electron-related methods
  void fillRecoElectrons(const edm::Event&);
  void fillRecoDielectrons(const edm::Event&);
  void fillGenElectrons(const edm::Event&);
  int findElectronIndex(const TLorentzVector& candP4) const;
  std::string sanitizeLabel(const std::string& raw) const;

  // Common methods
  void fillEventInfo(const edm::Event&);
  void fillPrimaryVertexInfo(const edm::Event&);
  void fillCentralityInfo(const edm::Event&);
  void fillTriggerInfo(const edm::Event&);
  std::string resolveTriggerName(const std::string& requested, const edm::TriggerNames&) const;

  // Array sizes
  static constexpr int Max_mu_size = 1000;
  static constexpr int Max_QQ_size = 10000;
  static constexpr int Max_ele_size = 1024;
  static constexpr int Max_ee_size = 512;

  // =====================================================
  // TOKENS - Input collections
  // =====================================================
  // Common
  edm::EDGetTokenT<reco::VertexCollection> pvToken_;
  edm::EDGetTokenT<reco::BeamSpot> beamSpotToken_;
  edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken_;
  edm::EDGetTokenT<reco::Centrality> centralityToken_;
  edm::EDGetTokenT<int> centralityBinToken_;
  edm::EDGetTokenT<reco::EvtPlaneCollection> evtPlaneToken_;
  edm::EDGetTokenT<reco::GenParticleCollection> genParticleToken_;
  edm::EDGetTokenT<GenEventInfoProduct> genInfoToken_;

  // Muon-specific
  edm::EDGetTokenT<pat::MuonCollection> muonToken_;
  edm::EDGetTokenT<pat::MuonCollection> muonNoTrigToken_;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> dimuonToken_;

  // Electron-specific
  edm::EDGetTokenT<pat::ElectronCollection> electronToken_;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> dielectronToken_;
  edm::EDGetTokenT<reco::ConversionCollection> conversionToken_;

  // =====================================================
  // CONFIGURATION PARAMETERS
  // =====================================================
  // Common
  bool isHI_;
  bool isMC_;
  bool fillTree_;
  bool fillHistos_;
  bool useEvtPlane_;
  bool checkTrigNames_;
  std::string mom4format_;

  // Muon-specific
  bool fillMuons_;
  bool fillSingleMuons_;
  std::string muonSel_;
  std::vector<std::string> dblMuonTriggerPathNames_;
  std::vector<std::string> sglMuonTriggerPathNames_;

  // Electron-specific
  bool fillElectrons_;
  bool storeGenInfo_;
  std::vector<std::string> electronTriggerPathNames_;

  // =====================================================
  // HLT/TRIGGER
  // =====================================================
  HLTConfigProvider hltConfig_;
  bool hltConfigInit_;
  HLTPrescaleProvider hltPrescaleProvider_;
  bool hltPrescaleInit_;

  std::map<std::string, std::string> muonTriggerNameMap_;
  std::map<std::string, std::string> muonFilterNameMap_;
  std::set<std::string> unresolvedPaths_;
  std::vector<unsigned int> electronTriggerIndices_;
  std::vector<bool> electronTriggerAccepts_;

  // =====================================================
  // OUTPUT
  // =====================================================
  edm::Service<TFileService> fs_;
  TTree* tree_;

  // Event-level variables (shared)
  UInt_t runNb_;
  UInt_t lumiSection_;
  ULong64_t eventNb_;
  Short_t nPV_;
  Float_t zVtx_;
  Int_t centBin_;
  Float_t SumET_HF_;
  Int_t nEP_;
  Float_t rpAng_[50];
  Float_t rpSin_[50];
  Float_t rpCos_[50];
  
  math::XYZPoint RefVtx_;
  Float_t RefVtx_xError_;
  Float_t RefVtx_yError_;
  Float_t RefVtx_zError_;

  // Trigger bits
  ULong64_t HLTriggers_;  // Muon triggers
  ULong64_t eleTriggerBits_;  // Electron triggers

  // Gen info
  Float_t Gen_weight_;
  Float_t Gen_pthat_;

  // =====================================================
  // MUON BRANCHES
  // =====================================================
  static const unsigned int sNTRIGGERS = 65;
  unsigned int NTRIGGERS_;
  unsigned int NTRIGGERS_DBL_;
  bool isTriggerMatched_[sNTRIGGERS];
  int trigPrescale_[sNTRIGGERS];

  // Reco muons
  Short_t Reco_mu_size_;
  TClonesArray* Reco_mu_4mom_;
  std::vector<float> Reco_mu_4mom_pt_;
  std::vector<float> Reco_mu_4mom_eta_;
  std::vector<float> Reco_mu_4mom_phi_;
  std::vector<float> Reco_mu_4mom_m_;
  Short_t Reco_mu_charge_[Max_mu_size];
  Short_t Reco_mu_type_[Max_mu_size];
  ULong64_t Reco_mu_trig_[Max_mu_size];
  bool Reco_mu_highPurity_[Max_mu_size];
  bool Reco_mu_isPF_[Max_mu_size];
  bool Reco_mu_isTracker_[Max_mu_size];
  bool Reco_mu_isGlobal_[Max_mu_size];
  bool Reco_mu_isSoftCutBased_[Max_mu_size];
  bool Reco_mu_isHybridSoft_[Max_mu_size];
  float Reco_mu_softMvaRun3Value_[Max_mu_size];
  int Reco_mu_nPixValHits_[Max_mu_size];
  int Reco_mu_nMuValHits_[Max_mu_size];
  int Reco_mu_nTrkHits_[Max_mu_size];
  int Reco_mu_nPixWMea_[Max_mu_size];
  int Reco_mu_nTrkWMea_[Max_mu_size];
  float Reco_mu_normChi2_inner_[Max_mu_size];
  float Reco_mu_dxy_[Max_mu_size];
  float Reco_mu_dxyErr_[Max_mu_size];
  float Reco_mu_dz_[Max_mu_size];
  float Reco_mu_dzErr_[Max_mu_size];
  Short_t Reco_mu_whichGen_[Max_mu_size];

  // Reco dimuons (QQ)
  Short_t Reco_QQ_size_;
  TClonesArray* Reco_QQ_4mom_;
  std::vector<float> Reco_QQ_4mom_pt_;
  std::vector<float> Reco_QQ_4mom_eta_;
  std::vector<float> Reco_QQ_4mom_phi_;
  std::vector<float> Reco_QQ_4mom_m_;
  Short_t Reco_QQ_type_[Max_QQ_size];
  Short_t Reco_QQ_sign_[Max_QQ_size];
  Short_t Reco_QQ_mupl_idx_[Max_QQ_size];
  Short_t Reco_QQ_mumi_idx_[Max_QQ_size];
  ULong64_t Reco_QQ_trig_[Max_QQ_size];
  Float_t Reco_QQ_VtxProb_[Max_QQ_size];
  Float_t Reco_QQ_ctau_[Max_QQ_size];
  Float_t Reco_QQ_ctauErr_[Max_QQ_size];
  Float_t Reco_QQ_cosAlpha_[Max_QQ_size];
  Float_t Reco_QQ_ctau3D_[Max_QQ_size];
  Float_t Reco_QQ_ctauErr3D_[Max_QQ_size];
  Float_t Reco_QQ_cosAlpha3D_[Max_QQ_size];
  Float_t Reco_QQ_dca_[Max_QQ_size];
  Short_t Reco_QQ_whichGen_[Max_QQ_size];

  // Gen muons
  Short_t Gen_mu_size_;
  TClonesArray* Gen_mu_4mom_;
  std::vector<float> Gen_mu_4mom_pt_;
  std::vector<float> Gen_mu_4mom_eta_;
  std::vector<float> Gen_mu_4mom_phi_;
  std::vector<float> Gen_mu_4mom_m_;
  Short_t Gen_mu_charge_[Max_mu_size];
  Short_t Gen_mu_whichRec_[Max_mu_size];

  // Gen dimuons (QQ)
  Short_t Gen_QQ_size_;
  TClonesArray* Gen_QQ_4mom_;
  std::vector<float> Gen_QQ_4mom_pt_;
  std::vector<float> Gen_QQ_4mom_eta_;
  std::vector<float> Gen_QQ_4mom_phi_;
  std::vector<float> Gen_QQ_4mom_m_;
  Float_t Gen_QQ_ctau_[Max_QQ_size];
  Float_t Gen_QQ_ctau3D_[Max_QQ_size];
  Short_t Gen_QQ_mupl_idx_[Max_QQ_size];
  Short_t Gen_QQ_mumi_idx_[Max_QQ_size];
  Short_t Gen_QQ_whichRec_[Max_QQ_size];

  // =====================================================
  // ELECTRON BRANCHES
  // =====================================================
  // Reco electrons
  Int_t Reco_ele_size_;
  std::vector<float> Reco_ele_pt_;
  std::vector<float> Reco_ele_eta_;
  std::vector<float> Reco_ele_phi_;
  std::vector<float> Reco_ele_y_;
  std::vector<float> Reco_ele_mass_;
  Short_t Reco_ele_charge_[Max_ele_size];
  Short_t Reco_ele_isEB_[Max_ele_size];
  Float_t Reco_ele_ptErr_[Max_ele_size];
  Float_t Reco_ele_energy_[Max_ele_size];
  Float_t Reco_ele_corrEnergy_[Max_ele_size];
  Float_t Reco_ele_EcalEnergy_[Max_ele_size];
  Float_t Reco_ele_trackPt_[Max_ele_size];
  Float_t Reco_ele_trackEta_[Max_ele_size];
  Float_t Reco_ele_trackPhi_[Max_ele_size];
  Float_t Reco_ele_trackNormalizedChi2_[Max_ele_size];
  Short_t Reco_ele_nValidHits_[Max_ele_size];
  Short_t Reco_ele_nMissingHits_[Max_ele_size];
  Short_t Reco_ele_trackerLayers_[Max_ele_size];
  Short_t Reco_ele_pixelLayers_[Max_ele_size];
  Float_t Reco_ele_dxy_[Max_ele_size];
  Float_t Reco_ele_dxyErr_[Max_ele_size];
  Float_t Reco_ele_dz_[Max_ele_size];
  Float_t Reco_ele_dzErr_[Max_ele_size];
  Float_t Reco_ele_scEta_[Max_ele_size];
  Float_t Reco_ele_scPhi_[Max_ele_size];
  Float_t Reco_ele_scEn_[Max_ele_size];
  Float_t Reco_ele_r9_[Max_ele_size];
  Float_t Reco_ele_sigmaIEtaIEta_[Max_ele_size];
  Float_t Reco_ele_hOverE_[Max_ele_size];
  Float_t Reco_ele_fbrem_[Max_ele_size];
  Float_t Reco_ele_eOverP_[Max_ele_size];
  Float_t Reco_ele_deltaEtaIn_[Max_ele_size];
  Float_t Reco_ele_deltaPhiIn_[Max_ele_size];
  Float_t Reco_ele_pfChIso_[Max_ele_size];
  Float_t Reco_ele_pfNeuIso_[Max_ele_size];
  Float_t Reco_ele_pfPhoIso_[Max_ele_size];
  Short_t Reco_ele_convVeto_[Max_ele_size];
  ULong64_t Reco_ele_trigBits_[Max_ele_size];
  
  // MVA-based ID and Isolation
  Float_t Reco_ele_MVAIso_[Max_ele_size];
  Float_t Reco_ele_MVAId_[Max_ele_size];
  Short_t Reco_ele_MVAIsoWP90_[Max_ele_size];
  Short_t Reco_ele_MVAIsoWP80_[Max_ele_size];
  Short_t Reco_ele_MVAIdWP90_[Max_ele_size];
  Short_t Reco_ele_MVAIdWP80_[Max_ele_size];
  Short_t Reco_ele_CutIdWP90_[Max_ele_size];
  Short_t Reco_ele_CutIdWP80_[Max_ele_size];

  // Reco dielectrons (ee)
  Int_t Reco_ee_size_;
  std::vector<float> Reco_ee_pt_;
  std::vector<float> Reco_ee_eta_;
  std::vector<float> Reco_ee_phi_;
  std::vector<float> Reco_ee_y_;
  std::vector<float> Reco_ee_mass_;
  Float_t Reco_ee_vProb_[Max_ee_size];
  Float_t Reco_ee_chi2_[Max_ee_size];
  Short_t Reco_ee_charge_[Max_ee_size];
  Short_t Reco_ee_ele1Idx_[Max_ee_size];
  Short_t Reco_ee_ele2Idx_[Max_ee_size];
  ULong64_t Reco_ee_trigBits_[Max_ee_size];
  Short_t Reco_ee_isGenMatched_[Max_ee_size];
  Short_t Reco_ee_matchedGenIdx_[Max_ee_size];

  // Gen electrons
  Int_t Gen_ele_size_;
  std::vector<float> Gen_ele_pt_;
  std::vector<float> Gen_ele_eta_;
  std::vector<float> Gen_ele_phi_;
  std::vector<float> Gen_ele_y_;
  std::vector<float> Gen_ele_mass_;
  Int_t Gen_ele_pdgId_[Max_ele_size];
  Int_t Gen_ele_motherId_[Max_ele_size];

  // Gen dielectrons
  Int_t Gen_ee_size_;
  std::vector<float> Gen_ee_pt_;
  std::vector<float> Gen_ee_eta_;
  std::vector<float> Gen_ee_phi_;
  std::vector<float> Gen_ee_y_;
  std::vector<float> Gen_ee_mass_;
  Short_t Gen_ee_charge_[Max_ee_size];
  Short_t Gen_ee_ele1Idx_[Max_ee_size];
  Short_t Gen_ee_ele2Idx_[Max_ee_size];

  // =====================================================
  // INTERNAL BOOKKEEPING
  // =====================================================
  std::map<long int, int> mapMuonMomToIndex_;
  std::map<long int, int> mapGenMuonMomToIndex_;
  std::vector<const pat::CompositeCandidate*> thePassedMuonCands_;
  std::vector<unsigned int> thePassedMuonCats_;

  unsigned int nEvents_;
};

#endif
