#ifndef HiSkim_HiOnia2EE_HiOnia2EEPAT_h
#define HiSkim_HiOnia2EE_HiOnia2EEPAT_h

// system include files
#include <memory>

// FW include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include <CommonTools/RecoAlgos/interface/MassiveCandidateConverter.h>
#include <CommonTools/RecoAlgos/src/TrackToCandidate.h>
#include "CommonTools/Utils/interface/PtComparator.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

// DataFormat includes
#include "DataFormats/Provenance/interface/Provenance.h"
#include <DataFormats/PatCandidates/interface/CompositeCandidate.h>
#include <DataFormats/PatCandidates/interface/Electron.h>
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "CommonTools/Egamma/interface/ConversionTools.h"
#include "CommonTools/Egamma/interface/EffectiveAreas.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"

#include <CommonTools/UtilAlgos/interface/StringCutObjectSelector.h>
#include "RecoVertex/VertexTools/interface/InvariantMassFromVertex.h"

template <typename T>
struct GreaterByVProb {
  typedef T first_argument_type;
  typedef T second_argument_type;
  bool operator()(const T& t1, const T& t2) const { return t1.userFloat("vProb") > t2.userFloat("vProb"); }
};

//
// class declaration
//

class HiOnia2EEPAT : public edm::stream::EDProducer<> {
public:
  explicit HiOnia2EEPAT(const edm::ParameterSet&);
  ~HiOnia2EEPAT() override;

private:
  virtual void beginJob();
  void produce(edm::Event&, const edm::EventSetup&) override;
  virtual void endJob();
  bool isGoodElectron(const pat::Electron*, const reco::BeamSpot&, const reco::ConversionCollection&);
  bool passElectronID(const pat::Electron&);
  const reco::TrackBase::Point rotatePoint(reco::TrackBase::Point PV, reco::TrackBase::Point TrkPoint, int flipJpsi);
  const reco::TrackBase::Vector rotateMomentum(reco::Track trk, int flipJpsi);
  bool isAbHadron(int pdgID);
  bool isAMixedbHadron(int pdgID, int momPdgID);
  reco::GenParticleRef findMotherRef(reco::GenParticleRef GenParticle, int GenParticlePDG);
  std::pair<int, std::pair<float, float> > findJpsiMCInfo(reco::GenParticleRef genJpsi);

  // ----------member data ---------------------------
private:
  edm::EDGetTokenT<edm::View<pat::Electron> > electronsToken_;
  edm::EDGetTokenT<reco::BeamSpot> thebeamspotToken_;
  edm::EDGetTokenT<reco::VertexCollection> thePVsToken_;
  edm::EDGetTokenT<reco::TrackCollection> recoTracksToken_;
  edm::EDGetTokenT<reco::GenParticleCollection> theGenParticlesToken_;
  edm::EDGetTokenT<reco::ConversionCollection> conversionsToken_;
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> magFieldToken_;
  edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> trackBuilderToken_;
  StringCutObjectSelector<pat::Electron> higherPuritySelection_;
  StringCutObjectSelector<pat::Electron> lowerPuritySelection_;
  StringCutObjectSelector<reco::Candidate, true> dielectronSelection_;
  StringCutObjectSelector<reco::Candidate, true> DiEleTrkSelection_;
  StringCutObjectSelector<reco::Candidate, true> trielectronSelection_;
  StringCutObjectSelector<reco::Candidate, true> LateDiElectronSel_;
  StringCutObjectSelector<reco::Candidate, true> LateDiEleTrkSel_;
  StringCutObjectSelector<reco::Candidate, true> LateTriElectronSel_;
  bool addCommonVertex_, addElectronlessPrimaryVertex_;
  bool resolveAmbiguity_;
  bool onlyGoodElectrons_;
  bool onlySingleElectrons_;
  bool doTriElectrons_;
  bool DiElectronTrk_;
  int flipJpsiDirection_;
  converter::TrackToCandidate Converter_;
  int trackType_;
  double trackMass_;
  GreaterByPt<pat::CompositeCandidate> pTComparator_;
  GreaterByVProb<pat::CompositeCandidate> vPComparator_;

  InvariantMassFromVertex massCalculator;
  math::XYZPoint RefVtx;
};

#endif