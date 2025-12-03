#include "HiSkim/HiOnia2EE/interface/HiOnia2EEPAT.h"

//Headers for the data items
#include <DataFormats/TrackReco/interface/TrackFwd.h>
#include <DataFormats/TrackReco/interface/Track.h>
#include <DataFormats/EgammaCandidates/interface/GsfElectronFwd.h>
#include <DataFormats/EgammaCandidates/interface/GsfElectron.h>
#include <DataFormats/Common/interface/View.h>
#include <DataFormats/HepMCCandidate/interface/GenParticle.h>
#include <DataFormats/PatCandidates/interface/Electron.h>
#include <DataFormats/VertexReco/interface/VertexFwd.h>

//Headers for services and tools
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/PrimaryVertexProducer/interface/PrimaryVertexProducer.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "TMath.h"
#include "Math/VectorUtil.h"
#include "TVector3.h"

#include "TrackingTools/PatternTools/interface/TwoTrackMinimumDistance.h"
#include "TrackingTools/IPTools/interface/IPTools.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "DataFormats/Math/interface/deltaR.h"

#include <cctype>
#include <algorithm>
#include <cstdint>

HiOnia2EEPAT::HiOnia2EEPAT(const edm::ParameterSet &iConfig)
    : electronsToken_(consumes<edm::View<pat::Electron> >(iConfig.getParameter<edm::InputTag>("electrons"))),
      thebeamspotToken_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotTag"))),
      thePVsToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("primaryVertexTag"))),
      recoTracksToken_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("srcTracks"))),
      theGenParticlesToken_(consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticles"))),
      conversionsToken_(consumes<reco::ConversionCollection>(iConfig.getParameter<edm::InputTag>("conversions"))),
      magFieldToken_(esConsumes<MagneticField, IdealMagneticFieldRecord>()),
      trackBuilderToken_(esConsumes(edm::ESInputTag("", "TransientTrackBuilder"))),
      higherPuritySelection_(iConfig.getParameter<std::string>("higherPuritySelection")),
      lowerPuritySelection_(iConfig.getParameter<std::string>("lowerPuritySelection")),
      dielectronSelection_(
          iConfig.existsAs<std::string>("dielectronSelection") ? iConfig.getParameter<std::string>("dielectronSelection") : ""),
      DiEleTrkSelection_(iConfig.existsAs<std::string>("DiEleTrkSelection")
                            ? iConfig.getParameter<std::string>("DiEleTrkSelection")
                            : ""),
      trielectronSelection_(iConfig.existsAs<std::string>("trielectronSelection")
                            ? iConfig.getParameter<std::string>("trielectronSelection")
                            : ""),
      LateDiElectronSel_(iConfig.existsAs<std::string>("LateDiElectronSel") ? iConfig.getParameter<std::string>("LateDiElectronSel")
                                                                    : ""),
      LateDiEleTrkSel_(
          iConfig.existsAs<std::string>("LateDiEleTrkSel") ? iConfig.getParameter<std::string>("LateDiEleTrkSel") : ""),
      LateTriElectronSel_(
          iConfig.existsAs<std::string>("LateTriElectronSel") ? iConfig.getParameter<std::string>("LateTriElectronSel") : ""),
      addCommonVertex_(iConfig.getParameter<bool>("addCommonVertex")),
      addElectronlessPrimaryVertex_(iConfig.getParameter<bool>("addElectronlessPrimaryVertex")),
      resolveAmbiguity_(iConfig.getParameter<bool>("resolvePileUpAmbiguity")),
      onlyGoodElectrons_(iConfig.getParameter<bool>("onlyGoodElectrons")),
      onlySingleElectrons_(iConfig.getParameter<bool>("onlySingleElectrons")),
      doTriElectrons_(iConfig.getParameter<bool>("doTriElectrons")),
      DiElectronTrk_(iConfig.getParameter<bool>("DiElectronTrk")),
      flipJpsiDirection_(iConfig.getParameter<int>("flipJpsiDirection")),
      Converter_(converter::TrackToCandidate(iConfig, consumesCollector())),
      trackType_(iConfig.getParameter<int>("particleType")),
      trackMass_(iConfig.getParameter<double>("trackMass")),
      doTriggerMatch_(false),
      triggerMatchDR_(0.3),
      triggerPaths_(),
      triggerLabels_(),
      triggerResultsToken_(),
      triggerObjectsToken_(),
      requireLastFilter_(true),
      requireL3Filter_(false) {
  if (iConfig.existsAs<bool>("doTriggerMatching")) {
    doTriggerMatch_ = iConfig.getParameter<bool>("doTriggerMatching");
  }

  if (doTriggerMatch_) {
    if (!iConfig.existsAs<edm::InputTag>("triggerResults") || !iConfig.existsAs<edm::InputTag>("triggerObjects") ||
        !iConfig.existsAs<std::vector<std::string> >("triggerPaths")) {
      throw cms::Exception("Configuration")
          << "doTriggerMatching is True but triggerResults, triggerObjects, or triggerPaths parameters are missing.";
    }

    triggerMatchDR_ = iConfig.existsAs<double>("triggerMatchDR") ? iConfig.getParameter<double>("triggerMatchDR") : 0.3;
    requireLastFilter_ =
        iConfig.existsAs<bool>("requireLastFilter") ? iConfig.getParameter<bool>("requireLastFilter") : true;
    requireL3Filter_ =
        iConfig.existsAs<bool>("requireL3Filter") ? iConfig.getParameter<bool>("requireL3Filter") : false;

    triggerPaths_ = iConfig.getParameter<std::vector<std::string> >("triggerPaths");
    triggerLabels_.reserve(triggerPaths_.size());
    for (const auto& path : triggerPaths_) {
      triggerLabels_.push_back("trig_" + sanitizeLabel(path));
    }

      triggerResultsToken_ = consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("triggerResults"));
    triggerObjectsToken_ = consumes<std::vector<pat::TriggerObjectStandAlone> >(
        iConfig.getParameter<edm::InputTag>("triggerObjects"));
  }

  // Electron ID configuration
  electronIDType_ = iConfig.existsAs<std::string>("electronIDType") 
                    ? iConfig.getParameter<std::string>("electronIDType") : "hardcoded";
  electronIDWP_ = iConfig.existsAs<std::string>("electronIDWP") 
                  ? iConfig.getParameter<std::string>("electronIDWP") : "";
  electronIDName_ = iConfig.existsAs<std::string>("electronIDName") 
                    ? iConfig.getParameter<std::string>("electronIDName") : "";

  produces<pat::CompositeCandidateCollection>("");
  produces<pat::CompositeCandidateCollection>("trielectron");
  produces<pat::CompositeCandidateCollection>("dieletrk");
};

HiOnia2EEPAT::~HiOnia2EEPAT(){};
//
// member functions
//

bool HiOnia2EEPAT::isGoodElectron(const pat::Electron *aElectron, const reco::BeamSpot& beamSpot, const reco::ConversionCollection& conversions) {
  if (!aElectron->gsfTrack().isNonnull()) return false;
  if (aElectron->gsfTrack()->hitPattern().trackerLayersWithMeasurement() <= 5) return false;
  if (aElectron->gsfTrack()->hitPattern().pixelLayersWithMeasurement() == 0) return false;
  if (fabs(aElectron->gsfTrack()->dxy(RefVtx)) >= 0.3) return false;
  if (fabs(aElectron->gsfTrack()->dz(RefVtx)) >= 20.) return false;
  if (ConversionTools::hasMatchedConversion(*aElectron, conversions, beamSpot.position())) return false;
  if (!passElectronID(*aElectron)) return false;
  return true;
}

bool HiOnia2EEPAT::passElectronID(const pat::Electron& ele) {
  // Use configurable electron ID based on electronIDType_
  if (electronIDType_ == "hardcoded") {
    return passHardcodedID(ele);
  } 
  else if (electronIDType_ == "cutbased" || electronIDType_ == "mva") {
    // Use MiniAOD's native electronID() method
    if (!electronIDName_.empty()) {
      return ele.electronID(electronIDName_) > 0.5;
    }
    // Fallback to standard ID names based on WP
    std::string idName;
    if (electronIDType_ == "cutbased") {
      if (electronIDWP_ == "veto") idName = "cutBasedElectronID-RunIIIWinter22-V1-veto";
      else if (electronIDWP_ == "loose") idName = "cutBasedElectronID-RunIIIWinter22-V1-loose";
      else if (electronIDWP_ == "medium") idName = "cutBasedElectronID-RunIIIWinter22-V1-medium";
      else if (electronIDWP_ == "tight") idName = "cutBasedElectronID-RunIIIWinter22-V1-tight";
      else idName = "cutBasedElectronID-RunIIIWinter22-V1-loose"; // default
    } else { // mva
      if (electronIDWP_ == "wp90") idName = "mvaEleID-RunIIIWinter22-iso-V1-wp90";
      else if (electronIDWP_ == "wp80") idName = "mvaEleID-RunIIIWinter22-iso-V1-wp80";
      else idName = "mvaEleID-RunIIIWinter22-iso-V1-wp90"; // default
    }
    if (ele.isElectronIDAvailable(idName)) {
      return ele.electronID(idName) > 0.5;
    }
    // ID not available, fallback to hardcoded
    return passHardcodedID(ele);
  }
  else if (electronIDType_ == "hiMVA") {
    // Use HI-specific MVA ID from HIElectronInfoProducer
    std::string wpKey = "hiMVAIdWP" + electronIDWP_;
    if (ele.hasUserInt(wpKey)) {
      return ele.userInt(wpKey) > 0;
    }
    // Fallback: check hiMVAId score directly with threshold
    if (ele.hasUserFloat("hiMVAId")) {
      float threshold = 0.0;
      if (electronIDWP_ == "95") threshold = -0.5;
      else if (electronIDWP_ == "90") threshold = 0.0;
      else if (electronIDWP_ == "85") threshold = 0.3;
      else if (electronIDWP_ == "80") threshold = 0.5;
      return ele.userFloat("hiMVAId") > threshold;
    }
    return passHardcodedID(ele);
  }
  else if (electronIDType_ == "none") {
    return true; // No ID applied
  }
  // Default fallback
  return passHardcodedID(ele);
}

bool HiOnia2EEPAT::passHardcodedID(const pat::Electron& ele) {
  if (ele.hcalOverEcal() > 0.15) return false;
  if (fabs(ele.superCluster()->eta()) <= 1.479) {
    if (ele.full5x5_sigmaIetaIeta() > 0.012) return false;
  } else {
    if (ele.full5x5_sigmaIetaIeta() > 0.035) return false;
  }
  if (fabs(ele.deltaEtaSuperClusterTrackAtVtx()) > 0.01) return false;
  if (fabs(ele.deltaPhiSuperClusterTrackAtVtx()) > 0.08) return false;
  return true;
}

//1: $z -> -z$ and $\phi -> \phi+\pi$ (mirror)
//2: $z -> -z$ and $\phi -> \phi+\pi/2$
//3: $z -> -z$
//4: $z -> -z$ and $\phi -> \phi-\pi/2$
//5: $\phi -> \phi+\pi/2$
//6: $\phi -> \phi+\pi$
//7: $\phi -> \phi-\pi/2$
const reco::TrackBase::Point HiOnia2EEPAT::rotatePoint(reco::TrackBase::Point PV,
                                                         reco::TrackBase::Point TrkPoint,
                                                         int flipJpsi) {
  float x = TrkPoint.x(), y = TrkPoint.y(), z = TrkPoint.z();
  float vx = PV.x(), vy = PV.y(), vz = PV.z();

  if (flipJpsi <= 4) {
    z = 2 * vz - z;
  }

  switch (flipJpsi) {
    case 1:
    case 6:
      x = 2 * vx - x;
      y = 2 * vy - y;
      break;
    case 2:
      x = -(y - vy) + vx;
      y = (x - vx) + vy;
      break;
    case 4:
      x = (y - vy) + vx;
      y = -(x - vx) + vy;
      break;
    case 5:
      x = -(y - vy) + vx;
      y = (TrkPoint.x() - vx) + vy;
      break;
    case 7:
      x = (y - vy) + vx;
      y = -(TrkPoint.x() - vx) + vy;
      break;
  }
  return reco::TrackBase::Point(x, y, z);
}

const reco::TrackBase::Vector HiOnia2EEPAT::rotateMomentum(reco::Track trk, int flipJpsi) {
  float px = trk.px(), py = trk.py(), pz = trk.pz();

  if (flipJpsi <= 4) {
    pz = -pz;
  }

  switch (flipJpsi) {
    case 1:
    case 6:
      px = -px;
      py = -py;
      break;
    case 2:
      px = -py;
      py = trk.px();
      break;
    case 4:
      px = py;
      py = -trk.px();
      break;
    case 5:
      px = -py;
      py = trk.px();
      break;
    case 7:
      px = py;
      py = -trk.px();
      break;
  }
  return reco::TrackBase::Vector(px, py, pz);
}

// ------------ method called to produce the data  ------------
void HiOnia2EEPAT::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  using namespace std;
  using namespace reco;
  typedef Candidate::LorentzVector LorentzVector;

  // output collection
  unique_ptr<pat::CompositeCandidateCollection> oniaOutput(new pat::CompositeCandidateCollection);
  unique_ptr<pat::CompositeCandidateCollection> trielectronOutput(new pat::CompositeCandidateCollection);
  unique_ptr<pat::CompositeCandidateCollection> dieletrkOutput(new pat::CompositeCandidateCollection);

  // Get event content
  Handle<reco::BeamSpot> theBeamSpot;
  iEvent.getByToken(thebeamspotToken_, theBeamSpot);
  RefVtx = theBeamSpot->position();

  Handle<reco::VertexCollection> priVtxs;
  iEvent.getByToken(thePVsToken_, priVtxs);
  if (!priVtxs->empty()) {
    RefVtx = priVtxs->begin()->position();
  }

  Handle<View<pat::Electron> > electrons;
  iEvent.getByToken(electronsToken_, electrons);

  Handle<reco::ConversionCollection> conversions;
  iEvent.getByToken(conversionsToken_, conversions);

  std::vector<std::string> resolvedTriggerPaths;
  std::vector<bool> triggerAccepted;
  std::vector<pat::TriggerObjectStandAlone> matchedTriggerObjects;
  const edm::TriggerResults* triggerResultsPtr = nullptr;

  auto resolveTriggerPath = [](const std::string& requested, const edm::TriggerNames& names) -> std::string {
    const auto& allNames = names.triggerNames();
    // Exact match first
    for (const auto& name : allNames) {
      if (name == requested)
        return name;
    }
    const std::string suffix("_v");
    auto pos = requested.find(suffix);
    if (pos != std::string::npos) {
      std::string prefix = requested.substr(0, pos);
      for (const auto& name : allNames) {
        if (name.compare(0, prefix.size(), prefix) == 0 && name.find(suffix) == prefix.size())
          return name;
      }
    }
    return std::string();
  };

  if (doTriggerMatch_) {
    edm::Handle<edm::TriggerResults> triggerResultsHandle;
    edm::Handle<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsHandle;
    iEvent.getByToken(triggerResultsToken_, triggerResultsHandle);
    iEvent.getByToken(triggerObjectsToken_, triggerObjectsHandle);

    if (triggerResultsHandle.isValid() && triggerObjectsHandle.isValid()) {
      triggerResultsPtr = triggerResultsHandle.product();
      const edm::TriggerNames& triggerNames = iEvent.triggerNames(*triggerResultsPtr);

      resolvedTriggerPaths.reserve(triggerPaths_.size());
      triggerAccepted.resize(triggerPaths_.size(), false);

      for (size_t ipath = 0; ipath < triggerPaths_.size(); ++ipath) {
        std::string resolved = resolveTriggerPath(triggerPaths_[ipath], triggerNames);
        resolvedTriggerPaths.emplace_back(resolved);
        if (!resolved.empty()) {
          unsigned int index = triggerNames.triggerIndex(resolved);
          if (index < triggerResultsPtr->size()) {
            triggerAccepted[ipath] = triggerResultsPtr->accept(index);
          }
        }
      }

      matchedTriggerObjects.reserve(triggerObjectsHandle->size());
      for (const auto& obj : *triggerObjectsHandle) {
        matchedTriggerObjects.push_back(obj);
        matchedTriggerObjects.back().unpackPathNames(triggerNames);
      }
    } else {
      resolvedTriggerPaths.assign(triggerPaths_.size(), std::string());
      triggerAccepted.assign(triggerPaths_.size(), false);
    }
  }

  const auto &theTTBuilder = iSetup.getHandle(trackBuilderToken_);
  KalmanVertexFitter vtxFitter(true);

  //For kinematic constrained fit
  KinematicParticleFactoryFromTransientTrack pFactory;
  ParticleMass electron_mass = 0.0005109989;  // electron mass in GeV
  float electron_sigma = 0.0000000001;
  ParticleMass jp_mass = 3.09687;
  MultiTrackKinematicConstraint *jpsi_c = new TwoTrackMassKinematicConstraint(jp_mass);
  KinematicConstrainedVertexFitter KCfitter;

  TrackCollection electronLess;  // track collection related to PV, minus the 2 electrons

  int Ntrk = -1;
  std::vector<reco::TrackRef> ourTracks;
  if (DiElectronTrk_) {
    Handle<reco::TrackCollection> collTracks;
    iEvent.getByToken(recoTracksToken_, collTracks);
    if (collTracks.isValid()) {
      Ntrk = 0;
      for (unsigned int tidx = 0; tidx < collTracks->size(); tidx++) {
        const reco::TrackRef track(collTracks, tidx);
        if (track->qualityByName("highPurity") && track->eta() < 2.4 && fabs(track->dxy(RefVtx)) < 0.3 &&
            fabs(track->dz(RefVtx)) < 20) {
          Ntrk++;
          if (DiElectronTrk_) {
            ourTracks.push_back(track);
          }
        }
      }
    }
  }

  std::vector<pat::Electron> ourElectrons;
  for (View<pat::Electron>::const_iterator it = electrons->begin(), itend = electrons->end(); it != itend; ++it) {
    if (lowerPuritySelection_(*it) && (!onlyGoodElectrons_ || isGoodElectron(&(*it), *theBeamSpot, *conversions))) {
      ourElectrons.push_back(*it);
    }
  }
  int ourEleNb = ourElectrons.size();

  if (onlySingleElectrons_)
    goto skipElectronLoop;

  // Quarkonia candidates only from electrons
  for (int i = 0; i < ourEleNb; i++) {
    const pat::Electron &it = ourElectrons[i];
    for (int j = i + 1; j < ourEleNb; j++) {
      const pat::Electron &it2 = ourElectrons[j];
      // one electron must pass tight quality
      if (!(higherPuritySelection_(it) || higherPuritySelection_(it2)))
        continue;
      if (!(it.gsfTrack().isNonnull()) || !(it2.gsfTrack().isNonnull()))
        continue;

      // --- some declarations ---
      std::map<std::string, int> userInt;
      std::map<std::string, float> userFloat;
      std::map<std::string, reco::Vertex> userVertex;
      std::map<std::string, reco::Track> userTrack;
      Vertex theOriginalPV;
      int flipJpsi = 0;
      TransientVertex myVertex;
      CachingVertex<5> VtxForInvMass;
      Measurement1D MassWErr;
      vector<TransientTrack> t_tks;
      float vChi2 = -100, vNDF = 1;

      pat::CompositeCandidate myCand;
      pat::CompositeCandidate myCandTmp;
      // ---- no explicit order defined ----
      myCand.addDaughter(it, "electron1");
      myCand.addDaughter(it2, "electron2");

      reco::GsfTrack electron1Trk = (*it.gsfTrack());
      reco::GsfTrack electron2Trk = (*it2.gsfTrack());
      LorentzVector ele1 = it.p4();
      LorentzVector ele2 = it2.p4();

      // ---- define and set candidate's 4momentum  ----
      LorentzVector jpsi = ele1 + ele2;
      myCand.setP4(jpsi);
      myCand.setCharge(it.charge() + it2.charge());

      // Store electron-specific information
      userInt["ele1ConvVeto"] = (int)(!ConversionTools::hasMatchedConversion(it, *conversions, theBeamSpot->position()));
      userInt["ele2ConvVeto"] = (int)(!ConversionTools::hasMatchedConversion(it2, *conversions, theBeamSpot->position()));
      
      userFloat["ele1SCEta"] = it.superCluster()->eta();
      userFloat["ele2SCEta"] = it2.superCluster()->eta();
      userFloat["ele1SCPhi"] = it.superCluster()->phi();
      userFloat["ele2SCPhi"] = it2.superCluster()->phi();
      userFloat["ele1SCEn"] = it.superCluster()->energy();
      userFloat["ele2SCEn"] = it2.superCluster()->energy();
      
      userFloat["ele1HoverE"] = it.hcalOverEcal();
      userFloat["ele2HoverE"] = it2.hcalOverEcal();
      userFloat["ele1SigmaIEtaIEta"] = it.full5x5_sigmaIetaIeta();
      userFloat["ele2SigmaIEtaIEta"] = it2.full5x5_sigmaIetaIeta();
      userFloat["ele1DeltaEtaIn"] = it.deltaEtaSuperClusterTrackAtVtx();
      userFloat["ele2DeltaEtaIn"] = it2.deltaEtaSuperClusterTrackAtVtx();
      userFloat["ele1DeltaPhiIn"] = it.deltaPhiSuperClusterTrackAtVtx();
      userFloat["ele2DeltaPhiIn"] = it2.deltaPhiSuperClusterTrackAtVtx();
      
      userFloat["ele1FBrem"] = it.fbrem();
      userFloat["ele2FBrem"] = it2.fbrem();
      
      userFloat["ele1EoverP"] = it.eSuperClusterOverP();
      userFloat["ele2EoverP"] = it2.eSuperClusterOverP();
      
      reco::GsfElectron::PflowIsolationVariables pfIso1 = it.pfIsolationVariables();
      reco::GsfElectron::PflowIsolationVariables pfIso2 = it2.pfIsolationVariables();
      userFloat["ele1PFChIso"] = pfIso1.sumChargedHadronPt;
      userFloat["ele2PFChIso"] = pfIso2.sumChargedHadronPt;
      userFloat["ele1PFNeuIso"] = pfIso1.sumNeutralHadronEt;
      userFloat["ele2PFNeuIso"] = pfIso2.sumNeutralHadronEt;
      userFloat["ele1PFPhoIso"] = pfIso1.sumPhotonEt;
      userFloat["ele2PFPhoIso"] = pfIso2.sumPhotonEt;
      userFloat["ele1PFPUIso"] = pfIso1.sumPUPt;
      userFloat["ele2PFPUIso"] = pfIso2.sumPUPt;
      
      // MVA-based ID and Isolation
      userFloat["ele1MVAIso"] = it.hasUserFloat("hiMVAIso") ? it.userFloat("hiMVAIso") : -99.f;
      userFloat["ele2MVAIso"] = it2.hasUserFloat("hiMVAIso") ? it2.userFloat("hiMVAIso") : -99.f;
      userFloat["ele1MVAId"] = it.hasUserFloat("hiMVAId") ? it.userFloat("hiMVAId") : -99.f;
      userFloat["ele2MVAId"] = it2.hasUserFloat("hiMVAId") ? it2.userFloat("hiMVAId") : -99.f;
      
      userInt["ele1MVAIsoWP95"] = it.hasUserInt("hiMVAIsoWP95") ? it.userInt("hiMVAIsoWP95") : -1;
      userInt["ele2MVAIsoWP95"] = it2.hasUserInt("hiMVAIsoWP95") ? it2.userInt("hiMVAIsoWP95") : -1;
      userInt["ele1MVAIsoWP90"] = it.hasUserInt("hiMVAIsoWP90") ? it.userInt("hiMVAIsoWP90") : -1;
      userInt["ele2MVAIsoWP90"] = it2.hasUserInt("hiMVAIsoWP90") ? it2.userInt("hiMVAIsoWP90") : -1;
      userInt["ele1MVAIsoWP85"] = it.hasUserInt("hiMVAIsoWP85") ? it.userInt("hiMVAIsoWP85") : -1;
      userInt["ele2MVAIsoWP85"] = it2.hasUserInt("hiMVAIsoWP85") ? it2.userInt("hiMVAIsoWP85") : -1;
      userInt["ele1MVAIsoWP80"] = it.hasUserInt("hiMVAIsoWP80") ? it.userInt("hiMVAIsoWP80") : -1;
      userInt["ele2MVAIsoWP80"] = it2.hasUserInt("hiMVAIsoWP80") ? it2.userInt("hiMVAIsoWP80") : -1;
      
      userInt["ele1MVAIdWP95"] = it.hasUserInt("hiMVAIdWP95") ? it.userInt("hiMVAIdWP95") : -1;
      userInt["ele2MVAIdWP95"] = it2.hasUserInt("hiMVAIdWP95") ? it2.userInt("hiMVAIdWP95") : -1;
      userInt["ele1MVAIdWP90"] = it.hasUserInt("hiMVAIdWP90") ? it.userInt("hiMVAIdWP90") : -1;
      userInt["ele2MVAIdWP90"] = it2.hasUserInt("hiMVAIdWP90") ? it2.userInt("hiMVAIdWP90") : -1;
      userInt["ele1MVAIdWP85"] = it.hasUserInt("hiMVAIdWP85") ? it.userInt("hiMVAIdWP85") : -1;
      userInt["ele2MVAIdWP85"] = it2.hasUserInt("hiMVAIdWP85") ? it2.userInt("hiMVAIdWP85") : -1;
      userInt["ele1MVAIdWP80"] = it.hasUserInt("hiMVAIdWP80") ? it.userInt("hiMVAIdWP80") : -1;
      userInt["ele2MVAIdWP80"] = it2.hasUserInt("hiMVAIdWP80") ? it2.userInt("hiMVAIdWP80") : -1;
      
      userInt["ele1CutIdWP95"] = it.hasUserInt("hiCutIdWP95") ? it.userInt("hiCutIdWP95") : -1;
      userInt["ele2CutIdWP95"] = it2.hasUserInt("hiCutIdWP95") ? it2.userInt("hiCutIdWP95") : -1;
      userInt["ele1CutIdWP90"] = it.hasUserInt("hiCutIdWP90") ? it.userInt("hiCutIdWP90") : -1;
      userInt["ele2CutIdWP90"] = it2.hasUserInt("hiCutIdWP90") ? it2.userInt("hiCutIdWP90") : -1;
      userInt["ele1CutIdWP80"] = it.hasUserInt("hiCutIdWP80") ? it.userInt("hiCutIdWP80") : -1;
      userInt["ele2CutIdWP80"] = it2.hasUserInt("hiCutIdWP80") ? it2.userInt("hiCutIdWP80") : -1;
      userInt["ele1CutIdWP70"] = it.hasUserInt("hiCutIdWP70") ? it.userInt("hiCutIdWP70") : -1;
      userInt["ele2CutIdWP70"] = it2.hasUserInt("hiCutIdWP70") ? it2.userInt("hiCutIdWP70") : -1;
      
      // Energy corrections
      userFloat["ele1RawPt"] = it.hasUserFloat("rawPt") ? it.userFloat("rawPt") : it.pt();
      userFloat["ele2RawPt"] = it2.hasUserFloat("rawPt") ? it2.userFloat("rawPt") : it2.pt();
      userFloat["ele1RawEcalEnergy"] = it.hasUserFloat("rawEcalEnergy") ? it.userFloat("rawEcalEnergy") : it.ecalEnergy();
      userFloat["ele2RawEcalEnergy"] = it2.hasUserFloat("rawEcalEnergy") ? it2.userFloat("rawEcalEnergy") : it2.ecalEnergy();

      const bool haveTriggerInfo = doTriggerMatch_ && triggerResultsPtr != nullptr && !resolvedTriggerPaths.empty();
      uint64_t candTrigBits = 0ULL;
      uint64_t ele1TrigBits = 0ULL;
      uint64_t ele2TrigBits = 0ULL;

      if (haveTriggerInfo) {
        for (size_t ipath = 0; ipath < triggerPaths_.size(); ++ipath) {
          int candMatch = 0;
          int ele1Match = 0;
          int ele2Match = 0;

          if (!resolvedTriggerPaths[ipath].empty() && triggerAccepted[ipath]) {
            for (const auto& trigObj : matchedTriggerObjects) {
              if (!trigObj.hasPathName(resolvedTriggerPaths[ipath], requireLastFilter_, requireL3Filter_))
                continue;
              if (!ele1Match && reco::deltaR(it.eta(), it.phi(), trigObj.eta(), trigObj.phi()) < triggerMatchDR_) {
                ele1Match = 1;
              }
              if (!ele2Match && reco::deltaR(it2.eta(), it2.phi(), trigObj.eta(), trigObj.phi()) < triggerMatchDR_) {
                ele2Match = 1;
              }
              if (ele1Match || ele2Match) {
                candMatch = 1;
              }
              if (ele1Match && ele2Match)
                break;
            }
          }

          userInt[triggerLabels_[ipath]] = candMatch;
          userInt["ele1_" + triggerLabels_[ipath]] = ele1Match;
          userInt["ele2_" + triggerLabels_[ipath]] = ele2Match;

          if (ipath < 64) {
            const uint64_t bit = (1ULL << ipath);
            if (candMatch)
              candTrigBits |= bit;
            if (ele1Match)
              ele1TrigBits |= bit;
            if (ele2Match)
              ele2TrigBits |= bit;
          }
        }
      } else if (doTriggerMatch_) {
        for (size_t ipath = 0; ipath < triggerPaths_.size(); ++ipath) {
          userInt[triggerLabels_[ipath]] = 0;
          userInt["ele1_" + triggerLabels_[ipath]] = 0;
          userInt["ele2_" + triggerLabels_[ipath]] = 0;
        }
      }

      myCand.addUserData<uint64_t>("trigBits", candTrigBits);
      myCand.addUserData<uint64_t>("ele1TrigBits", ele1TrigBits);
      myCand.addUserData<uint64_t>("ele2TrigBits", ele2TrigBits);

      // ---- apply the dielectron cut ----
      if (!(dielectronSelection_(myCand) || (resolveAmbiguity_ && doTriElectrons_))) {
        if (DiElectronTrk_ || flipJpsiDirection_ > 0 || (!doTriElectrons_))
          continue;
        else {
          goto TriElectronCand;
        }
      }

      // ---- fit vertex using Kalman vertex fitter ----
      {
        TransientTrack electron1TT((*theTTBuilder).build(it.gsfTrack()));
        TransientTrack electron2TT((*theTTBuilder).build(it2.gsfTrack()));
        t_tks.clear();
        t_tks.push_back(electron1TT);
        t_tks.push_back(electron2TT);

      if (addCommonVertex_) {
        myVertex = vtxFitter.vertex(t_tks);
        vChi2 = myVertex.totalChiSquared();
        vNDF = myVertex.degreesOfFreedom();
        userFloat["vNChi2"] = (vNDF == 0 ? -1 : vChi2 / vNDF);
        userFloat["vProb"] = TMath::Prob(vChi2, (int)rint(vNDF));
        userFloat["chi2"] = vChi2;
        userFloat["ndf"] = vNDF;

        if (myVertex.isValid()) {
          GlobalPoint vtxPos(myVertex.position());
          GlobalError vtxError = myVertex.positionError();
          
          // Convert GlobalError to reco::Vertex::Error (3x3 SMatrix)
          reco::Vertex::Error vtxCov;
          vtxCov(0,0) = vtxError.cxx();
          vtxCov(0,1) = vtxError.cyx(); vtxCov(1,0) = vtxCov(0,1);
          vtxCov(0,2) = vtxError.czx(); vtxCov(2,0) = vtxCov(0,2);
          vtxCov(1,1) = vtxError.cyy();
          vtxCov(1,2) = vtxError.czy(); vtxCov(2,1) = vtxCov(1,2);
          vtxCov(2,2) = vtxError.czz();
          
          userVertex["PCAVtx"] = Vertex(reco::Vertex::Point(vtxPos.x(), vtxPos.y(), vtxPos.z()), 
                                       vtxCov, vChi2, vNDF, 2);
        }
      }
      } // End vertex fitting block

      // store variables in the user area of the candidate
      for (auto const &key : userFloat) myCand.addUserFloat(key.first, key.second);
      for (auto const &key : userInt) myCand.addUserInt(key.first, key.second);
      for (auto const &key : userVertex) myCand.addUserData(key.first, key.second);
      for (auto const &key : userTrack) myCand.addUserData(key.first, key.second);

      oniaOutput->push_back(myCand);

    TriElectronCand:;
      // Do nothing for now - trielectron reconstruction can be added later
    }
  }

skipElectronLoop:;

  // sort candidates by vProb, then by pt
  if (resolveAmbiguity_) {
    std::sort(oniaOutput->begin(), oniaOutput->end(), vPComparator_);
  } else {
    std::sort(oniaOutput->begin(), oniaOutput->end(), pTComparator_);
  }

  iEvent.put(std::move(oniaOutput));
  iEvent.put(std::move(trielectronOutput), "trielectron");
  iEvent.put(std::move(dieletrkOutput), "dieletrk");
}

// ------------ method called once each job just before starting event loop  ------------
void HiOnia2EEPAT::beginJob() {}

// ------------ method called once each job just after ending the event loop  ------------
void HiOnia2EEPAT::endJob() {}

bool HiOnia2EEPAT::isAbHadron(int pdgID) {
  int ID = abs(pdgID);
  return ((ID / 100) % 10 == 5 || (ID / 1000) % 10 == 5);
}

bool HiOnia2EEPAT::isAMixedbHadron(int pdgID, int momPdgID) {
  int ID = abs(pdgID);
  int momID = abs(momPdgID);
  return ((ID / 100) % 10 == 5 && (momID / 100) % 10 == 5 && (ID / 100) != (momID / 100));
}

reco::GenParticleRef HiOnia2EEPAT::findMotherRef(reco::GenParticleRef GenParticle, int GenParticlePDG) {
  if (GenParticle->mother() != NULL) {
    if (GenParticle->mother()->pdgId() != GenParticlePDG) {
      return findMotherRef(GenParticle->motherRef(), GenParticlePDG);
    } else
      return GenParticle->motherRef();
  } else
    return GenParticle;
}

std::pair<int, std::pair<float, float> > HiOnia2EEPAT::findJpsiMCInfo(reco::GenParticleRef genJpsi) {
  int momJpsiID = 0;
  float trueLife = -99.;
  TVector3 trueVtx(0.0, 0.0, 0.0);
  TVector3 trueP(0.0, 0.0, 0.0);
  TVector3 trueVtxMom(0.0, 0.0, 0.0);

  trueVtx.SetXYZ(genJpsi->vertex().x(), genJpsi->vertex().y(), genJpsi->vertex().z());
  trueP.SetXYZ(genJpsi->momentum().x(), genJpsi->momentum().y(), genJpsi->momentum().z());

  if (genJpsi->mother() != NULL) {
    momJpsiID = genJpsi->mother()->pdgId();
    trueVtxMom.SetXYZ(
        genJpsi->mother()->vertex().x(), genJpsi->mother()->vertex().y(), genJpsi->mother()->vertex().z());
  }

  TVector3 vdiff = trueVtx - trueVtxMom;
  if (momJpsiID != 0) {
    trueLife = vdiff.Dot(trueP) * (3.09687 / trueP.Mag()) / 2.99792458e1;
  } else {
    trueLife = 0;
  }

  std::pair<float, float> trueLifePair;
  trueLifePair.first = trueLife;
  trueLifePair.second = 0;
  std::pair<int, std::pair<float, float> > result;
  result.first = momJpsiID;
  result.second = trueLifePair;

  return result;
}

std::string HiOnia2EEPAT::sanitizeLabel(const std::string& raw) const {
  std::string safe(raw);
  std::transform(safe.begin(), safe.end(), safe.begin(), [](unsigned char c) {
    if (std::isalnum(c) || c == '_')
      return static_cast<char>(c);
    return '_';
  });
  return safe;
}

//define this as a plug-in
DEFINE_FWK_MODULE(HiOnia2EEPAT);
