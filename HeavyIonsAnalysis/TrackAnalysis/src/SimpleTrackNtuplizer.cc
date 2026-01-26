#include "HeavyIonsAnalysis/TrackAnalysis/interface/SimpleTrackNtuplizer.h"

SimpleTrackNtuplizer::SimpleTrackNtuplizer(const edm::ParameterSet& iConfig)
    : trackSrc_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("trackSrc"))) {
}

SimpleTrackNtuplizer::~SimpleTrackNtuplizer() {}

void SimpleTrackNtuplizer::beginJob() {
  fs->file().cd();
  trackTree_ = fs->make<TTree>("trackTree", "Track tree");

  trackTree_->Branch("nRun", &nRun);
  trackTree_->Branch("nEv", &nEv);
  trackTree_->Branch("nLumi", &nLumi);
  trackTree_->Branch("nTrk", &nTrk);

  trackTree_->Branch("trkPt", &trkPt);
  trackTree_->Branch("trkEta", &trkEta);
  trackTree_->Branch("trkPhi", &trkPhi);
  trackTree_->Branch("trkP", &trkP);
  trackTree_->Branch("trkEnergy", &trkEnergy);
  trackTree_->Branch("trkCharge", &trkCharge);
  trackTree_->Branch("trkChi2", &trkChi2);
  trackTree_->Branch("trkNdof", &trkNdof);
  trackTree_->Branch("trkNormChi2", &trkNormChi2);
  trackTree_->Branch("trkNHits", &trkNHits);
  trackTree_->Branch("trkNPixelHits", &trkNPixelHits);
  trackTree_->Branch("trkNStripHits", &trkNStripHits);
  trackTree_->Branch("trkHighPurity", &trkHighPurity);
  trackTree_->Branch("trkDxy", &trkDxy);
  trackTree_->Branch("trkDz", &trkDz);
  trackTree_->Branch("trkPtError", &trkPtError);
  trackTree_->Branch("trkEtaError", &trkEtaError);
  trackTree_->Branch("trkPhiError", &trkPhiError);
}

void SimpleTrackNtuplizer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  nEv = (int)iEvent.id().event();
  nRun = (int)iEvent.id().run();
  nLumi = (int)iEvent.luminosityBlock();

  clearVectors();

  const auto& tracks = iEvent.get(trackSrc_);

  nTrk = tracks.size();

  for (const auto& track : tracks) {
    trkPt.push_back(track.pt());
    trkEta.push_back(track.eta());
    trkPhi.push_back(track.phi());
    trkP.push_back(track.p());
    trkEnergy.push_back(track.p() * cosh(track.eta()));  // approximate energy
    trkCharge.push_back(track.charge());
    trkChi2.push_back(track.chi2());
    trkNdof.push_back(track.ndof());
    trkNormChi2.push_back(track.normalizedChi2());
    trkNHits.push_back(track.numberOfValidHits());
    trkNPixelHits.push_back(track.hitPattern().numberOfValidPixelHits());
    trkNStripHits.push_back(track.hitPattern().numberOfValidStripHits());
    trkHighPurity.push_back(track.quality(reco::TrackBase::qualityByName("highPurity")));
    trkDxy.push_back(track.dxy());
    trkDz.push_back(track.dz());
    trkPtError.push_back(track.ptError());
    trkEtaError.push_back(track.etaError());
    trkPhiError.push_back(track.phiError());
  }

  trackTree_->Fill();
}

void SimpleTrackNtuplizer::endJob() {}

void SimpleTrackNtuplizer::clearVectors() {
  nTrk = 0;
  trkPt.clear();
  trkEta.clear();
  trkPhi.clear();
  trkP.clear();
  trkEnergy.clear();
  trkCharge.clear();
  trkChi2.clear();
  trkNdof.clear();
  trkNormChi2.clear();
  trkNHits.clear();
  trkNPixelHits.clear();
  trkNStripHits.clear();
  trkHighPurity.clear();
  trkDxy.clear();
  trkDz.clear();
  trkPtError.clear();
  trkEtaError.clear();
  trkPhiError.clear();
}

DEFINE_FWK_MODULE(SimpleTrackNtuplizer);
