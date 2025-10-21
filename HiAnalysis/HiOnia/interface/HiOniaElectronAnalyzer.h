#ifndef HIANALYSIS_HIONIA_HIONIAELECTRONANALYZER_H
#define HIANALYSIS_HIONIA_HIONIAELECTRONANALYZER_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <TLorentzVector.h>
#include <TTree.h>

#include "CommonTools/Egamma/interface/ConversionTools.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/Candidate/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/HeavyIonEvent/interface/Centrality.h"
#include "DataFormats/HeavyIonEvent/interface/EvtPlane.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
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

class HiOniaElectronAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources, edm::one::WatchRuns> {
public:
  explicit HiOniaElectronAnalyzer(const edm::ParameterSet&);
  ~HiOniaElectronAnalyzer() override;

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;
  void beginRun(const edm::Run&, const edm::EventSetup&) override;
  void endRun(const edm::Run&, const edm::EventSetup&) override;

  void initTree();
  void initEvent();

  void fillEventInfo(const edm::Event&);
  void fillPrimaryVertexInfo(const edm::Event&);
  void fillCentralityInfo(const edm::Event&);
  void fillTriggerInfo(const edm::Event&);
  void fillRecoElectrons(const edm::Event&);
  void fillRecoDielectrons(const edm::Event&);
  void fillGeneratorInfo(const edm::Event&);

  std::string resolveTriggerName(const std::string& requested, const edm::TriggerNames&) const;
  int findElectronIndex(const TLorentzVector& candP4) const;
  std::string sanitizeLabel(const std::string& raw) const;

  static constexpr int Max_ele_size = 1024;
  static constexpr int Max_ee_size = 512;

  // Tokens
  edm::EDGetTokenT<pat::ElectronCollection> electronToken_;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> dielectronToken_;
  edm::EDGetTokenT<reco::VertexCollection> pvToken_;
  edm::EDGetTokenT<reco::BeamSpot> beamSpotToken_;
  edm::EDGetTokenT<reco::ConversionCollection> conversionToken_;
  edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken_;
  edm::EDGetTokenT<reco::Centrality> centralityToken_;
  edm::EDGetTokenT<int> centralityBinToken_;
  edm::EDGetTokenT<reco::EvtPlaneCollection> evtPlaneToken_;
  edm::EDGetTokenT<reco::GenParticleCollection> genParticleToken_;

  // Configuration parameters
  std::vector<std::string> triggerPathNames_;
  std::vector<std::string> triggerLabels_;
  bool checkTriggerNames_;
  bool storeGenInfo_;
  bool isHI_;
  bool isMC_;
  bool useEvtPlane_;
  bool fillTree_;
  bool fillHistos_;

  // Trigger bookkeeping
  HLTConfigProvider hltConfig_;
  bool hltConfigInit_;
  ULong64_t eventTriggerBits_;
  std::set<std::string> unresolvedPaths_;

  // Services
  edm::Service<TFileService> fs_;

  // Output tree
  TTree* tree_;

  // Event-level variables
  UInt_t runNum_;
  UInt_t lumiSec_;
  ULong64_t eventNum_;
  Int_t nPrimaryVertices_;
  Float_t pvX_;
  Float_t pvY_;
  Float_t pvZ_;
  bool hasPrimaryVertex_;
  math::XYZPoint primaryVertexPoint_;
  Float_t centralityValue_;
  Int_t centralityBin_;
  Float_t evtPlaneAngle_;

  // Trigger
  ULong64_t triggerBits_;

  // Electron collections
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
  Float_t Reco_ele_corrEnergyErr_[Max_ele_size];
  Float_t Reco_ele_EcalEnergy_[Max_ele_size];
  Float_t Reco_ele_trackPt_[Max_ele_size];
  Float_t Reco_ele_trackEta_[Max_ele_size];
  Float_t Reco_ele_trackPhi_[Max_ele_size];
  Float_t Reco_ele_trackChi2_[Max_ele_size];
  Float_t Reco_ele_trackNdof_[Max_ele_size];
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
  Float_t Reco_ele_scRawEn_[Max_ele_size];
  Float_t Reco_ele_r9_[Max_ele_size];
  Float_t Reco_ele_sigmaIEtaIEta_[Max_ele_size];
  Float_t Reco_ele_sigmaIPhiIPhi_[Max_ele_size];
  Float_t Reco_ele_hOverE_[Max_ele_size];
  Float_t Reco_ele_fbrem_[Max_ele_size];
  Float_t Reco_ele_eOverP_[Max_ele_size];
  Float_t Reco_ele_eOverPInv_[Max_ele_size];
  Float_t Reco_ele_deltaEtaIn_[Max_ele_size];
  Float_t Reco_ele_deltaPhiIn_[Max_ele_size];
  Float_t Reco_ele_sigmaEtaEta_[Max_ele_size];
  Float_t Reco_ele_sigmaPhiPhi_[Max_ele_size];
  Float_t Reco_ele_pfChIso_[Max_ele_size];
  Float_t Reco_ele_pfNeuIso_[Max_ele_size];
  Float_t Reco_ele_pfPhoIso_[Max_ele_size];
  Float_t Reco_ele_pfPUIso_[Max_ele_size];
  Short_t Reco_ele_convVeto_[Max_ele_size];
  ULong64_t Reco_ele_trigBits_[Max_ele_size];

  // Dielectron collections
  Int_t Reco_ee_size_;
  std::vector<float> Reco_ee_pt_;
  std::vector<float> Reco_ee_eta_;
  std::vector<float> Reco_ee_phi_;
  std::vector<float> Reco_ee_y_;
  std::vector<float> Reco_ee_mass_;
  Float_t Reco_ee_vProb_[Max_ee_size];
  Float_t Reco_ee_chi2_[Max_ee_size];
  Float_t Reco_ee_ndf_[Max_ee_size];
  Short_t Reco_ee_charge_[Max_ee_size];
  Short_t Reco_ee_ele1Idx_[Max_ee_size];
  Short_t Reco_ee_ele2Idx_[Max_ee_size];
  ULong64_t Reco_ee_trigBits_[Max_ee_size];

  // Generator info (if requested)
  Int_t Gen_ele_size_;
  std::vector<float> Gen_ele_pt_;
  std::vector<float> Gen_ele_eta_;
  std::vector<float> Gen_ele_phi_;
  std::vector<float> Gen_ele_y_;
  std::vector<float> Gen_ele_mass_;
  Int_t Gen_ele_pdgId_[Max_ele_size];
  Int_t Gen_ele_motherId_[Max_ele_size];

  // Internal caches for trigger decisions
  std::vector<unsigned int> triggerIndices_;
  std::vector<bool> triggerAccepts_;
};

#endif
