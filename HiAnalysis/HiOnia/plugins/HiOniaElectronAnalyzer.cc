#include "HiAnalysis/HiOnia/interface/HiOniaElectronAnalyzer.h"

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

HiOniaElectronAnalyzer::HiOniaElectronAnalyzer(const edm::ParameterSet& iConfig)
    : electronToken_(consumes<pat::ElectronCollection>(iConfig.getParameter<edm::InputTag>("srcElectron"))),
      dielectronToken_(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("srcDielectron"))),
      pvToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("primaryVertexTag"))),
      beamSpotToken_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotTag"))),
      conversionToken_(consumes<reco::ConversionCollection>(iConfig.getParameter<edm::InputTag>("conversions"))),
      triggerResultsToken_(consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("triggerResults"))),
      centralityToken_(),
      centralityBinToken_(),
      evtPlaneToken_(),
      genParticleToken_(),
      triggerPathNames_(iConfig.getParameter<std::vector<std::string> >("triggerPathNames")),
      triggerLabels_(),
      checkTriggerNames_(iConfig.getParameter<bool>("checkTriggerNames")),
      storeGenInfo_(iConfig.getParameter<bool>("storeGenInfo")),
      isHI_(iConfig.getUntrackedParameter<bool>("isHI", false)),
      isMC_(iConfig.getUntrackedParameter<bool>("isMC", false)),
      useEvtPlane_(iConfig.getUntrackedParameter<bool>("useEvtPlane", false)),
      fillTree_(iConfig.getParameter<bool>("fillTree")),
      fillHistos_(iConfig.getParameter<bool>("fillHistos")),
      hltConfig_(),
      hltConfigInit_(false),
      eventTriggerBits_(0),
      tree_(nullptr),
      Reco_ele_size_(0),
      Reco_ee_size_(0),
      Gen_ele_size_(0) {
  usesResource(TFileService::kSharedResource);

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
  if (isMC_ && iConfig.existsAs<edm::InputTag>("genParticles")) {
    const auto genTag = iConfig.getParameter<edm::InputTag>("genParticles");
    if (!genTag.label().empty()) {
      genParticleToken_ = consumes<reco::GenParticleCollection>(genTag);
    }
  }

  triggerLabels_.reserve(triggerPathNames_.size());
  for (const auto& path : triggerPathNames_) {
    triggerLabels_.push_back("trig_" + sanitizeLabel(path));
  }

  triggerIndices_.resize(triggerPathNames_.size(), std::numeric_limits<unsigned int>::max());
  triggerAccepts_.resize(triggerPathNames_.size(), false);

  hasPrimaryVertex_ = false;
}

HiOniaElectronAnalyzer::~HiOniaElectronAnalyzer() = default;

void HiOniaElectronAnalyzer::beginJob() {
  initTree();
}

void HiOniaElectronAnalyzer::endJob() {}

void HiOniaElectronAnalyzer::beginRun(const edm::Run& run, const edm::EventSetup& setup) {

  EDConsumerBase::Labels labelTriggerResults;
  EDConsumerBase::labelsForToken(triggerResultsToken_, labelTriggerResults);
  const std::string pro = labelTriggerResults.process;
  std::cout << "Setting Trigger Result Process: " << pro << std::endl;
  bool changed = true;
  if (hltConfig_.init(run, setup, pro, changed)) {
    hltConfigInit_ = true;
  } else {
    throw cms::Exception("HiOniaElectronAnalyzer") << "Failed to initialize HLTConfigProvider";
  }
}

void HiOniaElectronAnalyzer::endRun(const edm::Run&, const edm::EventSetup&) {}

void HiOniaElectronAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup&) {
  initEvent();

  fillEventInfo(iEvent);
  fillPrimaryVertexInfo(iEvent);
  fillCentralityInfo(iEvent);
  fillTriggerInfo(iEvent);
  fillRecoElectrons(iEvent);
  fillRecoDielectrons(iEvent);
  if (storeGenInfo_ && isMC_) {
    fillGeneratorInfo(iEvent);
  }

  if (fillTree_ && tree_)
    tree_->Fill();
}

void HiOniaElectronAnalyzer::initTree() {
  tree_ = fs_->make<TTree>("eleTree", "Onia electron tree");

  tree_->Branch("run", &runNum_, "run/i");
  tree_->Branch("lumi", &lumiSec_, "lumi/i");
  tree_->Branch("event", &eventNum_, "event/l");
  tree_->Branch("nPV", &nPrimaryVertices_, "nPV/I");
  tree_->Branch("pvX", &pvX_, "pvX/F");
  tree_->Branch("pvY", &pvY_, "pvY/F");
  tree_->Branch("pvZ", &pvZ_, "pvZ/F");
  tree_->Branch("centrality", &centralityValue_, "centrality/F");
  tree_->Branch("centralityBin", &centralityBin_, "centralityBin/I");
  tree_->Branch("evtPlane", &evtPlaneAngle_, "evtPlane/F");
  tree_->Branch("triggerBits", &triggerBits_, "triggerBits/l");

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
  tree_->Branch("Reco_ele_corrEnergyErr", Reco_ele_corrEnergyErr_, "Reco_ele_corrEnergyErr[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_EcalEnergy", Reco_ele_EcalEnergy_, "Reco_ele_EcalEnergy[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_trackPt", Reco_ele_trackPt_, "Reco_ele_trackPt[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_trackEta", Reco_ele_trackEta_, "Reco_ele_trackEta[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_trackPhi", Reco_ele_trackPhi_, "Reco_ele_trackPhi[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_trackChi2", Reco_ele_trackChi2_, "Reco_ele_trackChi2[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_trackNdof", Reco_ele_trackNdof_, "Reco_ele_trackNdof[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_trackNormalizedChi2", Reco_ele_trackNormalizedChi2_,
                "Reco_ele_trackNormalizedChi2[Reco_ele_size]/F");
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
  tree_->Branch("Reco_ele_scRawEn", Reco_ele_scRawEn_, "Reco_ele_scRawEn[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_r9", Reco_ele_r9_, "Reco_ele_r9[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_sigmaIEtaIEta", Reco_ele_sigmaIEtaIEta_,
                "Reco_ele_sigmaIEtaIEta[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_sigmaIPhiIPhi", Reco_ele_sigmaIPhiIPhi_,
                "Reco_ele_sigmaIPhiIPhi[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_hOverE", Reco_ele_hOverE_, "Reco_ele_hOverE[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_fbrem", Reco_ele_fbrem_, "Reco_ele_fbrem[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_eOverP", Reco_ele_eOverP_, "Reco_ele_eOverP[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_eOverPInv", Reco_ele_eOverPInv_, "Reco_ele_eOverPInv[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_deltaEtaIn", Reco_ele_deltaEtaIn_, "Reco_ele_deltaEtaIn[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_deltaPhiIn", Reco_ele_deltaPhiIn_, "Reco_ele_deltaPhiIn[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_sigmaEtaEta", Reco_ele_sigmaEtaEta_, "Reco_ele_sigmaEtaEta[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_sigmaPhiPhi", Reco_ele_sigmaPhiPhi_, "Reco_ele_sigmaPhiPhi[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_pfChIso", Reco_ele_pfChIso_, "Reco_ele_pfChIso[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_pfNeuIso", Reco_ele_pfNeuIso_, "Reco_ele_pfNeuIso[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_pfPhoIso", Reco_ele_pfPhoIso_, "Reco_ele_pfPhoIso[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_pfPUIso", Reco_ele_pfPUIso_, "Reco_ele_pfPUIso[Reco_ele_size]/F");
  tree_->Branch("Reco_ele_convVeto", Reco_ele_convVeto_, "Reco_ele_convVeto[Reco_ele_size]/S");
  tree_->Branch("Reco_ele_trigBits", Reco_ele_trigBits_, "Reco_ele_trigBits[Reco_ele_size]/l");

  tree_->Branch("Reco_ee_size", &Reco_ee_size_, "Reco_ee_size/I");
  tree_->Branch("Reco_ee_pt", &Reco_ee_pt_);
  tree_->Branch("Reco_ee_eta", &Reco_ee_eta_);
  tree_->Branch("Reco_ee_phi", &Reco_ee_phi_);
  tree_->Branch("Reco_ee_y", &Reco_ee_y_);
  tree_->Branch("Reco_ee_mass", &Reco_ee_mass_);
  tree_->Branch("Reco_ee_vProb", Reco_ee_vProb_, "Reco_ee_vProb[Reco_ee_size]/F");
  tree_->Branch("Reco_ee_chi2", Reco_ee_chi2_, "Reco_ee_chi2[Reco_ee_size]/F");
  tree_->Branch("Reco_ee_ndf", Reco_ee_ndf_, "Reco_ee_ndf[Reco_ee_size]/F");
  tree_->Branch("Reco_ee_charge", Reco_ee_charge_, "Reco_ee_charge[Reco_ee_size]/S");
  tree_->Branch("Reco_ee_ele1Idx", Reco_ee_ele1Idx_, "Reco_ee_ele1Idx[Reco_ee_size]/S");
  tree_->Branch("Reco_ee_ele2Idx", Reco_ee_ele2Idx_, "Reco_ee_ele2Idx[Reco_ee_size]/S");
  tree_->Branch("Reco_ee_trigBits", Reco_ee_trigBits_, "Reco_ee_trigBits[Reco_ee_size]/l");

  if (storeGenInfo_ && isMC_) {
    tree_->Branch("Gen_ele_size", &Gen_ele_size_, "Gen_ele_size/I");
    tree_->Branch("Gen_ele_pt", &Gen_ele_pt_);
    tree_->Branch("Gen_ele_eta", &Gen_ele_eta_);
    tree_->Branch("Gen_ele_phi", &Gen_ele_phi_);
    tree_->Branch("Gen_ele_y", &Gen_ele_y_);
    tree_->Branch("Gen_ele_mass", &Gen_ele_mass_);
    tree_->Branch("Gen_ele_pdgId", Gen_ele_pdgId_, "Gen_ele_pdgId[Gen_ele_size]/I");
    tree_->Branch("Gen_ele_motherId", Gen_ele_motherId_, "Gen_ele_motherId[Gen_ele_size]/I");
  }
}

void HiOniaElectronAnalyzer::initEvent() {
  runNum_ = 0;
  lumiSec_ = 0;
  eventNum_ = 0;
  nPrimaryVertices_ = 0;
  pvX_ = pvY_ = pvZ_ = kInvalidFloat;
  hasPrimaryVertex_ = false;
  primaryVertexPoint_ = math::XYZPoint();
  centralityValue_ = kInvalidFloat;
  centralityBin_ = -1;
  evtPlaneAngle_ = kInvalidFloat;
  eventTriggerBits_ = 0;
  triggerBits_ = 0;

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

  Gen_ele_size_ = 0;
  Gen_ele_pt_.clear();
  Gen_ele_eta_.clear();
  Gen_ele_phi_.clear();
  Gen_ele_y_.clear();
  Gen_ele_mass_.clear();

  std::fill(triggerAccepts_.begin(), triggerAccepts_.end(), false);
}

void HiOniaElectronAnalyzer::fillEventInfo(const edm::Event& iEvent) {
  runNum_ = iEvent.id().run();
  lumiSec_ = iEvent.luminosityBlock();
  eventNum_ = iEvent.id().event();
}

void HiOniaElectronAnalyzer::fillPrimaryVertexInfo(const edm::Event& iEvent) {
  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(pvToken_, vertices);
  if (vertices.isValid() && !vertices->empty()) {
    const auto& pv = vertices->front();
    nPrimaryVertices_ = vertices->size();
    pvX_ = pv.x();
    pvY_ = pv.y();
    pvZ_ = pv.z();
    primaryVertexPoint_ = pv.position();
    hasPrimaryVertex_ = true;
  } else {
    nPrimaryVertices_ = 0;
    pvX_ = pvY_ = pvZ_ = kInvalidFloat;
    hasPrimaryVertex_ = false;
  }
}

void HiOniaElectronAnalyzer::fillCentralityInfo(const edm::Event& iEvent) {
  if (!centralityToken_.isUninitialized()) {
    edm::Handle<reco::Centrality> centrality;
    iEvent.getByToken(centralityToken_, centrality);
    if (centrality.isValid()) {
      centralityValue_ = centrality->EtHFhitSum();
    }
  }
  if (!centralityBinToken_.isUninitialized()) {
    edm::Handle<int> centBin;
    iEvent.getByToken(centralityBinToken_, centBin);
    if (centBin.isValid()) {
      centralityBin_ = *centBin;
    }
  }
  if (useEvtPlane_ && !evtPlaneToken_.isUninitialized()) {
    edm::Handle<reco::EvtPlaneCollection> evtPlanes;
    iEvent.getByToken(evtPlaneToken_, evtPlanes);
    if (evtPlanes.isValid() && !evtPlanes->empty()) {
      evtPlaneAngle_ = evtPlanes->front().angle();
    }
  }
}

void HiOniaElectronAnalyzer::fillTriggerInfo(const edm::Event& iEvent) {
  edm::Handle<edm::TriggerResults> trigResults;
  iEvent.getByToken(triggerResultsToken_, trigResults);
  eventTriggerBits_ = 0ULL;

  if (!trigResults.isValid()) {
    triggerBits_ = eventTriggerBits_;
    return;
  }

  const edm::TriggerNames& trigNames = iEvent.triggerNames(*trigResults);
  for (size_t i = 0; i < triggerPathNames_.size(); ++i) {
    const std::string& requested = triggerPathNames_[i];
    const std::string resolved = resolveTriggerName(requested, trigNames);
    if (resolved.empty()) {
      if (checkTriggerNames_ && unresolvedPaths_.insert(requested).second) {
        edm::LogWarning("HiOniaElectronAnalyzer")
            << "Requested trigger path " << requested << " could not be resolved in the HLT menu.";
      }
      triggerIndices_[i] = std::numeric_limits<unsigned int>::max();
      triggerAccepts_[i] = false;
      continue;
    }
    const unsigned int index = trigNames.triggerIndex(resolved);
    triggerIndices_[i] = index;
    const bool accepted = (index < trigResults->size()) ? trigResults->accept(index) : false;
    triggerAccepts_[i] = accepted;
    if (accepted && i < 64) {
      eventTriggerBits_ |= (1ULL << i);
    }
  }
  triggerBits_ = eventTriggerBits_;
}

void HiOniaElectronAnalyzer::fillRecoElectrons(const edm::Event& iEvent) {
  edm::Handle<pat::ElectronCollection> electrons;
  iEvent.getByToken(electronToken_, electrons);
  if (!electrons.isValid())
    return;

  edm::Handle<reco::ConversionCollection> conversions;
  iEvent.getByToken(conversionToken_, conversions);

  edm::Handle<reco::BeamSpot> beamSpot;
  iEvent.getByToken(beamSpotToken_, beamSpot);
  const math::XYZPoint beamSpotPos = beamSpot.isValid() ? beamSpot->position() : math::XYZPoint(0., 0., 0.);

  const size_t nEle = electrons->size();
  Reco_ele_pt_.reserve(Reco_ele_pt_.size() + nEle);
  Reco_ele_eta_.reserve(Reco_ele_eta_.size() + nEle);
  Reco_ele_phi_.reserve(Reco_ele_phi_.size() + nEle);
  Reco_ele_y_.reserve(Reco_ele_y_.size() + nEle);
  Reco_ele_mass_.reserve(Reco_ele_mass_.size() + nEle);

  for (size_t iEle = 0; iEle < nEle; ++iEle) {
    if (Reco_ele_size_ >= Max_ele_size)
      break;

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
    Reco_ele_corrEnergyErr_[idx] = kInvalidFloat;
    Reco_ele_EcalEnergy_[idx] = ele.ecalEnergy();
    Reco_ele_isEB_[idx] = (ele.superCluster().isNonnull() && std::abs(ele.superCluster()->eta()) < 1.479) ? 1 : 0;

    if (ele.gsfTrack().isNonnull()) {
      const auto& gsfTrack = ele.gsfTrack();
      Reco_ele_ptErr_[idx] = gsfTrack->ptError();
      Reco_ele_trackPt_[idx] = gsfTrack->pt();
      Reco_ele_trackEta_[idx] = gsfTrack->eta();
      Reco_ele_trackPhi_[idx] = gsfTrack->phi();
      Reco_ele_trackChi2_[idx] = gsfTrack->chi2();
      Reco_ele_trackNdof_[idx] = gsfTrack->ndof();
      Reco_ele_trackNormalizedChi2_[idx] = gsfTrack->normalizedChi2();
      Reco_ele_nValidHits_[idx] = gsfTrack->hitPattern().numberOfValidHits();
      Reco_ele_nMissingHits_[idx] = gsfTrack->hitPattern().numberOfLostHits(reco::HitPattern::MISSING_INNER_HITS);
      Reco_ele_trackerLayers_[idx] = gsfTrack->hitPattern().trackerLayersWithMeasurement();
      Reco_ele_pixelLayers_[idx] = gsfTrack->hitPattern().pixelLayersWithMeasurement();
      if (hasPrimaryVertex_) {
        Reco_ele_dxy_[idx] = gsfTrack->dxy(primaryVertexPoint_);
        Reco_ele_dxyErr_[idx] = gsfTrack->dxyError();
        Reco_ele_dz_[idx] = gsfTrack->dz(primaryVertexPoint_);
        Reco_ele_dzErr_[idx] = gsfTrack->dzError();
      } else {
        Reco_ele_dxy_[idx] = kInvalidFloat;
        Reco_ele_dxyErr_[idx] = kInvalidFloat;
        Reco_ele_dz_[idx] = kInvalidFloat;
        Reco_ele_dzErr_[idx] = kInvalidFloat;
      }
    } else {
      Reco_ele_ptErr_[idx] = kInvalidFloat;
      Reco_ele_trackPt_[idx] = kInvalidFloat;
      Reco_ele_trackEta_[idx] = kInvalidFloat;
      Reco_ele_trackPhi_[idx] = kInvalidFloat;
      Reco_ele_trackChi2_[idx] = kInvalidFloat;
      Reco_ele_trackNdof_[idx] = kInvalidFloat;
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
      Reco_ele_scRawEn_[idx] = sc->rawEnergy();
    } else {
      Reco_ele_scEta_[idx] = kInvalidFloat;
      Reco_ele_scPhi_[idx] = kInvalidFloat;
      Reco_ele_scEn_[idx] = kInvalidFloat;
      Reco_ele_scRawEn_[idx] = kInvalidFloat;
    }

    Reco_ele_r9_[idx] = ele.full5x5_r9();
    Reco_ele_sigmaIEtaIEta_[idx] = ele.full5x5_sigmaIetaIeta();
    Reco_ele_sigmaIPhiIPhi_[idx] = ele.full5x5_sigmaIphiIphi();
    Reco_ele_hOverE_[idx] = ele.hcalOverEcal();
    Reco_ele_fbrem_[idx] = ele.fbrem();
    Reco_ele_eOverP_[idx] = ele.eSuperClusterOverP();
    Reco_ele_eOverPInv_[idx] = (ele.eSuperClusterOverP() > 0.f) ? (1.f / ele.eSuperClusterOverP()) : kInvalidFloat;
    Reco_ele_deltaEtaIn_[idx] = ele.deltaEtaSuperClusterTrackAtVtx();
    Reco_ele_deltaPhiIn_[idx] = ele.deltaPhiSuperClusterTrackAtVtx();
    if (ele.superCluster().isNonnull()) {
      Reco_ele_sigmaEtaEta_[idx] = ele.superCluster()->etaWidth();
      Reco_ele_sigmaPhiPhi_[idx] = ele.superCluster()->phiWidth();
    } else {
      Reco_ele_sigmaEtaEta_[idx] = kInvalidFloat;
      Reco_ele_sigmaPhiPhi_[idx] = kInvalidFloat;
    }

    const auto iso = ele.pfIsolationVariables();
    Reco_ele_pfChIso_[idx] = iso.sumChargedHadronPt;
    Reco_ele_pfNeuIso_[idx] = iso.sumNeutralHadronEt;
    Reco_ele_pfPhoIso_[idx] = iso.sumPhotonEt;
    Reco_ele_pfPUIso_[idx] = iso.sumPUPt;

    short convVeto = kInvalidShort;
    if (conversions.isValid()) {
      const bool pass = !ConversionTools::hasMatchedConversion(ele, *conversions, beamSpotPos);
      convVeto = pass ? 1 : 0;
    }
    Reco_ele_convVeto_[idx] = convVeto;

    Reco_ele_trigBits_[idx] = 0ULL;

    ++Reco_ele_size_;
  }

  Reco_ele_size_ = static_cast<Int_t>(Reco_ele_pt_.size());
}

int HiOniaElectronAnalyzer::findElectronIndex(const TLorentzVector& candP4) const {
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

void HiOniaElectronAnalyzer::fillRecoDielectrons(const edm::Event& iEvent) {
  edm::Handle<pat::CompositeCandidateCollection> dielectrons;
  iEvent.getByToken(dielectronToken_, dielectrons);
  if (!dielectrons.isValid())
    return;

  for (const auto& cand : *dielectrons) {
    if (Reco_ee_size_ >= Max_ee_size)
      break;

    const auto idx = Reco_ee_size_;
    const auto& p4 = cand.p4();

    Reco_ee_pt_.push_back(static_cast<float>(p4.pt()));
    Reco_ee_eta_.push_back(static_cast<float>(p4.eta()));
    Reco_ee_phi_.push_back(static_cast<float>(p4.phi()));
    Reco_ee_y_.push_back(static_cast<float>(p4.Rapidity()));
    Reco_ee_mass_.push_back(static_cast<float>(p4.M()));

    uint64_t candTrigBits = 0ULL;
    uint64_t ele1TrigBits = 0ULL;
    uint64_t ele2TrigBits = 0ULL;
    if (cand.hasUserData("trigBits")) {
      candTrigBits = *cand.userData<uint64_t>("trigBits");
    }
    if (cand.hasUserData("ele1TrigBits")) {
      ele1TrigBits = *cand.userData<uint64_t>("ele1TrigBits");
    }
    if (cand.hasUserData("ele2TrigBits")) {
      ele2TrigBits = *cand.userData<uint64_t>("ele2TrigBits");
    }

    Reco_ee_charge_[idx] = cand.charge();
    Reco_ee_vProb_[idx] = cand.hasUserFloat("vProb") ? cand.userFloat("vProb") : kInvalidFloat;
    Reco_ee_chi2_[idx] = cand.hasUserFloat("chi2") ? cand.userFloat("chi2") : kInvalidFloat;
    Reco_ee_ndf_[idx] = cand.hasUserFloat("ndf") ? cand.userFloat("ndf") : kInvalidFloat;

    Short_t ele1Idx = -1;
    Short_t ele2Idx = -1;
    if (cand.numberOfDaughters() >= 2) {
      const auto* dau1 = dynamic_cast<const pat::Electron*>(cand.daughter(0));
      const auto* dau2 = dynamic_cast<const pat::Electron*>(cand.daughter(1));
      if (dau1) {
        const TLorentzVector v1(dau1->px(), dau1->py(), dau1->pz(), dau1->energy());
        ele1Idx = findElectronIndex(v1);
      }
      if (dau2) {
        const TLorentzVector v2(dau2->px(), dau2->py(), dau2->pz(), dau2->energy());
        ele2Idx = findElectronIndex(v2);
      }
    }
    Reco_ee_ele1Idx_[idx] = ele1Idx;
    Reco_ee_ele2Idx_[idx] = ele2Idx;

    Reco_ee_trigBits_[idx] = static_cast<ULong64_t>(candTrigBits);
    if (ele1Idx >= 0 && ele1Idx < Max_ele_size) {
      Reco_ele_trigBits_[ele1Idx] |= static_cast<ULong64_t>(ele1TrigBits);
    }
    if (ele2Idx >= 0 && ele2Idx < Max_ele_size) {
      Reco_ele_trigBits_[ele2Idx] |= static_cast<ULong64_t>(ele2TrigBits);
    }

    ++Reco_ee_size_;
  }

  Reco_ee_size_ = static_cast<Int_t>(Reco_ee_pt_.size());
}

void HiOniaElectronAnalyzer::fillGeneratorInfo(const edm::Event& iEvent) {
  if (genParticleToken_.isUninitialized())
    return;

  edm::Handle<reco::GenParticleCollection> genParticles;
  iEvent.getByToken(genParticleToken_, genParticles);
  if (!genParticles.isValid())
    return;

  for (const auto& gen : *genParticles) {
    if (std::abs(gen.pdgId()) != 11)
      continue;
    if (Gen_ele_size_ >= Max_ele_size)
      break;

    const auto& p4 = gen.p4();
    Gen_ele_pt_.push_back(static_cast<float>(p4.pt()));
    Gen_ele_eta_.push_back(static_cast<float>(p4.eta()));
    Gen_ele_phi_.push_back(static_cast<float>(p4.phi()));
    Gen_ele_y_.push_back(static_cast<float>(p4.Rapidity()));
    Gen_ele_mass_.push_back(static_cast<float>(p4.M()));
    Gen_ele_pdgId_[Gen_ele_size_] = gen.pdgId();
    Gen_ele_motherId_[Gen_ele_size_] = gen.mother() ? gen.mother()->pdgId() : 0;
    ++Gen_ele_size_;
  }

  Gen_ele_size_ = static_cast<Int_t>(Gen_ele_pt_.size());
}

std::string HiOniaElectronAnalyzer::resolveTriggerName(const std::string& requested,
                                                      const edm::TriggerNames& names) const {
  if (requested.empty())
    return std::string();

  const auto& triggerNames = names.triggerNames();
  for (const auto& name : triggerNames) {
    if (name == requested)
      return name;
  }

  const auto pos = requested.find("_v");
  if (pos != std::string::npos) {
    const std::string prefix = requested.substr(0, pos);
    for (const auto& name : triggerNames) {
      if (name.compare(0, prefix.size(), prefix) == 0)
        return name;
    }
  }
  return std::string();
}

std::string HiOniaElectronAnalyzer::sanitizeLabel(const std::string& raw) const {
  std::string safe = raw;
  for (auto& c : safe) {
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
      c = '_';
    }
  }
  return safe;
}

DEFINE_FWK_MODULE(HiOniaElectronAnalyzer);
