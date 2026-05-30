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
#include "RecoVertex/AdaptiveVertexFit/interface/AdaptiveVertexFitter.h"
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
  applyConversionVeto_ = iConfig.existsAs<bool>("applyConversionVeto")
                         ? iConfig.getParameter<bool>("applyConversionVeto") : true;

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
  // Conversion veto - configurable (default: true for backwards compatibility)
  if (applyConversionVeto_ && ConversionTools::hasMatchedConversion(*aElectron, conversions, beamSpot.position())) return false;
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

  if (DiElectronTrk_ && doTriElectrons_) {
    cout << "FATAL ERROR: DiElectronTrk_ and doTriElectrons_ cannot be both true ! Change one of them in the config file !"
         << endl;
    return;
  }

  if (flipJpsiDirection_ > 0 && !doTriElectrons_)
    cout << " ***** BEWARE !!! Undefined behaviour when flipJpsiDirection option is true, but not doTriElectrons_ !"
         << endl;

  vector<double> eleMasses;
  eleMasses.push_back(0.0005109989);
  eleMasses.push_back(0.0005109989);

  unique_ptr<pat::CompositeCandidateCollection> oniaOutput(new pat::CompositeCandidateCollection);
  unique_ptr<pat::CompositeCandidateCollection> trielectronOutput(new pat::CompositeCandidateCollection);
  unique_ptr<pat::CompositeCandidateCollection> dieletrkOutput(new pat::CompositeCandidateCollection);

  Vertex thePrimaryV;
  Vertex theBeamSpotV;

  const auto& bField = iSetup.getHandle(magFieldToken_);

  Handle<BeamSpot> theBeamSpot;
  iEvent.getByToken(thebeamspotToken_, theBeamSpot);
  BeamSpot bs = *theBeamSpot;
  theBeamSpotV = Vertex(bs.position(), bs.covariance3D());

  Handle<VertexCollection> priVtxs;
  iEvent.getByToken(thePVsToken_, priVtxs);
  if (priVtxs->begin() != priVtxs->end()) {
    thePrimaryV = Vertex(*(priVtxs->begin()));
  } else {
    thePrimaryV = Vertex(bs.position(), bs.covariance3D());
  }
  RefVtx = thePrimaryV.position();

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

  const auto& theTTBuilder = iSetup.getHandle(trackBuilderToken_);
  KalmanVertexFitter vtxFitter(true);
  TrackCollection electronLess;

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
          ourTracks.push_back(track);
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

  for (int i = 0; i < ourEleNb; i++) {
    const pat::Electron& it = ourElectrons[i];
    for (int j = i + 1; j < ourEleNb; j++) {
      const pat::Electron& it2 = ourElectrons[j];
      if (!(higherPuritySelection_(it) || higherPuritySelection_(it2)))
        continue;
      if (!(it.gsfTrack().isNonnull()) || !(it2.gsfTrack().isNonnull()))
        continue;

      std::map<std::string, int> userInt;
      std::map<std::string, float> userFloat;
      std::map<std::string, reco::Vertex> userVertex;
      std::map<std::string, reco::Track> userTrack;
      Vertex theOriginalPV;
      TransientVertex myVertex;
      CachingVertex<5> VtxForInvMass;
      Measurement1D MassWErr;
      vector<TransientTrack> t_tks;
      float vChi2 = -100, vNDF = 1;

      pat::CompositeCandidate myCand;
      myCand.addDaughter(it, "electron1");
      myCand.addDaughter(it2, "electron2");

      LorentzVector ele1 = it.p4();
      LorentzVector ele2 = it2.p4();
      LorentzVector jpsi = ele1 + ele2;
      myCand.setP4(jpsi);
      myCand.setCharge(it.charge() + it2.charge());

      userInt["ele1ConvVeto"] = static_cast<int>(!ConversionTools::hasMatchedConversion(it, *conversions, theBeamSpot->position()));
      userInt["ele2ConvVeto"] = static_cast<int>(!ConversionTools::hasMatchedConversion(it2, *conversions, theBeamSpot->position()));

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

      if (doTriggerMatch_) {
        myCand.addUserData<uint64_t>("trigBits", candTrigBits);
        myCand.addUserData<uint64_t>("ele1TrigBits", ele1TrigBits);
        myCand.addUserData<uint64_t>("ele2TrigBits", ele2TrigBits);
      }

      if (!(dielectronSelection_(myCand) || (resolveAmbiguity_ && doTriElectrons_))) {
        if (DiElectronTrk_ || flipJpsiDirection_ > 0 || (!doTriElectrons_))
          continue;
        else
          goto TriElectronCand;
      }

      t_tks.clear();
      t_tks.push_back(theTTBuilder->build(it.gsfTrack()));
      t_tks.push_back(theTTBuilder->build(it2.gsfTrack()));

      VtxForInvMass = vtxFitter.vertex(t_tks);
      MassWErr = massCalculator.invariantMass(VtxForInvMass, eleMasses);
      userFloat["MassErr"] = MassWErr.error();

      myVertex = vtxFitter.vertex(t_tks);

      if (myVertex.isValid()) {
        if (resolveAmbiguity_) {
          float minDz = 999999.;

          TwoTrackMinimumDistance ttmd;
          bool status = ttmd.calculate(
              GlobalTrajectoryParameters(
                  GlobalPoint(myVertex.position().x(), myVertex.position().y(), myVertex.position().z()),
                  GlobalVector(myCand.px(), myCand.py(), myCand.pz()),
                  TrackCharge(0),
                  &(*bField)),
              GlobalTrajectoryParameters(GlobalPoint(bs.position().x(), bs.position().y(), bs.position().z()),
                                         GlobalVector(bs.dxdz(), bs.dydz(), 1.),
                                         TrackCharge(0),
                                         &(*bField)));
          float extrapZ = -9E20;
          if (status)
            extrapZ = ttmd.points().first.z();

          for (VertexCollection::const_iterator itv = priVtxs->begin(), itvend = priVtxs->end(); itv != itvend; ++itv) {
            if (itv->isFake() || itv->tracksSize() < 2 || fabs(itv->position().z()) > 25 || itv->position().Rho() > 2)
              continue;
            float deltaZ = fabs(extrapZ - itv->position().z());
            if (deltaZ < minDz) {
              minDz = deltaZ;
              thePrimaryV = Vertex(*itv);
            }
          }
        }

        theOriginalPV = thePrimaryV;

        if (!dielectronSelection_(myCand)) {
          if (DiElectronTrk_ || (!doTriElectrons_) || flipJpsiDirection_ > 0)
            continue;
          else
            goto TriElectronCand;
        }

        electronLess.clear();
        electronLess.reserve(thePrimaryV.tracksSize());
        if (addElectronlessPrimaryVertex_ && thePrimaryV.tracksSize() > 2) {
          const reco::TrackRef ele1Ctf = it.closestCtfTrackRef();
          const reco::TrackRef ele2Ctf = it2.closestCtfTrackRef();

          if (thePrimaryV.hasRefittedTracks()) {
            for (const auto& itRefittedTrack : thePrimaryV.refittedTracks()) {
              const reco::TrackBaseRef original = thePrimaryV.originalTrack(itRefittedTrack);
              if (ele1Ctf.isNonnull() && original.isNonnull() && original.key() == ele1Ctf.key())
                continue;
              if (ele2Ctf.isNonnull() && original.isNonnull() && original.key() == ele2Ctf.key())
                continue;
              if (original.isNonnull())
                electronLess.push_back(*original);
            }
          } else {
            for (std::vector<reco::TrackBaseRef>::const_iterator itPVtrack = thePrimaryV.tracks_begin();
                 itPVtrack != thePrimaryV.tracks_end();
                 ++itPVtrack) {
              if (!itPVtrack->isNonnull())
                continue;
              if (ele1Ctf.isNonnull() && itPVtrack->key() == ele1Ctf.key())
                continue;
              if (ele2Ctf.isNonnull() && itPVtrack->key() == ele2Ctf.key())
                continue;
              electronLess.push_back(**itPVtrack);
            }
          }

          if (electronLess.size() > 1 && electronLess.size() < thePrimaryV.tracksSize()) {
            std::vector<reco::TransientTrack> t_tks_electronless;
            t_tks_electronless.reserve(electronLess.size());
            for (const auto& trk : electronLess) {
              t_tks_electronless.push_back(theTTBuilder->build(trk));
              t_tks_electronless.back().setBeamSpot(bs);
            }
            std::unique_ptr<AdaptiveVertexFitter> theFitter(new AdaptiveVertexFitter());
            TransientVertex pvs = theFitter->vertex(t_tks_electronless, bs);
            if (pvs.isValid()) {
              thePrimaryV = Vertex(pvs);
            } else {
              std::cout << "TransientVertex re-fitted is not valid. Keeping the original PV." << std::endl;
            }
          }
        }

        if (!doTriElectrons_ && !DiElectronTrk_) {
          double vertexWeight = -1., sumPTPV = -1.;
          int countTksOfPV = -1;

          EDConsumerBase::Labels thePVsLabel;
          EDConsumerBase::labelsForToken(thePVsToken_, thePVsLabel);
          if (thePVsLabel.module == (std::string)("offlinePrimaryVertices")) {
            const reco::TrackRef ele1Ctf = it.closestCtfTrackRef();
            const reco::TrackRef ele2Ctf = it2.closestCtfTrackRef();
            try {
              for (reco::Vertex::trackRef_iterator itVtx = theOriginalPV.tracks_begin();
                   itVtx != theOriginalPV.tracks_end();
                   ++itVtx) {
                if (!itVtx->isNonnull())
                  continue;
                const reco::Track& track = **itVtx;
                if (!track.quality(reco::TrackBase::highPurity))
                  continue;
                if (track.pt() < 0.5)
                  continue;

                TransientTrack tt = theTTBuilder->build(track);
                pair<bool, Measurement1D> tkPVdist = IPTools::absoluteImpactParameter3D(tt, theOriginalPV);

                if (!tkPVdist.first)
                  continue;
                if (tkPVdist.second.significance() > 3)
                  continue;
                if (track.ptError() / track.pt() > 0.1)
                  continue;

                if (ele1Ctf.isNonnull() && ele1Ctf.key() == itVtx->key())
                  continue;
                if (ele2Ctf.isNonnull() && ele2Ctf.key() == itVtx->key())
                  continue;

                vertexWeight += theOriginalPV.trackWeight(*itVtx);
                if (theOriginalPV.trackWeight(*itVtx) > 0.5) {
                  countTksOfPV++;
                  sumPTPV += track.pt();
                }
              }
            } catch (std::exception&) {
              std::cout << " Counting tracks from PV, fails! " << std::endl;
              return;
            }
          }
          userInt["countTksOfPV"] = countTksOfPV;
          userFloat["vertexWeight"] = static_cast<float>(vertexWeight);
          userFloat["sumPTPV"] = static_cast<float>(sumPTPV);
        }

        vChi2 = myVertex.totalChiSquared();
        vNDF = myVertex.degreesOfFreedom();
        float vProb = TMath::Prob(vChi2, static_cast<int>(vNDF));

        userFloat["vNChi2"] = (vNDF == 0 ? -1.f : (vChi2 / vNDF));
        userFloat["vProb"] = vProb;
        userFloat["chi2"] = vChi2;
        userFloat["ndf"] = vNDF;

        TVector3 vtx, vtx3D;
        TVector3 pvtx, pvtx3D;
        VertexDistanceXY vdistXY;
        VertexDistance3D vdistXYZ;

        vtx.SetXYZ(myVertex.position().x(), myVertex.position().y(), 0);
        TVector3 pperp(jpsi.px(), jpsi.py(), 0);
        AlgebraicVector3 vpperp(pperp.x(), pperp.y(), 0.);

        vtx3D.SetXYZ(myVertex.position().x(), myVertex.position().y(), myVertex.position().z());
        TVector3 pxyz(jpsi.px(), jpsi.py(), jpsi.pz());
        AlgebraicVector3 vpxyz(pxyz.x(), pxyz.y(), pxyz.z());

        TrajectoryStateClosestToPoint ele1TS = t_tks[0].impactPointTSCP();
        TrajectoryStateClosestToPoint ele2TS = t_tks[1].impactPointTSCP();
        float dca = 1E20;
        if (ele1TS.isValid() && ele2TS.isValid()) {
          ClosestApproachInRPhi cApp;
          cApp.calculate(ele1TS.theState(), ele2TS.theState());
          if (cApp.status())
            dca = cApp.distance();
        }
        userFloat["DCA"] = dca;

        if (addElectronlessPrimaryVertex_) {
          userVertex["electronlessPV"] = thePrimaryV;
          userVertex["PVwithelectrons"] = theOriginalPV;
        } else {
          userVertex["PVwithelectrons"] = thePrimaryV;
        }

        pvtx.SetXYZ(thePrimaryV.position().x(), thePrimaryV.position().y(), 0);
        TVector3 vdiff = vtx - pvtx;
        double cosAlpha = vdiff.Dot(pperp) / (vdiff.Perp() * pperp.Perp());
        Measurement1D distXY = vdistXY.distance(Vertex(myVertex), thePrimaryV);
        double ctauPV = distXY.value() * cosAlpha * 3.096916 / pperp.Perp();
        GlobalError v1e = (Vertex(myVertex)).error();
        GlobalError v2e = thePrimaryV.error();
        AlgebraicSymMatrix33 vXYe = v1e.matrix() + v2e.matrix();
        double ctauErrPV = sqrt(ROOT::Math::Similarity(vpperp, vXYe)) * 3.096916 / (pperp.Perp2());

        userFloat["ppdlPV"] = ctauPV;
        userFloat["ppdlErrPV"] = ctauErrPV;
        userFloat["cosAlpha"] = cosAlpha;

        pvtx3D.SetXYZ(thePrimaryV.position().x(), thePrimaryV.position().y(), thePrimaryV.position().z());
        TVector3 vdiff3D = vtx3D - pvtx3D;
        double cosAlpha3D = vdiff3D.Dot(pxyz) / (vdiff3D.Mag() * pxyz.Mag());
        Measurement1D distXYZ = vdistXYZ.distance(Vertex(myVertex), thePrimaryV);
        double ctauPV3D = distXYZ.value() * cosAlpha3D * 3.096916 / pxyz.Mag();
        double ctauErrPV3D = sqrt(ROOT::Math::Similarity(vpxyz, vXYe)) * 3.096916 / (pxyz.Mag2());

        userFloat["ppdlPV3D"] = ctauPV3D;
        userFloat["ppdlErrPV3D"] = ctauErrPV3D;
        userFloat["cosAlpha3D"] = cosAlpha3D;

        if (addElectronlessPrimaryVertex_ && !doTriElectrons_ && !DiElectronTrk_) {
          pvtx.SetXYZ(theOriginalPV.position().x(), theOriginalPV.position().y(), 0);
          vdiff = vtx - pvtx;
          double cosAlphaOrigPV = vdiff.Dot(pperp) / (vdiff.Perp() * pperp.Perp());
          distXY = vdistXY.distance(Vertex(myVertex), theOriginalPV);
          double ctauOrigPV = distXY.value() * cosAlphaOrigPV * 3.096916 / pperp.Perp();
          GlobalError v1eOrigPV = (Vertex(myVertex)).error();
          GlobalError v2eOrigPV = theOriginalPV.error();
          AlgebraicSymMatrix33 vXYeOrigPV = v1eOrigPV.matrix() + v2eOrigPV.matrix();
          double ctauErrOrigPV = sqrt(ROOT::Math::Similarity(vpperp, vXYeOrigPV)) * 3.096916 / (pperp.Perp2());

          userFloat["ppdlOrigPV"] = ctauOrigPV;
          userFloat["ppdlErrOrigPV"] = ctauErrOrigPV;

          pvtx3D.SetXYZ(theOriginalPV.position().x(), theOriginalPV.position().y(), theOriginalPV.position().z());
          vdiff3D = vtx3D - pvtx3D;
          double cosAlphaOrigPV3D = vdiff3D.Dot(pxyz) / (vdiff3D.Mag() * pxyz.Mag());
          distXYZ = vdistXYZ.distance(Vertex(myVertex), theOriginalPV);
          double ctauOrigPV3D = distXYZ.value() * cosAlphaOrigPV3D * 3.096916 / pxyz.Mag();
          double ctauErrOrigPV3D = sqrt(ROOT::Math::Similarity(vpxyz, vXYeOrigPV)) * 3.096916 / (pxyz.Mag2());

          userFloat["ppdlOrigPV3D"] = ctauOrigPV3D;
          userFloat["ppdlErrOrigPV3D"] = ctauErrOrigPV3D;
        } else {
          userFloat["ppdlOrigPV"] = ctauPV;
          userFloat["ppdlErrOrigPV"] = ctauErrPV;
          userFloat["ppdlOrigPV3D"] = ctauPV3D;
          userFloat["ppdlErrOrigPV3D"] = ctauErrPV3D;
        }

        if (!doTriElectrons_ && !DiElectronTrk_) {
          pvtx.SetXYZ(theBeamSpotV.position().x(), theBeamSpotV.position().y(), 0);
          vdiff = vtx - pvtx;
          cosAlpha = vdiff.Dot(pperp) / (vdiff.Perp() * pperp.Perp());
          distXY = vdistXY.distance(Vertex(myVertex), theBeamSpotV);
          double ctauBS = distXY.value() * cosAlpha * 3.096916 / pperp.Perp();
          GlobalError v1eB = (Vertex(myVertex)).error();
          GlobalError v2eB = theBeamSpotV.error();
          AlgebraicSymMatrix33 vXYeB = v1eB.matrix() + v2eB.matrix();
          double ctauErrBS = sqrt(ROOT::Math::Similarity(vpperp, vXYeB)) * 3.096916 / (pperp.Perp2());

          userFloat["ppdlBS"] = ctauBS;
          userFloat["ppdlErrBS"] = ctauErrBS;
          pvtx3D.SetXYZ(theBeamSpotV.position().x(), theBeamSpotV.position().y(), theBeamSpotV.position().z());
          vdiff3D = vtx3D - pvtx3D;
          cosAlpha3D = vdiff3D.Dot(pxyz) / (vdiff3D.Mag() * pxyz.Mag());
          distXYZ = vdistXYZ.distance(Vertex(myVertex), theBeamSpotV);
          double ctauBS3D = distXYZ.value() * cosAlpha3D * 3.096916 / pxyz.Mag();
          double ctauErrBS3D = sqrt(ROOT::Math::Similarity(vpxyz, vXYeB)) * 3.096916 / (pxyz.Mag2());

          userFloat["ppdlBS3D"] = ctauBS3D;
          userFloat["ppdlErrBS3D"] = ctauErrBS3D;
        }

        userVertex["PCAVtx"] = Vertex(myVertex);
        if (addCommonVertex_) {
          userVertex["commonVertex"] = Vertex(myVertex);
        }
      } else {
        userFloat["vNChi2"] = -1;
        userFloat["vProb"] = -1;
        userFloat["chi2"] = -1;
        userFloat["ndf"] = -1;
        userFloat["vertexWeight"] = -100;
        userFloat["sumPTPV"] = -100;
        userFloat["MassErr"] = -100;
        userFloat["DCA"] = -10;
        userFloat["ppdlPV"] = -100;
        userFloat["ppdlErrPV"] = -100;
        userFloat["cosAlpha"] = -10;
        userFloat["ppdlBS"] = -100;
        userFloat["ppdlErrBS"] = -100;
        userFloat["ppdlOrigPV"] = -100;
        userFloat["ppdlErrOrigPV"] = -100;
        userFloat["ppdlPV3D"] = -100;
        userFloat["ppdlErrPV3D"] = -100;
        userFloat["cosAlpha3D"] = -10;
        userFloat["ppdlBS3D"] = -100;
        userFloat["ppdlErrBS3D"] = -100;
        userFloat["ppdlOrigPV3D"] = -100;
        userFloat["ppdlErrOrigPV3D"] = -100;

        userInt["countTksOfPV"] = -1;

        userVertex["PCAVtx"] = Vertex();
        if (addCommonVertex_) {
          userVertex["commonVertex"] = Vertex();
        }
        if (addElectronlessPrimaryVertex_) {
          userVertex["electronlessPV"] = Vertex();
          userVertex["PVwithelectrons"] = Vertex();
        } else {
          userVertex["PVwithelectrons"] = Vertex();
        }
      }

      if (DiElectronTrk_) {
        userInt["Ntrk"] = Ntrk;
      }

      for (const auto& kv : userFloat) {
        myCand.addUserFloat(kv.first, kv.second);
      }
      for (const auto& kv : userInt) {
        myCand.addUserInt(kv.first, kv.second);
      }
      for (const auto& kv : userVertex) {
        myCand.addUserData(kv.first, kv.second);
      }
      for (const auto& kv : userTrack) {
        myCand.addUserData(kv.first, kv.second);
      }

      if (!LateDiElectronSel_(myCand)) {
        if (DiElectronTrk_ || flipJpsiDirection_ > 0 || (!doTriElectrons_))
          continue;
        else
          goto TriElectronCand;
      }

      oniaOutput->push_back(myCand);

    TriElectronCand:;
    }
  }

skipElectronLoop:;

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
