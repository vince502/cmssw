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
      trackMass_(iConfig.getParameter<double>("trackMass")) {
  produces<pat::CompositeCandidateCollection>("");
  produces<pat::CompositeCandidateCollection>("trielectron");
  produces<pat::CompositeCandidateCollection>("dieletrk");
};

HiOnia2EEPAT::~HiOnia2EEPAT(){};
//
// member functions
//

bool HiOnia2EEPAT::isGoodElectron(const pat::Electron *aElectron, const reco::BeamSpot& beamSpot, const reco::ConversionCollection& conversions) {
  // Basic electron quality cuts
  if (!aElectron->gsfTrack().isNonnull()) return false;
  
  // Track quality requirements
  if (aElectron->gsfTrack()->hitPattern().trackerLayersWithMeasurement() <= 5) return false;
  if (aElectron->gsfTrack()->hitPattern().pixelLayersWithMeasurement() == 0) return false;
  if (fabs(aElectron->gsfTrack()->dxy(RefVtx)) >= 0.3) return false;
  if (fabs(aElectron->gsfTrack()->dz(RefVtx)) >= 20.) return false;
  
  // Conversion rejection - crucial for electron identification
  bool passConvVeto = !ConversionTools::hasMatchedConversion(*aElectron, conversions, beamSpot.position());
  if (!passConvVeto) return false;
  
  // Basic electron ID cuts
  if (!passElectronID(*aElectron)) return false;
  
  return true;
}

bool HiOnia2EEPAT::passElectronID(const pat::Electron& ele) {
  // Basic electron ID requirements similar to those used in ggHiNtuplizer
  // These can be made configurable if needed
  
  // H/E cut
  if (ele.hcalOverEcal() > 0.15) return false;
  
  // Shower shape cut (sigma ieta ieta)
  if (fabs(ele.superCluster()->eta()) <= 1.479) {
    // Barrel
    if (ele.full5x5_sigmaIetaIeta() > 0.012) return false;
  } else {
    // Endcap  
    if (ele.full5x5_sigmaIetaIeta() > 0.035) return false;
  }
  
  // Track-cluster matching cuts
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

      // ---- Store electron-specific information ----
      // Conversion veto results
      userInt["ele1ConvVeto"] = (int)(!ConversionTools::hasMatchedConversion(it, *conversions, theBeamSpot->position()));
      userInt["ele2ConvVeto"] = (int)(!ConversionTools::hasMatchedConversion(it2, *conversions, theBeamSpot->position()));
      
      // SuperCluster variables
      userFloat["ele1SCEta"] = it.superCluster()->eta();
      userFloat["ele2SCEta"] = it2.superCluster()->eta();
      userFloat["ele1SCPhi"] = it.superCluster()->phi();
      userFloat["ele2SCPhi"] = it2.superCluster()->phi();
      userFloat["ele1SCEn"] = it.superCluster()->energy();
      userFloat["ele2SCEn"] = it2.superCluster()->energy();
      
      // Electron ID variables
      userFloat["ele1HoverE"] = it.hcalOverEcal();
      userFloat["ele2HoverE"] = it2.hcalOverEcal();
      userFloat["ele1SigmaIEtaIEta"] = it.full5x5_sigmaIetaIeta();
      userFloat["ele2SigmaIEtaIEta"] = it2.full5x5_sigmaIetaIeta();
      userFloat["ele1DeltaEtaIn"] = it.deltaEtaSuperClusterTrackAtVtx();
      userFloat["ele2DeltaEtaIn"] = it2.deltaEtaSuperClusterTrackAtVtx();
      userFloat["ele1DeltaPhiIn"] = it.deltaPhiSuperClusterTrackAtVtx();
      userFloat["ele2DeltaPhiIn"] = it2.deltaPhiSuperClusterTrackAtVtx();
      
      // Bremsstrahlung
      userFloat["ele1FBrem"] = it.fbrem();
      userFloat["ele2FBrem"] = it2.fbrem();
      
      // E/p ratio
      userFloat["ele1EoverP"] = it.eSuperClusterOverP();
      userFloat["ele2EoverP"] = it2.eSuperClusterOverP();
      
      // PF Isolation
      reco::GsfElectron::PflowIsolationVariables pfIso1 = it.pfIsolationVariables();
      reco::GsfElectron::PflowIsolationVariables pfIso2 = it2.pfIsolationVariables();
      userFloat["ele1PFChIso"] = pfIso1.sumChargedHadronPt;
      userFloat["ele2PFChIso"] = pfIso2.sumChargedHadronPt;
      userFloat["ele1PFNeuIso"] = pfIso1.sumNeutralHadronEt;
      userFloat["ele2PFNeuIso"] = pfIso2.sumNeutralHadronEt;
      userFloat["ele1PFPhoIso"] = pfIso1.sumPhotonEt;
      userFloat["ele2PFPhoIso"] = pfIso2.sumPhotonEt;

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

//define this as a plug-in
DEFINE_FWK_MODULE(HiOnia2EEPAT);