// Combined Analyzer for J/psi -> mu+mu- and J/psi -> e+e-
// Produces a single tree with both muon and electron branches

#include "HiAnalysis/HiOnia/interface/HiOniaCombinedAnalyzer.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"

#include <cctype>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <limits>

namespace {
  constexpr float kInvalidFloat = -9999.f;
  constexpr short kInvalidShort = -1;
}

HiOniaCombinedAnalyzer::HiOniaCombinedAnalyzer(const edm::ParameterSet& iConfig)
    : pvToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("primaryVertexTag"))),
      beamSpotToken_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotTag"))),
      triggerResultsToken_(consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("triggerResults"))),
      centralityToken_(),
      centralityBinToken_(),
      evtPlaneToken_(),
      genParticleToken_(),
      genInfoToken_(),
      muonToken_(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("srcMuon"))),
      muonNoTrigToken_(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("srcMuonNoTrig"))),
      dimuonToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("srcDimuon"))),
      electronToken_(consumes<pat::ElectronCollection>(iConfig.getParameter<edm::InputTag>("srcElectron"))),
      dielectronToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("srcDielectron"))),
      conversionToken_(consumes<reco::ConversionCollection>(iConfig.getParameter<edm::InputTag>("conversions"))),
      isHI_(iConfig.getUntrackedParameter<bool>("isHI", true)),
      isMC_(iConfig.getUntrackedParameter<bool>("isMC", false)),
      fillTree_(iConfig.getParameter<bool>("fillTree")),
      fillHistos_(iConfig.getParameter<bool>("fillHistos")),
      useEvtPlane_(iConfig.getUntrackedParameter<bool>("useEvtPlane", false)),
      checkTrigNames_(iConfig.getParameter<bool>("checkTrigNames")),
      mom4format_(iConfig.getParameter<std::string>("mom4format")),
      fillMuons_(iConfig.getParameter<bool>("fillMuons")),
      fillSingleMuons_(iConfig.getParameter<bool>("fillSingleMuons")),
      muonSel_(iConfig.getParameter<std::string>("muonSel")),
      dblMuonTriggerPathNames_(iConfig.getParameter<std::vector<std::string>>("dblMuonTriggerPathNames")),
      sglMuonTriggerPathNames_(iConfig.getParameter<std::vector<std::string>>("sglMuonTriggerPathNames")),
      fillElectrons_(iConfig.getParameter<bool>("fillElectrons")),
      storeGenInfo_(iConfig.getParameter<bool>("storeGenInfo")),
      electronTriggerPathNames_(iConfig.getParameter<std::vector<std::string>>("electronTriggerPathNames")),
      hltConfig_(),
      hltConfigInit_(false),
      hltPrescaleProvider_(iConfig, consumesCollector(), *this),
      hltPrescaleInit_(false),
      tree_(nullptr),
      Reco_mu_4mom_(nullptr),
      Reco_QQ_4mom_(nullptr),
      Gen_mu_4mom_(nullptr),
      Gen_QQ_4mom_(nullptr),
      nEvents_(0) {

  usesResource(TFileService::kSharedResource);

  // Centrality tokens
  if (iConfig.existsAs<edm::InputTag>("CentralitySrc")) {
    const auto centTag = iConfig.getParameter<edm::InputTag>("CentralitySrc");
    if (!centTag.label().empty()) {
      centralityToken_ = consumes<reco::Centrality>(centTag);
    }
  }
  if (iConfig.existsAs<edm::InputTag>("CentralityBinSrc")) {
    const auto centBinTag = iConfig.getParameter<edm::InputTag>("CentralityBinSrc");
    if (!centBinTag.label().empty()) {
      centralityBinToken_ = consumes<int>(centBinTag);
    }
  }
  if (useEvtPlane_ && iConfig.existsAs<edm::InputTag>("EvtPlane")) {
    const auto evtPlaneTag = iConfig.getParameter<edm::InputTag>("EvtPlane");
    if (!evtPlaneTag.label().empty()) {
      evtPlaneToken_ = consumes<reco::EvtPlaneCollection>(evtPlaneTag);
    }
  }
  if (isMC_) {
    if (iConfig.existsAs<edm::InputTag>("genParticles")) {
      const auto genTag = iConfig.getParameter<edm::InputTag>("genParticles");
      if (!genTag.label().empty()) {
        genParticleToken_ = consumes<reco::GenParticleCollection>(genTag);
      }
    }
    genInfoToken_ = consumes<GenEventInfoProduct>(edm::InputTag("generator"));
  }

  // Setup trigger indices
  NTRIGGERS_DBL_ = dblMuonTriggerPathNames_.size();
  NTRIGGERS_ = NTRIGGERS_DBL_ + sglMuonTriggerPathNames_.size() + 1;  // +1 for NoTrigger

  electronTriggerIndices_.resize(electronTriggerPathNames_.size(), std::numeric_limits<unsigned int>::max());
  electronTriggerAccepts_.resize(electronTriggerPathNames_.size(), false);

  std::cout << "HiOniaCombinedAnalyzer initialized:" << std::endl;
  std::cout << "  Fill Muons: " << fillMuons_ << std::endl;
  std::cout << "  Fill Electrons: " << fillElectrons_ << std::endl;
  std::cout << "  Muon triggers (double): " << NTRIGGERS_DBL_ << std::endl;
  std::cout << "  Muon triggers (single): " << sglMuonTriggerPathNames_.size() << std::endl;
  std::cout << "  Electron triggers: " << electronTriggerPathNames_.size() << std::endl;
}

HiOniaCombinedAnalyzer::~HiOniaCombinedAnalyzer() {
  if (Reco_mu_4mom_) delete Reco_mu_4mom_;
  if (Reco_QQ_4mom_) delete Reco_QQ_4mom_;
  if (Gen_mu_4mom_) delete Gen_mu_4mom_;
  if (Gen_QQ_4mom_) delete Gen_QQ_4mom_;
}

void HiOniaCombinedAnalyzer::beginJob() {
  InitTree();
}

void HiOniaCombinedAnalyzer::endJob() {
  std::cout << "HiOniaCombinedAnalyzer: Total events processed = " << nEvents_ << std::endl;
}

void HiOniaCombinedAnalyzer::beginRun(const edm::Run& run, const edm::EventSetup& setup) {
  // Initialize HLT config
  EDConsumerBase::Labels labelTriggerResults;
  EDConsumerBase::labelsForToken(triggerResultsToken_, labelTriggerResults);
  const std::string pro = labelTriggerResults.process;
  std::cout << "HiOniaCombinedAnalyzer: Setting Trigger Result Process: " << pro << std::endl;

  bool changed = true;
  if (hltConfig_.init(run, setup, pro, changed)) {
    hltConfigInit_ = true;
  } else {
    edm::LogWarning("HiOniaCombinedAnalyzer") << "Failed to initialize HLTConfigProvider";
  }

  changed = true;
  hltPrescaleInit_ = hltPrescaleProvider_.init(run, setup, pro, changed);

  // Build muon trigger name -> filter maps
  for (const auto& pathLabel : dblMuonTriggerPathNames_)
    muonTriggerNameMap_[pathLabel] = "";
  for (const auto& pathLabel : sglMuonTriggerPathNames_)
    muonTriggerNameMap_[pathLabel] = "";

  for (const auto& hltPath : hltConfig_.triggerNames()) {
    if (hltPath.rfind("HLT_", 0) == 0) {
      for (auto& p : muonTriggerNameMap_) {
        if (TString(hltPath).Contains(TRegexp(TString(p.first)))) {
          p.second = hltPath;
          break;
        }
      }
    }
  }

  // Extract last filter names for muon triggers
  for (const auto& p : muonTriggerNameMap_) {
    muonFilterNameMap_[p.first] = "";
    if (p.second.empty()) continue;
    const auto& m = hltConfig_.moduleLabels(hltConfig_.triggerIndex(p.second));
    for (int j = m.size() - 1; j >= 0; j--) {
      if ((m[j].rfind("hltL", 0) == 0 && m[j].rfind("Filtered") != std::string::npos) ||
          (m[j].rfind("hltL1s", 0) == 0)) {
        muonFilterNameMap_.at(p.first) = m[j];
        break;
      }
    }
  }
}

void HiOniaCombinedAnalyzer::endRun(const edm::Run&, const edm::EventSetup&) {}

void HiOniaCombinedAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup&) {
  InitEvent();
  nEvents_++;

  fillEventInfo(iEvent);
  fillPrimaryVertexInfo(iEvent);
  fillCentralityInfo(iEvent);
  fillTriggerInfo(iEvent);

  // Fill muon analysis
  if (fillMuons_) {
    fillRecoMuons(iEvent);
    fillRecoDimuons(iEvent);
  }

  // Fill electron analysis
  if (fillElectrons_) {
    fillRecoElectrons(iEvent);
    fillRecoDielectrons(iEvent);
  }

  // Fill generator info
  if (isMC_) {
    edm::Handle<GenEventInfoProduct> genInfo;
    iEvent.getByToken(genInfoToken_, genInfo);
    if (genInfo.isValid()) {
      Gen_weight_ = genInfo->weight();
      if (genInfo->hasBinningValues()) {
        Gen_pthat_ = genInfo->binningValues()[0];
      }
    }

    if (fillMuons_) fillGenMuons(iEvent);
    if (fillElectrons_ && storeGenInfo_) fillGenElectrons(iEvent);
  }

  if (fillTree_ && tree_)
    tree_->Fill();
}

void HiOniaCombinedAnalyzer::InitTree() {
  // Initialize TClonesArrays
  Reco_mu_4mom_ = new TClonesArray("TLorentzVector", Max_mu_size);
  Reco_QQ_4mom_ = new TClonesArray("TLorentzVector", Max_QQ_size);
  if (isMC_) {
    Gen_mu_4mom_ = new TClonesArray("TLorentzVector", 10);
    Gen_QQ_4mom_ = new TClonesArray("TLorentzVector", 10);
  }

  tree_ = fs_->make<TTree>("OniaTree", "Combined Onia Tree for Muons and Electrons");

  // Event info
  tree_->Branch("runNb", &runNb_, "runNb/i");
  tree_->Branch("LS", &lumiSection_, "LS/i");
  tree_->Branch("eventNb", &eventNb_, "eventNb/l");
  tree_->Branch("zVtx", &zVtx_, "zVtx/F");
  tree_->Branch("nPV", &nPV_, "nPV/S");

  if (isHI_) {
    tree_->Branch("Centrality", &centBin_, "Centrality/I");
    tree_->Branch("SumET_HF", &SumET_HF_, "SumET_HF/F");
  }

  // Trigger info
  tree_->Branch("HLTriggers", &HLTriggers_, "HLTriggers/l");
  tree_->Branch("eleTriggerBits", &eleTriggerBits_, "eleTriggerBits/l");

  if (useEvtPlane_) {
    tree_->Branch("nEP", &nEP_, "nEP/I");
    tree_->Branch("rpAng", rpAng_, "rpAng[nEP]/F");
  }

  // ===========================================
  // MUON BRANCHES
  // ===========================================
  if (fillMuons_) {
    // Reco muons
    tree_->Branch("Reco_mu_size", &Reco_mu_size_, "Reco_mu_size/S");
    if (mom4format_ == "vector") {
      tree_->Branch("Reco_mu_4mom_pt", &Reco_mu_4mom_pt_);
      tree_->Branch("Reco_mu_4mom_eta", &Reco_mu_4mom_eta_);
      tree_->Branch("Reco_mu_4mom_phi", &Reco_mu_4mom_phi_);
      tree_->Branch("Reco_mu_4mom_m", &Reco_mu_4mom_m_);
    } else {
      tree_->Branch("Reco_mu_4mom", "TClonesArray", &Reco_mu_4mom_, 32000, 0);
    }
    tree_->Branch("Reco_mu_charge", Reco_mu_charge_, "Reco_mu_charge[Reco_mu_size]/S");
    tree_->Branch("Reco_mu_type", Reco_mu_type_, "Reco_mu_type[Reco_mu_size]/S");
    tree_->Branch("Reco_mu_trig", Reco_mu_trig_, "Reco_mu_trig[Reco_mu_size]/l");
    tree_->Branch("Reco_mu_highPurity", Reco_mu_highPurity_, "Reco_mu_highPurity[Reco_mu_size]/O");
    tree_->Branch("Reco_mu_isPF", Reco_mu_isPF_, "Reco_mu_isPF[Reco_mu_size]/O");
    tree_->Branch("Reco_mu_isTracker", Reco_mu_isTracker_, "Reco_mu_isTracker[Reco_mu_size]/O");
    tree_->Branch("Reco_mu_isGlobal", Reco_mu_isGlobal_, "Reco_mu_isGlobal[Reco_mu_size]/O");
    tree_->Branch("Reco_mu_isSoftCutBased", Reco_mu_isSoftCutBased_, "Reco_mu_isSoftCutBased[Reco_mu_size]/O");
    tree_->Branch("Reco_mu_isHybridSoft", Reco_mu_isHybridSoft_, "Reco_mu_isHybridSoft[Reco_mu_size]/O");
    tree_->Branch("Reco_mu_softMvaRun3Value", Reco_mu_softMvaRun3Value_, "Reco_mu_softMvaRun3Value[Reco_mu_size]/F");
    tree_->Branch("Reco_mu_nPixValHits", Reco_mu_nPixValHits_, "Reco_mu_nPixValHits[Reco_mu_size]/I");
    tree_->Branch("Reco_mu_nMuValHits", Reco_mu_nMuValHits_, "Reco_mu_nMuValHits[Reco_mu_size]/I");
    tree_->Branch("Reco_mu_nTrkHits", Reco_mu_nTrkHits_, "Reco_mu_nTrkHits[Reco_mu_size]/I");
    tree_->Branch("Reco_mu_nPixWMea", Reco_mu_nPixWMea_, "Reco_mu_nPixWMea[Reco_mu_size]/I");
    tree_->Branch("Reco_mu_nTrkWMea", Reco_mu_nTrkWMea_, "Reco_mu_nTrkWMea[Reco_mu_size]/I");
    tree_->Branch("Reco_mu_normChi2_inner", Reco_mu_normChi2_inner_, "Reco_mu_normChi2_inner[Reco_mu_size]/F");
    tree_->Branch("Reco_mu_dxy", Reco_mu_dxy_, "Reco_mu_dxy[Reco_mu_size]/F");
    tree_->Branch("Reco_mu_dxyErr", Reco_mu_dxyErr_, "Reco_mu_dxyErr[Reco_mu_size]/F");
    tree_->Branch("Reco_mu_dz", Reco_mu_dz_, "Reco_mu_dz[Reco_mu_size]/F");
    tree_->Branch("Reco_mu_dzErr", Reco_mu_dzErr_, "Reco_mu_dzErr[Reco_mu_size]/F");

    // Reco dimuons
    tree_->Branch("Reco_QQ_size", &Reco_QQ_size_, "Reco_QQ_size/S");
    if (mom4format_ == "vector") {
      tree_->Branch("Reco_QQ_4mom_pt", &Reco_QQ_4mom_pt_);
      tree_->Branch("Reco_QQ_4mom_eta", &Reco_QQ_4mom_eta_);
      tree_->Branch("Reco_QQ_4mom_phi", &Reco_QQ_4mom_phi_);
      tree_->Branch("Reco_QQ_4mom_m", &Reco_QQ_4mom_m_);
    } else {
      tree_->Branch("Reco_QQ_4mom", "TClonesArray", &Reco_QQ_4mom_, 32000, 0);
    }
    tree_->Branch("Reco_QQ_type", Reco_QQ_type_, "Reco_QQ_type[Reco_QQ_size]/S");
    tree_->Branch("Reco_QQ_sign", Reco_QQ_sign_, "Reco_QQ_sign[Reco_QQ_size]/S");
    tree_->Branch("Reco_QQ_mupl_idx", Reco_QQ_mupl_idx_, "Reco_QQ_mupl_idx[Reco_QQ_size]/S");
    tree_->Branch("Reco_QQ_mumi_idx", Reco_QQ_mumi_idx_, "Reco_QQ_mumi_idx[Reco_QQ_size]/S");
    tree_->Branch("Reco_QQ_trig", Reco_QQ_trig_, "Reco_QQ_trig[Reco_QQ_size]/l");
    tree_->Branch("Reco_QQ_VtxProb", Reco_QQ_VtxProb_, "Reco_QQ_VtxProb[Reco_QQ_size]/F");
    tree_->Branch("Reco_QQ_ctau", Reco_QQ_ctau_, "Reco_QQ_ctau[Reco_QQ_size]/F");
    tree_->Branch("Reco_QQ_ctauErr", Reco_QQ_ctauErr_, "Reco_QQ_ctauErr[Reco_QQ_size]/F");
    tree_->Branch("Reco_QQ_cosAlpha", Reco_QQ_cosAlpha_, "Reco_QQ_cosAlpha[Reco_QQ_size]/F");
    tree_->Branch("Reco_QQ_ctau3D", Reco_QQ_ctau3D_, "Reco_QQ_ctau3D[Reco_QQ_size]/F");
    tree_->Branch("Reco_QQ_ctauErr3D", Reco_QQ_ctauErr3D_, "Reco_QQ_ctauErr3D[Reco_QQ_size]/F");
    tree_->Branch("Reco_QQ_cosAlpha3D", Reco_QQ_cosAlpha3D_, "Reco_QQ_cosAlpha3D[Reco_QQ_size]/F");
    tree_->Branch("Reco_QQ_dca", Reco_QQ_dca_, "Reco_QQ_dca[Reco_QQ_size]/F");

    // Gen muon branches (MC only)
    if (isMC_) {
      tree_->Branch("Reco_mu_whichGen", Reco_mu_whichGen_, "Reco_mu_whichGen[Reco_mu_size]/S");
      tree_->Branch("Reco_QQ_whichGen", Reco_QQ_whichGen_, "Reco_QQ_whichGen[Reco_QQ_size]/S");

      tree_->Branch("Gen_weight", &Gen_weight_, "Gen_weight/F");
      tree_->Branch("Gen_pthat", &Gen_pthat_, "Gen_pthat/F");

      tree_->Branch("Gen_mu_size", &Gen_mu_size_, "Gen_mu_size/S");
      if (mom4format_ == "vector") {
        tree_->Branch("Gen_mu_4mom_pt", &Gen_mu_4mom_pt_);
        tree_->Branch("Gen_mu_4mom_eta", &Gen_mu_4mom_eta_);
        tree_->Branch("Gen_mu_4mom_phi", &Gen_mu_4mom_phi_);
        tree_->Branch("Gen_mu_4mom_m", &Gen_mu_4mom_m_);
      } else {
        tree_->Branch("Gen_mu_4mom", "TClonesArray", &Gen_mu_4mom_, 32000, 0);
      }
      tree_->Branch("Gen_mu_charge", Gen_mu_charge_, "Gen_mu_charge[Gen_mu_size]/S");
      tree_->Branch("Gen_mu_whichRec", Gen_mu_whichRec_, "Gen_mu_whichRec[Gen_mu_size]/S");

      tree_->Branch("Gen_QQ_size", &Gen_QQ_size_, "Gen_QQ_size/S");
      if (mom4format_ == "vector") {
        tree_->Branch("Gen_QQ_4mom_pt", &Gen_QQ_4mom_pt_);
        tree_->Branch("Gen_QQ_4mom_eta", &Gen_QQ_4mom_eta_);
        tree_->Branch("Gen_QQ_4mom_phi", &Gen_QQ_4mom_phi_);
        tree_->Branch("Gen_QQ_4mom_m", &Gen_QQ_4mom_m_);
      } else {
        tree_->Branch("Gen_QQ_4mom", "TClonesArray", &Gen_QQ_4mom_, 32000, 0);
      }
      tree_->Branch("Gen_QQ_ctau", Gen_QQ_ctau_, "Gen_QQ_ctau[Gen_QQ_size]/F");
      tree_->Branch("Gen_QQ_ctau3D", Gen_QQ_ctau3D_, "Gen_QQ_ctau3D[Gen_QQ_size]/F");
      tree_->Branch("Gen_QQ_mupl_idx", Gen_QQ_mupl_idx_, "Gen_QQ_mupl_idx[Gen_QQ_size]/S");
      tree_->Branch("Gen_QQ_mumi_idx", Gen_QQ_mumi_idx_, "Gen_QQ_mumi_idx[Gen_QQ_size]/S");
      tree_->Branch("Gen_QQ_whichRec", Gen_QQ_whichRec_, "Gen_QQ_whichRec[Gen_QQ_size]/S");
    }
  }

  // ===========================================
  // ELECTRON BRANCHES
  // ===========================================
  if (fillElectrons_) {
    // Reco electrons
    tree_->Branch("Reco_ele_size", &Reco_ele_size_, "Reco_ele_size/I");
    tree_->Branch("Reco_ele_pt", &Reco_ele_pt_);
    tree_->Branch("Reco_ele_eta", &Reco_ele_eta_);
    tree_->Branch("Reco_ele_phi", &Reco_ele_phi_);
    tree_->Branch("Reco_ele_y", &Reco_ele_y_);
    tree_->Branch("Reco_ele_mass", &Reco_ele_mass_);
    tree_->Branch("Reco_ele_charge", Reco_ele_charge_, "Reco_ele_charge[Reco_ele_size]/S");
    tree_->Branch("Reco_ele_isEB", Reco_ele_isEB_, "Reco_ele_isEB[Reco_ele_size]/S");
    tree_->Branch("Reco_ele_ptErr", Reco_ele_ptErr_, "Reco_ele_ptErr[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_energy", Reco_ele_energy_, "Reco_ele_energy[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_corrEnergy", Reco_ele_corrEnergy_, "Reco_ele_corrEnergy[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_EcalEnergy", Reco_ele_EcalEnergy_, "Reco_ele_EcalEnergy[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_trackPt", Reco_ele_trackPt_, "Reco_ele_trackPt[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_trackEta", Reco_ele_trackEta_, "Reco_ele_trackEta[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_trackPhi", Reco_ele_trackPhi_, "Reco_ele_trackPhi[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_trackNormalizedChi2", Reco_ele_trackNormalizedChi2_, "Reco_ele_trackNormalizedChi2[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_nValidHits", Reco_ele_nValidHits_, "Reco_ele_nValidHits[Reco_ele_size]/S");
    tree_->Branch("Reco_ele_nMissingHits", Reco_ele_nMissingHits_, "Reco_ele_nMissingHits[Reco_ele_size]/S");
    tree_->Branch("Reco_ele_trackerLayers", Reco_ele_trackerLayers_, "Reco_ele_trackerLayers[Reco_ele_size]/S");
    tree_->Branch("Reco_ele_pixelLayers", Reco_ele_pixelLayers_, "Reco_ele_pixelLayers[Reco_ele_size]/S");
    tree_->Branch("Reco_ele_dxy", Reco_ele_dxy_, "Reco_ele_dxy[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_dxyErr", Reco_ele_dxyErr_, "Reco_ele_dxyErr[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_dz", Reco_ele_dz_, "Reco_ele_dz[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_dzErr", Reco_ele_dzErr_, "Reco_ele_dzErr[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_scEta", Reco_ele_scEta_, "Reco_ele_scEta[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_scPhi", Reco_ele_scPhi_, "Reco_ele_scPhi[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_scEn", Reco_ele_scEn_, "Reco_ele_scEn[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_r9", Reco_ele_r9_, "Reco_ele_r9[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_sigmaIEtaIEta", Reco_ele_sigmaIEtaIEta_, "Reco_ele_sigmaIEtaIEta[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_hOverE", Reco_ele_hOverE_, "Reco_ele_hOverE[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_fbrem", Reco_ele_fbrem_, "Reco_ele_fbrem[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_eOverP", Reco_ele_eOverP_, "Reco_ele_eOverP[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_deltaEtaIn", Reco_ele_deltaEtaIn_, "Reco_ele_deltaEtaIn[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_deltaPhiIn", Reco_ele_deltaPhiIn_, "Reco_ele_deltaPhiIn[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_pfChIso", Reco_ele_pfChIso_, "Reco_ele_pfChIso[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_pfNeuIso", Reco_ele_pfNeuIso_, "Reco_ele_pfNeuIso[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_pfPhoIso", Reco_ele_pfPhoIso_, "Reco_ele_pfPhoIso[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_convVeto", Reco_ele_convVeto_, "Reco_ele_convVeto[Reco_ele_size]/S");
    tree_->Branch("Reco_ele_trigBits", Reco_ele_trigBits_, "Reco_ele_trigBits[Reco_ele_size]/l");
    
    // MVA IDs
    tree_->Branch("Reco_ele_MVAIso", Reco_ele_MVAIso_, "Reco_ele_MVAIso[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_MVAId", Reco_ele_MVAId_, "Reco_ele_MVAId[Reco_ele_size]/F");
    tree_->Branch("Reco_ele_MVAIsoWP90", Reco_ele_MVAIsoWP90_, "Reco_ele_MVAIsoWP90[Reco_ele_size]/S");
    tree_->Branch("Reco_ele_MVAIsoWP80", Reco_ele_MVAIsoWP80_, "Reco_ele_MVAIsoWP80[Reco_ele_size]/S");
    tree_->Branch("Reco_ele_MVAIdWP90", Reco_ele_MVAIdWP90_, "Reco_ele_MVAIdWP90[Reco_ele_size]/S");
    tree_->Branch("Reco_ele_MVAIdWP80", Reco_ele_MVAIdWP80_, "Reco_ele_MVAIdWP80[Reco_ele_size]/S");
    tree_->Branch("Reco_ele_CutIdWP90", Reco_ele_CutIdWP90_, "Reco_ele_CutIdWP90[Reco_ele_size]/S");
    tree_->Branch("Reco_ele_CutIdWP80", Reco_ele_CutIdWP80_, "Reco_ele_CutIdWP80[Reco_ele_size]/S");

    // Reco dielectrons
    tree_->Branch("Reco_ee_size", &Reco_ee_size_, "Reco_ee_size/I");
    tree_->Branch("Reco_ee_pt", &Reco_ee_pt_);
    tree_->Branch("Reco_ee_eta", &Reco_ee_eta_);
    tree_->Branch("Reco_ee_phi", &Reco_ee_phi_);
    tree_->Branch("Reco_ee_y", &Reco_ee_y_);
    tree_->Branch("Reco_ee_mass", &Reco_ee_mass_);
    tree_->Branch("Reco_ee_vProb", Reco_ee_vProb_, "Reco_ee_vProb[Reco_ee_size]/F");
    tree_->Branch("Reco_ee_chi2", Reco_ee_chi2_, "Reco_ee_chi2[Reco_ee_size]/F");
    tree_->Branch("Reco_ee_charge", Reco_ee_charge_, "Reco_ee_charge[Reco_ee_size]/S");
    tree_->Branch("Reco_ee_ele1Idx", Reco_ee_ele1Idx_, "Reco_ee_ele1Idx[Reco_ee_size]/S");
    tree_->Branch("Reco_ee_ele2Idx", Reco_ee_ele2Idx_, "Reco_ee_ele2Idx[Reco_ee_size]/S");
    tree_->Branch("Reco_ee_trigBits", Reco_ee_trigBits_, "Reco_ee_trigBits[Reco_ee_size]/l");

    // Gen electron branches (MC only)
    if (isMC_ && storeGenInfo_) {
      tree_->Branch("Reco_ee_isGenMatched", Reco_ee_isGenMatched_, "Reco_ee_isGenMatched[Reco_ee_size]/S");
      tree_->Branch("Reco_ee_matchedGenIdx", Reco_ee_matchedGenIdx_, "Reco_ee_matchedGenIdx[Reco_ee_size]/S");

      tree_->Branch("Gen_ele_size", &Gen_ele_size_, "Gen_ele_size/I");
      tree_->Branch("Gen_ele_pt", &Gen_ele_pt_);
      tree_->Branch("Gen_ele_eta", &Gen_ele_eta_);
      tree_->Branch("Gen_ele_phi", &Gen_ele_phi_);
      tree_->Branch("Gen_ele_y", &Gen_ele_y_);
      tree_->Branch("Gen_ele_mass", &Gen_ele_mass_);
      tree_->Branch("Gen_ele_pdgId", Gen_ele_pdgId_, "Gen_ele_pdgId[Gen_ele_size]/I");
      tree_->Branch("Gen_ele_motherId", Gen_ele_motherId_, "Gen_ele_motherId[Gen_ele_size]/I");

      tree_->Branch("Gen_ee_size", &Gen_ee_size_, "Gen_ee_size/I");
      tree_->Branch("Gen_ee_pt", &Gen_ee_pt_);
      tree_->Branch("Gen_ee_eta", &Gen_ee_eta_);
      tree_->Branch("Gen_ee_phi", &Gen_ee_phi_);
      tree_->Branch("Gen_ee_y", &Gen_ee_y_);
      tree_->Branch("Gen_ee_mass", &Gen_ee_mass_);
      tree_->Branch("Gen_ee_charge", Gen_ee_charge_, "Gen_ee_charge[Gen_ee_size]/S");
      tree_->Branch("Gen_ee_ele1Idx", Gen_ee_ele1Idx_, "Gen_ee_ele1Idx[Gen_ee_size]/S");
      tree_->Branch("Gen_ee_ele2Idx", Gen_ee_ele2Idx_, "Gen_ee_ele2Idx[Gen_ee_size]/S");
    }
  }
}

void HiOniaCombinedAnalyzer::InitEvent() {
  runNb_ = 0;
  lumiSection_ = 0;
  eventNb_ = 0;
  nPV_ = 0;
  zVtx_ = kInvalidFloat;
  centBin_ = -1;
  SumET_HF_ = kInvalidFloat;
  nEP_ = 0;
  HLTriggers_ = 0;
  eleTriggerBits_ = 0;
  Gen_weight_ = -1.f;
  Gen_pthat_ = -1.f;

  RefVtx_ = math::XYZPoint();
  RefVtx_xError_ = 0.f;
  RefVtx_yError_ = 0.f;
  RefVtx_zError_ = 0.f;

  // Muon clearing
  if (fillMuons_) {
    Reco_mu_size_ = 0;
    Reco_mu_4mom_->Clear();
    Reco_mu_4mom_pt_.clear();
    Reco_mu_4mom_eta_.clear();
    Reco_mu_4mom_phi_.clear();
    Reco_mu_4mom_m_.clear();

    Reco_QQ_size_ = 0;
    Reco_QQ_4mom_->Clear();
    Reco_QQ_4mom_pt_.clear();
    Reco_QQ_4mom_eta_.clear();
    Reco_QQ_4mom_phi_.clear();
    Reco_QQ_4mom_m_.clear();

    thePassedMuonCands_.clear();
    thePassedMuonCats_.clear();
    mapMuonMomToIndex_.clear();

    for (unsigned int iTr = 0; iTr < sNTRIGGERS; ++iTr) {
      isTriggerMatched_[iTr] = false;
      trigPrescale_[iTr] = -1;
    }

    if (isMC_) {
      Gen_mu_size_ = 0;
      Gen_mu_4mom_->Clear();
      Gen_mu_4mom_pt_.clear();
      Gen_mu_4mom_eta_.clear();
      Gen_mu_4mom_phi_.clear();
      Gen_mu_4mom_m_.clear();

      Gen_QQ_size_ = 0;
      Gen_QQ_4mom_->Clear();
      Gen_QQ_4mom_pt_.clear();
      Gen_QQ_4mom_eta_.clear();
      Gen_QQ_4mom_phi_.clear();
      Gen_QQ_4mom_m_.clear();

      mapGenMuonMomToIndex_.clear();
    }
  }

  // Electron clearing
  if (fillElectrons_) {
    Reco_ele_size_ = 0;
    Reco_ele_pt_.clear();
    Reco_ele_eta_.clear();
    Reco_ele_phi_.clear();
    Reco_ele_y_.clear();
    Reco_ele_mass_.clear();
    std::fill_n(Reco_ele_trigBits_, Max_ele_size, 0ULL);

    Reco_ee_size_ = 0;
    Reco_ee_pt_.clear();
    Reco_ee_eta_.clear();
    Reco_ee_phi_.clear();
    Reco_ee_y_.clear();
    Reco_ee_mass_.clear();
    std::fill_n(Reco_ee_trigBits_, Max_ee_size, 0ULL);
    std::fill_n(Reco_ee_isGenMatched_, Max_ee_size, static_cast<Short_t>(0));
    std::fill_n(Reco_ee_matchedGenIdx_, Max_ee_size, static_cast<Short_t>(-1));

    std::fill(electronTriggerAccepts_.begin(), electronTriggerAccepts_.end(), false);

    if (isMC_ && storeGenInfo_) {
      Gen_ele_size_ = 0;
      Gen_ele_pt_.clear();
      Gen_ele_eta_.clear();
      Gen_ele_phi_.clear();
      Gen_ele_y_.clear();
      Gen_ele_mass_.clear();

      Gen_ee_size_ = 0;
      Gen_ee_pt_.clear();
      Gen_ee_eta_.clear();
      Gen_ee_phi_.clear();
      Gen_ee_y_.clear();
      Gen_ee_mass_.clear();
    }
  }
}

void HiOniaCombinedAnalyzer::fillEventInfo(const edm::Event& iEvent) {
  runNb_ = iEvent.id().run();
  lumiSection_ = iEvent.luminosityBlock();
  eventNb_ = iEvent.id().event();
}

void HiOniaCombinedAnalyzer::fillPrimaryVertexInfo(const edm::Event& iEvent) {
  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(pvToken_, vertices);
  if (vertices.isValid() && !vertices->empty()) {
    const auto& pv = vertices->front();
    nPV_ = vertices->size();
    RefVtx_ = pv.position();
    RefVtx_xError_ = pv.xError();
    RefVtx_yError_ = pv.yError();
    RefVtx_zError_ = pv.zError();
    zVtx_ = pv.z();
  }
}

void HiOniaCombinedAnalyzer::fillCentralityInfo(const edm::Event& iEvent) {
  if (!centralityToken_.isUninitialized()) {
    edm::Handle<reco::Centrality> centrality;
    iEvent.getByToken(centralityToken_, centrality);
    if (centrality.isValid()) {
      SumET_HF_ = centrality->EtHFtowerSum();
    }
  }
  if (!centralityBinToken_.isUninitialized()) {
    edm::Handle<int> centBin;
    iEvent.getByToken(centralityBinToken_, centBin);
    if (centBin.isValid()) {
      centBin_ = *centBin;
    }
  }
  if (useEvtPlane_ && !evtPlaneToken_.isUninitialized()) {
    edm::Handle<reco::EvtPlaneCollection> evtPlanes;
    iEvent.getByToken(evtPlaneToken_, evtPlanes);
    if (evtPlanes.isValid()) {
      nEP_ = 0;
      for (reco::EvtPlaneCollection::const_iterator rp = evtPlanes->begin(); rp != evtPlanes->end() && nEP_ < 50; rp++) {
        rpAng_[nEP_] = rp->angle(2);
        rpSin_[nEP_] = rp->sumSin(2);
        rpCos_[nEP_] = rp->sumCos(2);
        nEP_++;
      }
    }
  }
}

void HiOniaCombinedAnalyzer::fillTriggerInfo(const edm::Event& iEvent) {
  edm::Handle<edm::TriggerResults> trigResults;
  iEvent.getByToken(triggerResultsToken_, trigResults);
  HLTriggers_ = 0;
  eleTriggerBits_ = 0;

  if (!trigResults.isValid()) return;

  const edm::TriggerNames& trigNames = iEvent.triggerNames(*trigResults);

  // Muon triggers
  if (fillMuons_) {
    unsigned int iTr = 1;
    for (const auto& pathName : dblMuonTriggerPathNames_) {
      const std::string resolved = resolveTriggerName(pathName, trigNames);
      if (!resolved.empty()) {
        const unsigned int index = trigNames.triggerIndex(resolved);
        if (index < trigResults->size() && trigResults->accept(index)) {
          HLTriggers_ |= (1ULL << (iTr - 1));
          isTriggerMatched_[iTr] = true;
        }
      }
      iTr++;
    }
    for (const auto& pathName : sglMuonTriggerPathNames_) {
      const std::string resolved = resolveTriggerName(pathName, trigNames);
      if (!resolved.empty()) {
        const unsigned int index = trigNames.triggerIndex(resolved);
        if (index < trigResults->size() && trigResults->accept(index)) {
          HLTriggers_ |= (1ULL << (iTr - 1));
          isTriggerMatched_[iTr] = true;
        }
      }
      iTr++;
    }
  }

  // Electron triggers
  if (fillElectrons_) {
    for (size_t i = 0; i < electronTriggerPathNames_.size(); ++i) {
      const std::string resolved = resolveTriggerName(electronTriggerPathNames_[i], trigNames);
      if (resolved.empty()) {
        electronTriggerIndices_[i] = std::numeric_limits<unsigned int>::max();
        electronTriggerAccepts_[i] = false;
        continue;
      }
      const unsigned int index = trigNames.triggerIndex(resolved);
      electronTriggerIndices_[i] = index;
      const bool accepted = (index < trigResults->size()) ? trigResults->accept(index) : false;
      electronTriggerAccepts_[i] = accepted;
      if (accepted && i < 64) {
        eleTriggerBits_ |= (1ULL << i);
      }
    }
  }
}

std::string HiOniaCombinedAnalyzer::resolveTriggerName(const std::string& requested,
                                                       const edm::TriggerNames& names) const {
  if (requested.empty()) return std::string();

  const auto& triggerNames = names.triggerNames();
  for (const auto& name : triggerNames) {
    if (name == requested) return name;
  }

  const auto pos = requested.find("_v");
  if (pos != std::string::npos) {
    const std::string prefix = requested.substr(0, pos);
    for (const auto& name : triggerNames) {
      if (name.compare(0, prefix.size(), prefix) == 0) return name;
    }
  }
  return std::string();
}

// =====================================================
// MUON METHODS
// =====================================================
void HiOniaCombinedAnalyzer::fillRecoMuons(const edm::Event& iEvent) {
  edm::Handle<pat::MuonCollection> muons;
  iEvent.getByToken(muonToken_, muons);
  if (!muons.isValid()) return;

  for (const auto& muon : *muons) {
    if (Reco_mu_size_ >= Max_mu_size) break;

    // Selection
    bool passSelection = false;
    Short_t muType = -1;
    if (muonSel_ == "GlbOrTrk" && selGlobalOrTrackerMuon(&muon)) {
      passSelection = true;
      muType = 3;  // GlbOrTrk
    } else if (muonSel_ == "GlbGlb" && selGlobalMuon(&muon)) {
      passSelection = true;
      muType = 2;  // Glb
    } else if (muonSel_ == "Trk" && selTrackerMuon(&muon)) {
      passSelection = true;
      muType = 1;  // Trk
    } else if (muonSel_ == "All") {
      passSelection = true;
      muType = 4;  // All
    }

    if (!passSelection) continue;

    const auto idx = Reco_mu_size_;
    TLorentzVector vMuon = lorentzMomentum(muon.p4());
    new ((*Reco_mu_4mom_)[idx]) TLorentzVector(vMuon);
    Reco_mu_4mom_pt_.push_back(vMuon.Pt());
    Reco_mu_4mom_eta_.push_back(vMuon.Eta());
    Reco_mu_4mom_phi_.push_back(vMuon.Phi());
    Reco_mu_4mom_m_.push_back(vMuon.M());

    mapMuonMomToIndex_[FloatToIntkey(vMuon.Pt())] = idx;

    Reco_mu_charge_[idx] = muon.charge();
    Reco_mu_type_[idx] = muType;
    Reco_mu_trig_[idx] = 0;

    // Fill muon quality variables
    Reco_mu_isPF_[idx] = muon.isPFMuon();
    Reco_mu_isTracker_[idx] = muon.isTrackerMuon();
    Reco_mu_isGlobal_[idx] = muon.isGlobalMuon();
    Reco_mu_isSoftCutBased_[idx] = muon.passed(reco::Muon::SoftCutBasedId);
    Reco_mu_softMvaRun3Value_[idx] = muon.softMvaRun3Value();
    Reco_mu_isHybridSoft_[idx] = isHybridSoftMuon(&muon);

    reco::TrackRef iTrack = muon.innerTrack();
    if (iTrack.isNonnull()) {
      Reco_mu_highPurity_[idx] = iTrack->quality(reco::TrackBase::highPurity);
      Reco_mu_nTrkHits_[idx] = iTrack->found();
      Reco_mu_normChi2_inner_[idx] = iTrack->normalizedChi2();
      Reco_mu_nPixValHits_[idx] = iTrack->hitPattern().numberOfValidPixelHits();
      Reco_mu_nPixWMea_[idx] = iTrack->hitPattern().pixelLayersWithMeasurement();
      Reco_mu_nTrkWMea_[idx] = iTrack->hitPattern().trackerLayersWithMeasurement();
      Reco_mu_dxy_[idx] = iTrack->dxy(RefVtx_);
      Reco_mu_dxyErr_[idx] = iTrack->dxyError();
      Reco_mu_dz_[idx] = iTrack->dz(RefVtx_);
      Reco_mu_dzErr_[idx] = iTrack->dzError();
    } else {
      Reco_mu_highPurity_[idx] = false;
      Reco_mu_nTrkHits_[idx] = -1;
      Reco_mu_normChi2_inner_[idx] = kInvalidFloat;
      Reco_mu_nPixValHits_[idx] = -1;
      Reco_mu_nPixWMea_[idx] = -1;
      Reco_mu_nTrkWMea_[idx] = -1;
      Reco_mu_dxy_[idx] = kInvalidFloat;
      Reco_mu_dxyErr_[idx] = kInvalidFloat;
      Reco_mu_dz_[idx] = kInvalidFloat;
      Reco_mu_dzErr_[idx] = kInvalidFloat;
    }

    if (muon.isGlobalMuon()) {
      reco::TrackRef gTrack = muon.globalTrack();
      Reco_mu_nMuValHits_[idx] = gTrack->hitPattern().numberOfValidMuonHits();
    } else {
      Reco_mu_nMuValHits_[idx] = -1;
    }

    Reco_mu_whichGen_[idx] = -1;
    Reco_mu_size_++;
  }
}

void HiOniaCombinedAnalyzer::fillRecoDimuons(const edm::Event& iEvent) {
  edm::Handle<pat::CompositeCandidateCollection> dimuons;
  iEvent.getByToken(dimuonToken_, dimuons);
  
  if (!dimuons.isValid()) {
    edm::LogWarning("HiOniaCombinedAnalyzer") << "Dimuon collection not valid!";
    return;
  }

  for (const auto& cand : *dimuons) {
    if (Reco_QQ_size_ >= Max_QQ_size) break;

    const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(cand.daughter("muon1"));
    const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(cand.daughter("muon2"));
    if (!muon1 || !muon2) {
      edm::LogWarning("HiOniaCombinedAnalyzer") << "Could not get muon daughters from dimuon candidate";
      continue;
    }

    // Check for PV userData (required by legacy analyzer)
    // The dimuon producer stores the PV info, if not present skip
    if (!cand.hasUserData("PVwithmuons") && !cand.hasUserData("muonlessPV")) {
      // This is expected if running without the full onia sequence
      // Just proceed without the PV check
    }

    // Apply muon eta cut (same as legacy: etaMax = 2.5)
    if (std::abs(muon1->eta()) >= 2.5 || std::abs(muon2->eta()) >= 2.5) {
      continue;
    }

    // Apply muon selection cuts based on muonSel_
    bool passSelection = false;
    Short_t qqType = 0;
    
    if (muonSel_ == "GlbOrTrk") {
      if (selGlobalOrTrackerMuon(muon1) && selGlobalOrTrackerMuon(muon2)) {
        passSelection = true;
        qqType = 3;  // GlbOrTrk_GlbOrTrk
      }
    } else if (muonSel_ == "GlbGlb" || muonSel_ == "Glb") {
      if (selGlobalMuon(muon1) && selGlobalMuon(muon2)) {
        passSelection = true;
        qqType = 1;  // Glb_Glb
      }
    } else if (muonSel_ == "GlbTrk") {
      if (selGlobalMuon(muon1) && selGlobalMuon(muon2)) {
        passSelection = true;
        qqType = 0;  // GlbTrk_GlbTrk
      }
    } else if (muonSel_ == "Trk") {
      if (selTrackerMuon(muon1) && selTrackerMuon(muon2)) {
        passSelection = true;
        qqType = 2;  // Trk_Trk
      }
    } else if (muonSel_ == "All") {
      passSelection = true;
      qqType = 5;  // All_All
    }

    if (!passSelection) {
      continue;
    }

    const auto idx = Reco_QQ_size_;
    TLorentzVector vJpsi = lorentzMomentum(cand.p4());
    new ((*Reco_QQ_4mom_)[idx]) TLorentzVector(vJpsi);
    Reco_QQ_4mom_pt_.push_back(vJpsi.Pt());
    Reco_QQ_4mom_eta_.push_back(vJpsi.Eta());
    Reco_QQ_4mom_phi_.push_back(vJpsi.Phi());
    Reco_QQ_4mom_m_.push_back(vJpsi.M());

    Reco_QQ_sign_[idx] = muon1->charge() + muon2->charge();
    Reco_QQ_type_[idx] = qqType;

    // Find muon indices
    TLorentzVector vMuon1 = lorentzMomentum(muon1->p4());
    TLorentzVector vMuon2 = lorentzMomentum(muon2->p4());
    if (muon1->charge() > muon2->charge()) {
      Reco_QQ_mupl_idx_[idx] = IndexOfThisMuon(&vMuon1);
      Reco_QQ_mumi_idx_[idx] = IndexOfThisMuon(&vMuon2);
    } else {
      Reco_QQ_mupl_idx_[idx] = IndexOfThisMuon(&vMuon2);
      Reco_QQ_mumi_idx_[idx] = IndexOfThisMuon(&vMuon1);
    }

    // Vertex and lifetime info
    Reco_QQ_VtxProb_[idx] = cand.hasUserFloat("vProb") ? cand.userFloat("vProb") : kInvalidFloat;
    Reco_QQ_ctau_[idx] = cand.hasUserFloat("ppdlPV") ? 10.0 * cand.userFloat("ppdlPV") : kInvalidFloat;
    Reco_QQ_ctauErr_[idx] = cand.hasUserFloat("ppdlErrPV") ? 10.0 * cand.userFloat("ppdlErrPV") : kInvalidFloat;
    Reco_QQ_cosAlpha_[idx] = cand.hasUserFloat("cosAlpha") ? cand.userFloat("cosAlpha") : kInvalidFloat;
    Reco_QQ_ctau3D_[idx] = cand.hasUserFloat("ppdlPV3D") ? 10.0 * cand.userFloat("ppdlPV3D") : kInvalidFloat;
    Reco_QQ_ctauErr3D_[idx] = cand.hasUserFloat("ppdlErrPV3D") ? 10.0 * cand.userFloat("ppdlErrPV3D") : kInvalidFloat;
    Reco_QQ_cosAlpha3D_[idx] = cand.hasUserFloat("cosAlpha3D") ? cand.userFloat("cosAlpha3D") : kInvalidFloat;
    Reco_QQ_dca_[idx] = cand.hasUserFloat("DCA") ? cand.userFloat("DCA") : kInvalidFloat;

    // Check trigger matching
    checkMuonTriggers(&cand);
    ULong64_t trigBits = 0;
    for (unsigned int iTr = 1; iTr < NTRIGGERS_; ++iTr) {
      if (isTriggerMatched_[iTr]) {
        trigBits |= (1ULL << (iTr - 1));
      }
    }
    Reco_QQ_trig_[idx] = trigBits;

    Reco_QQ_whichGen_[idx] = -1;
    Reco_QQ_size_++;
  }
}

void HiOniaCombinedAnalyzer::fillGenMuons(const edm::Event& iEvent) {
  if (genParticleToken_.isUninitialized()) return;

  edm::Handle<reco::GenParticleCollection> genParticles;
  iEvent.getByToken(genParticleToken_, genParticles);
  if (!genParticles.isValid()) return;

  // First pass: collect gen muons
  for (size_t i = 0; i < genParticles->size(); ++i) {
    const auto& gen = (*genParticles)[i];
    if (std::abs(gen.pdgId()) != 13) continue;
    if (Gen_mu_size_ >= Max_mu_size) break;

    const auto& p4 = gen.p4();
    TLorentzVector vMuon(p4.px(), p4.py(), p4.pz(), p4.energy());
    new ((*Gen_mu_4mom_)[Gen_mu_size_]) TLorentzVector(vMuon);
    Gen_mu_4mom_pt_.push_back(vMuon.Pt());
    Gen_mu_4mom_eta_.push_back(vMuon.Eta());
    Gen_mu_4mom_phi_.push_back(vMuon.Phi());
    Gen_mu_4mom_m_.push_back(vMuon.M());
    Gen_mu_charge_[Gen_mu_size_] = (gen.pdgId() == 13) ? -1 : 1;
    Gen_mu_whichRec_[Gen_mu_size_] = -1;

    mapGenMuonMomToIndex_[FloatToIntkey(vMuon.Pt())] = Gen_mu_size_;
    Gen_mu_size_++;
  }

  // Second pass: collect gen J/psi -> mu+mu-
  for (size_t i = 0; i < genParticles->size(); ++i) {
    const auto& gen = (*genParticles)[i];
    if (gen.pdgId() != 443) continue;  // J/psi
    if (Gen_QQ_size_ >= Max_QQ_size) break;

    // Find daughter muons
    Short_t mupl_idx = -1, mumi_idx = -1;
    for (size_t d = 0; d < gen.numberOfDaughters(); ++d) {
      const auto* dau = gen.daughter(d);
      if (dau->pdgId() == 13) {  // mu-
        TLorentzVector vMu(dau->px(), dau->py(), dau->pz(), dau->energy());
        auto it = mapGenMuonMomToIndex_.find(FloatToIntkey(vMu.Pt()));
        if (it != mapGenMuonMomToIndex_.end()) mumi_idx = it->second;
      } else if (dau->pdgId() == -13) {  // mu+
        TLorentzVector vMu(dau->px(), dau->py(), dau->pz(), dau->energy());
        auto it = mapGenMuonMomToIndex_.find(FloatToIntkey(vMu.Pt()));
        if (it != mapGenMuonMomToIndex_.end()) mupl_idx = it->second;
      }
    }

    const auto& p4 = gen.p4();
    TLorentzVector vJpsi(p4.px(), p4.py(), p4.pz(), p4.energy());
    new ((*Gen_QQ_4mom_)[Gen_QQ_size_]) TLorentzVector(vJpsi);
    Gen_QQ_4mom_pt_.push_back(vJpsi.Pt());
    Gen_QQ_4mom_eta_.push_back(vJpsi.Eta());
    Gen_QQ_4mom_phi_.push_back(vJpsi.Phi());
    Gen_QQ_4mom_m_.push_back(vJpsi.M());
    Gen_QQ_mupl_idx_[Gen_QQ_size_] = mupl_idx;
    Gen_QQ_mumi_idx_[Gen_QQ_size_] = mumi_idx;
    Gen_QQ_ctau_[Gen_QQ_size_] = kInvalidFloat;
    Gen_QQ_ctau3D_[Gen_QQ_size_] = kInvalidFloat;
    Gen_QQ_whichRec_[Gen_QQ_size_] = -1;

    Gen_QQ_size_++;
  }
}

int HiOniaCombinedAnalyzer::IndexOfThisMuon(TLorentzVector* v1, bool isGen) {
  const auto& theMap = isGen ? mapGenMuonMomToIndex_ : mapMuonMomToIndex_;
  auto it = theMap.find(FloatToIntkey(v1->Pt()));
  if (it != theMap.end()) return it->second;
  return -1;
}

bool HiOniaCombinedAnalyzer::selGlobalMuon(const pat::Muon* aMuon) {
  // Match legacy behavior: just check isGlobalMuon (no applyCuts)
  return aMuon->isGlobalMuon();
}

bool HiOniaCombinedAnalyzer::selTrackerMuon(const pat::Muon* aMuon) {
  return aMuon->isTrackerMuon();
}

bool HiOniaCombinedAnalyzer::selGlobalOrTrackerMuon(const pat::Muon* aMuon) {
  // Match legacy behavior: just check isGlobal OR isTracker (no applyCuts)
  return (aMuon->isGlobalMuon() || aMuon->isTrackerMuon());
}

bool HiOniaCombinedAnalyzer::isMuonInAccept(const pat::Muon* aMuon, std::string muonType) {
  return std::abs(aMuon->eta()) < 2.4;
}

bool HiOniaCombinedAnalyzer::isSoftMuonBase(const pat::Muon* aMuon) {
  return aMuon->passed(reco::Muon::SoftCutBasedId);
}

bool HiOniaCombinedAnalyzer::isHybridSoftMuon(const pat::Muon* aMuon) {
  if (!aMuon->isTrackerMuon()) return false;
  if (!aMuon->innerTrack().isNonnull()) return false;
  return (aMuon->innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5 &&
          aMuon->innerTrack()->hitPattern().pixelLayersWithMeasurement() > 0);
}

void HiOniaCombinedAnalyzer::checkMuonTriggers(const pat::CompositeCandidate* aJpsiCand) {
  const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(aJpsiCand->daughter("muon1"));
  const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(aJpsiCand->daughter("muon2"));
  if (!muon1 || !muon2) return;

  unsigned int iTr = 1;
  for (const auto& pathName : dblMuonTriggerPathNames_) {
    auto it = muonFilterNameMap_.find(pathName);
    if (it != muonFilterNameMap_.end() && !it->second.empty()) {
      const auto& mu1HLTMatches = muon1->triggerObjectMatchesByFilter(it->second);
      const auto& mu2HLTMatches = muon2->triggerObjectMatchesByFilter(it->second);
      isTriggerMatched_[iTr] = (!mu1HLTMatches.empty() && !mu2HLTMatches.empty());
    }
    iTr++;
  }
  for (const auto& pathName : sglMuonTriggerPathNames_) {
    auto it = muonFilterNameMap_.find(pathName);
    if (it != muonFilterNameMap_.end() && !it->second.empty()) {
      const auto& mu1HLTMatches = muon1->triggerObjectMatchesByFilter(it->second);
      const auto& mu2HLTMatches = muon2->triggerObjectMatchesByFilter(it->second);
      isTriggerMatched_[iTr] = (!mu1HLTMatches.empty() || !mu2HLTMatches.empty());
    }
    iTr++;
  }
}

TLorentzVector HiOniaCombinedAnalyzer::lorentzMomentum(const reco::Candidate::LorentzVector& p) {
  return TLorentzVector(p.px(), p.py(), p.pz(), p.energy());
}

long int HiOniaCombinedAnalyzer::FloatToIntkey(float v) {
  return static_cast<long int>(v * 1e6);
}

// =====================================================
// ELECTRON METHODS
// =====================================================
void HiOniaCombinedAnalyzer::fillRecoElectrons(const edm::Event& iEvent) {
  edm::Handle<pat::ElectronCollection> electrons;
  iEvent.getByToken(electronToken_, electrons);
  if (!electrons.isValid()) return;

  edm::Handle<reco::ConversionCollection> conversions;
  iEvent.getByToken(conversionToken_, conversions);

  edm::Handle<reco::BeamSpot> beamSpot;
  iEvent.getByToken(beamSpotToken_, beamSpot);
  const math::XYZPoint beamSpotPos = beamSpot.isValid() ? beamSpot->position() : math::XYZPoint(0., 0., 0.);

  for (size_t iEle = 0; iEle < electrons->size(); ++iEle) {
    if (Reco_ele_size_ >= Max_ele_size) break;

    const auto& ele = (*electrons)[iEle];
    const auto idx = Reco_ele_size_;
    const auto& p4 = ele.p4();

    Reco_ele_pt_.push_back(static_cast<float>(p4.pt()));
    Reco_ele_eta_.push_back(static_cast<float>(p4.eta()));
    Reco_ele_phi_.push_back(static_cast<float>(p4.phi()));
    Reco_ele_y_.push_back(static_cast<float>(p4.Rapidity()));
    Reco_ele_mass_.push_back(static_cast<float>(p4.M()));

    Reco_ele_charge_[idx] = ele.charge();
    Reco_ele_energy_[idx] = ele.energy();
    Reco_ele_corrEnergy_[idx] = ele.ecalEnergy();
    Reco_ele_EcalEnergy_[idx] = ele.ecalEnergy();
    Reco_ele_isEB_[idx] = (ele.superCluster().isNonnull() && std::abs(ele.superCluster()->eta()) < 1.479) ? 1 : 0;

    if (ele.gsfTrack().isNonnull()) {
      const auto& gsfTrack = ele.gsfTrack();
      Reco_ele_ptErr_[idx] = gsfTrack->ptError();
      Reco_ele_trackPt_[idx] = gsfTrack->pt();
      Reco_ele_trackEta_[idx] = gsfTrack->eta();
      Reco_ele_trackPhi_[idx] = gsfTrack->phi();
      Reco_ele_trackNormalizedChi2_[idx] = gsfTrack->normalizedChi2();
      Reco_ele_nValidHits_[idx] = gsfTrack->hitPattern().numberOfValidHits();
      Reco_ele_nMissingHits_[idx] = gsfTrack->hitPattern().numberOfLostHits(reco::HitPattern::MISSING_INNER_HITS);
      Reco_ele_trackerLayers_[idx] = gsfTrack->hitPattern().trackerLayersWithMeasurement();
      Reco_ele_pixelLayers_[idx] = gsfTrack->hitPattern().pixelLayersWithMeasurement();
      Reco_ele_dxy_[idx] = gsfTrack->dxy(RefVtx_);
      Reco_ele_dxyErr_[idx] = gsfTrack->dxyError();
      Reco_ele_dz_[idx] = gsfTrack->dz(RefVtx_);
      Reco_ele_dzErr_[idx] = gsfTrack->dzError();
    } else {
      Reco_ele_ptErr_[idx] = kInvalidFloat;
      Reco_ele_trackPt_[idx] = kInvalidFloat;
      Reco_ele_trackEta_[idx] = kInvalidFloat;
      Reco_ele_trackPhi_[idx] = kInvalidFloat;
      Reco_ele_trackNormalizedChi2_[idx] = kInvalidFloat;
      Reco_ele_nValidHits_[idx] = kInvalidShort;
      Reco_ele_nMissingHits_[idx] = kInvalidShort;
      Reco_ele_trackerLayers_[idx] = kInvalidShort;
      Reco_ele_pixelLayers_[idx] = kInvalidShort;
      Reco_ele_dxy_[idx] = kInvalidFloat;
      Reco_ele_dxyErr_[idx] = kInvalidFloat;
      Reco_ele_dz_[idx] = kInvalidFloat;
      Reco_ele_dzErr_[idx] = kInvalidFloat;
    }

    if (ele.superCluster().isNonnull()) {
      const auto& sc = ele.superCluster();
      Reco_ele_scEta_[idx] = sc->eta();
      Reco_ele_scPhi_[idx] = sc->phi();
      Reco_ele_scEn_[idx] = sc->energy();
    } else {
      Reco_ele_scEta_[idx] = kInvalidFloat;
      Reco_ele_scPhi_[idx] = kInvalidFloat;
      Reco_ele_scEn_[idx] = kInvalidFloat;
    }

    Reco_ele_r9_[idx] = ele.full5x5_r9();
    Reco_ele_sigmaIEtaIEta_[idx] = ele.full5x5_sigmaIetaIeta();
    Reco_ele_hOverE_[idx] = ele.hcalOverEcal();
    Reco_ele_fbrem_[idx] = ele.fbrem();
    Reco_ele_eOverP_[idx] = ele.eSuperClusterOverP();
    Reco_ele_deltaEtaIn_[idx] = ele.deltaEtaSuperClusterTrackAtVtx();
    Reco_ele_deltaPhiIn_[idx] = ele.deltaPhiSuperClusterTrackAtVtx();

    const auto iso = ele.pfIsolationVariables();
    Reco_ele_pfChIso_[idx] = iso.sumChargedHadronPt;
    Reco_ele_pfNeuIso_[idx] = iso.sumNeutralHadronEt;
    Reco_ele_pfPhoIso_[idx] = iso.sumPhotonEt;

    short convVeto = kInvalidShort;
    if (conversions.isValid()) {
      const bool pass = !ConversionTools::hasMatchedConversion(ele, *conversions, beamSpotPos);
      convVeto = pass ? 1 : 0;
    }
    Reco_ele_convVeto_[idx] = convVeto;

    // MVA IDs from HI producer
    Reco_ele_MVAIso_[idx] = ele.hasUserFloat("hiMVAIso") ? ele.userFloat("hiMVAIso") : kInvalidFloat;
    Reco_ele_MVAId_[idx] = ele.hasUserFloat("hiMVAId") ? ele.userFloat("hiMVAId") : kInvalidFloat;
    Reco_ele_MVAIsoWP90_[idx] = ele.hasUserInt("hiMVAIsoWP90") ? ele.userInt("hiMVAIsoWP90") : kInvalidShort;
    Reco_ele_MVAIsoWP80_[idx] = ele.hasUserInt("hiMVAIsoWP80") ? ele.userInt("hiMVAIsoWP80") : kInvalidShort;
    Reco_ele_MVAIdWP90_[idx] = ele.hasUserInt("hiMVAIdWP90") ? ele.userInt("hiMVAIdWP90") : kInvalidShort;
    Reco_ele_MVAIdWP80_[idx] = ele.hasUserInt("hiMVAIdWP80") ? ele.userInt("hiMVAIdWP80") : kInvalidShort;
    Reco_ele_CutIdWP90_[idx] = ele.hasUserInt("hiCutIdWP90") ? ele.userInt("hiCutIdWP90") : kInvalidShort;
    Reco_ele_CutIdWP80_[idx] = ele.hasUserInt("hiCutIdWP80") ? ele.userInt("hiCutIdWP80") : kInvalidShort;

    Reco_ele_trigBits_[idx] = 0ULL;
    ++Reco_ele_size_;
  }
}

int HiOniaCombinedAnalyzer::findElectronIndex(const TLorentzVector& candP4) const {
  double bestDr = 1e9;
  int bestIdx = -1;
  for (size_t i = 0; i < Reco_ele_pt_.size(); ++i) {
    const double dEta = Reco_ele_eta_[i] - candP4.Eta();
    const double dPhi = reco::deltaPhi(Reco_ele_phi_[i], candP4.Phi());
    const double dR = std::sqrt(dEta * dEta + dPhi * dPhi);
    const double dPtRel = std::abs(Reco_ele_pt_[i] - candP4.Pt());
    if (dR < 0.02 && dPtRel < 5.0 && dR < bestDr) {
      bestDr = dR;
      bestIdx = static_cast<int>(i);
    }
  }
  return bestIdx;
}

void HiOniaCombinedAnalyzer::fillRecoDielectrons(const edm::Event& iEvent) {
  edm::Handle<pat::CompositeCandidateCollection> dielectrons;
  iEvent.getByToken(dielectronToken_, dielectrons);
  if (!dielectrons.isValid()) return;

  for (const auto& cand : *dielectrons) {
    if (Reco_ee_size_ >= Max_ee_size) break;

    const auto idx = Reco_ee_size_;
    const auto& p4 = cand.p4();

    Reco_ee_pt_.push_back(static_cast<float>(p4.pt()));
    Reco_ee_eta_.push_back(static_cast<float>(p4.eta()));
    Reco_ee_phi_.push_back(static_cast<float>(p4.phi()));
    Reco_ee_y_.push_back(static_cast<float>(p4.Rapidity()));
    Reco_ee_mass_.push_back(static_cast<float>(p4.M()));

    Reco_ee_charge_[idx] = cand.charge();
    Reco_ee_vProb_[idx] = cand.hasUserFloat("vProb") ? cand.userFloat("vProb") : kInvalidFloat;
    Reco_ee_chi2_[idx] = cand.hasUserFloat("chi2") ? cand.userFloat("chi2") : kInvalidFloat;

    // Find electron indices - use named daughters from HiOnia2EEPAT
    Short_t ele1Idx = -1, ele2Idx = -1;
    const auto* dau1 = dynamic_cast<const pat::Electron*>(cand.daughter("electron1"));
    const auto* dau2 = dynamic_cast<const pat::Electron*>(cand.daughter("electron2"));
    if (dau1) {
      const TLorentzVector v1(dau1->px(), dau1->py(), dau1->pz(), dau1->energy());
      ele1Idx = findElectronIndex(v1);
    }
    if (dau2) {
      const TLorentzVector v2(dau2->px(), dau2->py(), dau2->pz(), dau2->energy());
      ele2Idx = findElectronIndex(v2);
    }
    Reco_ee_ele1Idx_[idx] = ele1Idx;
    Reco_ee_ele2Idx_[idx] = ele2Idx;

    // Get trigger bits from candidate
    uint64_t candTrigBits = 0ULL;
    if (cand.hasUserData("trigBits")) {
      candTrigBits = *cand.userData<uint64_t>("trigBits");
    }
    Reco_ee_trigBits_[idx] = static_cast<ULong64_t>(candTrigBits);

    ++Reco_ee_size_;
  }
}

void HiOniaCombinedAnalyzer::fillGenElectrons(const edm::Event& iEvent) {
  if (genParticleToken_.isUninitialized()) return;

  edm::Handle<reco::GenParticleCollection> genParticles;
  iEvent.getByToken(genParticleToken_, genParticles);
  if (!genParticles.isValid()) return;

  // Collect gen electrons
  std::vector<size_t> genElectronIndices;
  for (size_t i = 0; i < genParticles->size(); ++i) {
    const auto& gen = (*genParticles)[i];
    if (std::abs(gen.pdgId()) != 11) continue;
    if (Gen_ele_size_ >= Max_ele_size) break;

    const auto& p4 = gen.p4();
    Gen_ele_pt_.push_back(static_cast<float>(p4.pt()));
    Gen_ele_eta_.push_back(static_cast<float>(p4.eta()));
    Gen_ele_phi_.push_back(static_cast<float>(p4.phi()));
    Gen_ele_y_.push_back(static_cast<float>(p4.Rapidity()));
    Gen_ele_mass_.push_back(static_cast<float>(p4.M()));
    Gen_ele_pdgId_[Gen_ele_size_] = gen.pdgId();
    Gen_ele_motherId_[Gen_ele_size_] = gen.mother() ? gen.mother()->pdgId() : 0;

    genElectronIndices.push_back(i);
    ++Gen_ele_size_;
  }

  // Create gen dielectron pairs from electrons with J/psi mother
  for (size_t i = 0; i < genElectronIndices.size(); ++i) {
    for (size_t j = i + 1; j < genElectronIndices.size(); ++j) {
      if (Gen_ee_size_ >= Max_ee_size) break;

      const auto& gen1 = (*genParticles)[genElectronIndices[i]];
      const auto& gen2 = (*genParticles)[genElectronIndices[j]];

      // Check if from same J/psi mother
      if (gen1.mother() && gen2.mother() &&
          gen1.mother()->pdgId() == 443 && gen2.mother()->pdgId() == 443) {
        const auto p4_ee = gen1.p4() + gen2.p4();
        Gen_ee_pt_.push_back(static_cast<float>(p4_ee.pt()));
        Gen_ee_eta_.push_back(static_cast<float>(p4_ee.eta()));
        Gen_ee_phi_.push_back(static_cast<float>(p4_ee.phi()));
        Gen_ee_y_.push_back(static_cast<float>(p4_ee.Rapidity()));
        Gen_ee_mass_.push_back(static_cast<float>(p4_ee.M()));
        Gen_ee_charge_[Gen_ee_size_] = gen1.charge() + gen2.charge();
        Gen_ee_ele1Idx_[Gen_ee_size_] = static_cast<Short_t>(i);
        Gen_ee_ele2Idx_[Gen_ee_size_] = static_cast<Short_t>(j);
        ++Gen_ee_size_;
      }
    }
  }
}

std::string HiOniaCombinedAnalyzer::sanitizeLabel(const std::string& raw) const {
  std::string safe = raw;
  for (auto& c : safe) {
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
      c = '_';
    }
  }
  return safe;
}

DEFINE_FWK_MODULE(HiOniaCombinedAnalyzer);
