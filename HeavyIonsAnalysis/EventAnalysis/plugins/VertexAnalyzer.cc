// -*- C++ -*-
//
// Package:    HeavyIonsAnalysis/EventAnalysis
// Class:      VertexAnalyzer
//
// Description: Detailed primary vertex analyzer storing all valid PVs
//              Provides AoS structure for position, quality, track statistics
//
// Author: Soohwan Lee
//

#include <memory>
#include <vector>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include <TTree.h>
#include <TMath.h>

class VertexAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit VertexAnalyzer(const edm::ParameterSet&);
  ~VertexAnalyzer() override = default;

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override {}

  // Tokens
  edm::EDGetTokenT<reco::VertexCollection> vertexToken_;
  edm::EDGetTokenT<reco::BeamSpot> beamSpotToken_;

  // Tree
  TTree* tree_;

  // Event info
  UInt_t run_;
  UInt_t lumi_;
  ULong64_t event_;

  // Beam spot
  Float_t beamSpotX_;
  Float_t beamSpotY_;
  Float_t beamSpotZ_;
  Float_t beamSpotXErr_;
  Float_t beamSpotYErr_;
  Float_t beamSpotZErr_;
  Float_t beamSpotSigmaZ_;
  Float_t beamSpotdxdz_;
  Float_t beamSpotdydz_;
  Float_t beamSpotWidthX_;
  Float_t beamSpotWidthY_;

  // Number of vertices
  Int_t nVtx_;           // All vertices
  Int_t nGoodVtx_;       // Non-fake, valid vertices

  // Vertex arrays (AoS style stored as SoA for ROOT)
  static const int maxVtx_ = 100;
  
  // Position
  Float_t vtxX_[maxVtx_];
  Float_t vtxY_[maxVtx_];
  Float_t vtxZ_[maxVtx_];
  
  // Position errors
  Float_t vtxXErr_[maxVtx_];
  Float_t vtxYErr_[maxVtx_];
  Float_t vtxZErr_[maxVtx_];
  
  // Covariance (off-diagonal)
  Float_t vtxXYCov_[maxVtx_];
  Float_t vtxXZCov_[maxVtx_];
  Float_t vtxYZCov_[maxVtx_];
  
  // Quality
  Float_t vtxChi2_[maxVtx_];
  Float_t vtxNdof_[maxVtx_];
  Float_t vtxNormChi2_[maxVtx_];  // chi2/ndof
  Float_t vtxProb_[maxVtx_];      // TMath::Prob(chi2, ndof)
  
  // Track statistics
  Int_t vtxNTracks_[maxVtx_];            // tracksSize()
  Int_t vtxNTracksW05_[maxVtx_];         // tracks with weight > 0.5
  Float_t vtxSumPt_[maxVtx_];            // sum of track pT
  Float_t vtxSumPt2_[maxVtx_];           // sum of track pT^2
  Float_t vtxAvgTrackPt_[maxVtx_];       // average track pT
  Float_t vtxMaxTrackPt_[maxVtx_];       // max track pT
  
  // Flags
  Bool_t vtxIsFake_[maxVtx_];
  Bool_t vtxIsValid_[maxVtx_];
  Bool_t vtxIsGood_[maxVtx_];  // !isFake && isValid && ndof > 4 && |z| < 24 && rho < 2
  
  // Index of best vertex (highest sumPt2)
  Int_t bestVtxIdx_;
};

VertexAnalyzer::VertexAnalyzer(const edm::ParameterSet& iConfig) {
  usesResource("TFileService");
  
  vertexToken_ = consumes<reco::VertexCollection>(
      iConfig.getParameter<edm::InputTag>("vertexSrc"));
  beamSpotToken_ = consumes<reco::BeamSpot>(
      iConfig.getParameter<edm::InputTag>("beamSpotSrc"));
}

void VertexAnalyzer::beginJob() {
  edm::Service<TFileService> fs;
  tree_ = fs->make<TTree>("vtxTree", "Primary Vertex Tree");

  // Event info
  tree_->Branch("run", &run_, "run/i");
  tree_->Branch("lumi", &lumi_, "lumi/i");
  tree_->Branch("event", &event_, "event/l");

  // Beam spot
  tree_->Branch("beamSpotX", &beamSpotX_, "beamSpotX/F");
  tree_->Branch("beamSpotY", &beamSpotY_, "beamSpotY/F");
  tree_->Branch("beamSpotZ", &beamSpotZ_, "beamSpotZ/F");
  tree_->Branch("beamSpotXErr", &beamSpotXErr_, "beamSpotXErr/F");
  tree_->Branch("beamSpotYErr", &beamSpotYErr_, "beamSpotYErr/F");
  tree_->Branch("beamSpotZErr", &beamSpotZErr_, "beamSpotZErr/F");
  tree_->Branch("beamSpotSigmaZ", &beamSpotSigmaZ_, "beamSpotSigmaZ/F");
  tree_->Branch("beamSpotdxdz", &beamSpotdxdz_, "beamSpotdxdz/F");
  tree_->Branch("beamSpotdydz", &beamSpotdydz_, "beamSpotdydz/F");
  tree_->Branch("beamSpotWidthX", &beamSpotWidthX_, "beamSpotWidthX/F");
  tree_->Branch("beamSpotWidthY", &beamSpotWidthY_, "beamSpotWidthY/F");

  // Vertex counts
  tree_->Branch("nVtx", &nVtx_, "nVtx/I");
  tree_->Branch("nGoodVtx", &nGoodVtx_, "nGoodVtx/I");
  tree_->Branch("bestVtxIdx", &bestVtxIdx_, "bestVtxIdx/I");

  // Position
  tree_->Branch("vtxX", vtxX_, "vtxX[nGoodVtx]/F");
  tree_->Branch("vtxY", vtxY_, "vtxY[nGoodVtx]/F");
  tree_->Branch("vtxZ", vtxZ_, "vtxZ[nGoodVtx]/F");

  // Position errors
  tree_->Branch("vtxXErr", vtxXErr_, "vtxXErr[nGoodVtx]/F");
  tree_->Branch("vtxYErr", vtxYErr_, "vtxYErr[nGoodVtx]/F");
  tree_->Branch("vtxZErr", vtxZErr_, "vtxZErr[nGoodVtx]/F");

  // Covariance
  tree_->Branch("vtxXYCov", vtxXYCov_, "vtxXYCov[nGoodVtx]/F");
  tree_->Branch("vtxXZCov", vtxXZCov_, "vtxXZCov[nGoodVtx]/F");
  tree_->Branch("vtxYZCov", vtxYZCov_, "vtxYZCov[nGoodVtx]/F");

  // Quality
  tree_->Branch("vtxChi2", vtxChi2_, "vtxChi2[nGoodVtx]/F");
  tree_->Branch("vtxNdof", vtxNdof_, "vtxNdof[nGoodVtx]/F");
  tree_->Branch("vtxNormChi2", vtxNormChi2_, "vtxNormChi2[nGoodVtx]/F");
  tree_->Branch("vtxProb", vtxProb_, "vtxProb[nGoodVtx]/F");

  // Track statistics
  tree_->Branch("vtxNTracks", vtxNTracks_, "vtxNTracks[nGoodVtx]/I");
  tree_->Branch("vtxNTracksW05", vtxNTracksW05_, "vtxNTracksW05[nGoodVtx]/I");
  tree_->Branch("vtxSumPt", vtxSumPt_, "vtxSumPt[nGoodVtx]/F");
  tree_->Branch("vtxSumPt2", vtxSumPt2_, "vtxSumPt2[nGoodVtx]/F");
  tree_->Branch("vtxAvgTrackPt", vtxAvgTrackPt_, "vtxAvgTrackPt[nGoodVtx]/F");
  tree_->Branch("vtxMaxTrackPt", vtxMaxTrackPt_, "vtxMaxTrackPt[nGoodVtx]/F");

  // Flags
  tree_->Branch("vtxIsFake", vtxIsFake_, "vtxIsFake[nGoodVtx]/O");
  tree_->Branch("vtxIsValid", vtxIsValid_, "vtxIsValid[nGoodVtx]/O");
  tree_->Branch("vtxIsGood", vtxIsGood_, "vtxIsGood[nGoodVtx]/O");
}

void VertexAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Event info
  run_ = iEvent.id().run();
  lumi_ = iEvent.luminosityBlock();
  event_ = iEvent.id().event();

  // Beam spot
  edm::Handle<reco::BeamSpot> beamSpot;
  iEvent.getByToken(beamSpotToken_, beamSpot);
  
  if (beamSpot.isValid()) {
    beamSpotX_ = beamSpot->x0();
    beamSpotY_ = beamSpot->y0();
    beamSpotZ_ = beamSpot->z0();
    beamSpotXErr_ = beamSpot->x0Error();
    beamSpotYErr_ = beamSpot->y0Error();
    beamSpotZErr_ = beamSpot->z0Error();
    beamSpotSigmaZ_ = beamSpot->sigmaZ();
    beamSpotdxdz_ = beamSpot->dxdz();
    beamSpotdydz_ = beamSpot->dydz();
    beamSpotWidthX_ = beamSpot->BeamWidthX();
    beamSpotWidthY_ = beamSpot->BeamWidthY();
  } else {
    beamSpotX_ = beamSpotY_ = beamSpotZ_ = 0;
    beamSpotXErr_ = beamSpotYErr_ = beamSpotZErr_ = 0;
    beamSpotSigmaZ_ = beamSpotdxdz_ = beamSpotdydz_ = 0;
    beamSpotWidthX_ = beamSpotWidthY_ = 0;
  }

  // Vertices
  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(vertexToken_, vertices);

  nVtx_ = 0;
  nGoodVtx_ = 0;
  bestVtxIdx_ = -1;
  float maxSumPt2 = -1;

  if (vertices.isValid()) {
    nVtx_ = vertices->size();

    for (size_t iv = 0; iv < vertices->size() && nGoodVtx_ < maxVtx_; ++iv) {
      const reco::Vertex& vtx = vertices->at(iv);

      // Skip fake vertices
      if (vtx.isFake()) continue;
      if (!vtx.isValid()) continue;

      int idx = nGoodVtx_;

      // Position
      vtxX_[idx] = vtx.x();
      vtxY_[idx] = vtx.y();
      vtxZ_[idx] = vtx.z();

      // Position errors
      vtxXErr_[idx] = vtx.xError();
      vtxYErr_[idx] = vtx.yError();
      vtxZErr_[idx] = vtx.zError();

      // Covariance
      vtxXYCov_[idx] = vtx.covariance(0, 1);
      vtxXZCov_[idx] = vtx.covariance(0, 2);
      vtxYZCov_[idx] = vtx.covariance(1, 2);

      // Quality
      vtxChi2_[idx] = vtx.chi2();
      vtxNdof_[idx] = vtx.ndof();
      vtxNormChi2_[idx] = (vtx.ndof() > 0) ? vtx.chi2() / vtx.ndof() : -1;
      vtxProb_[idx] = (vtx.ndof() > 0) ? TMath::Prob(vtx.chi2(), vtx.ndof()) : -1;

      // Track statistics
      vtxNTracks_[idx] = vtx.tracksSize();
      
      int nTracksW05 = 0;
      float sumPt = 0, sumPt2 = 0, maxPt = 0;
      
      for (auto trk = vtx.tracks_begin(); trk != vtx.tracks_end(); ++trk) {
        float w = vtx.trackWeight(*trk);
        if (w > 0.5) nTracksW05++;
        
        float pt = (*trk)->pt();
        sumPt += pt;
        sumPt2 += pt * pt;
        if (pt > maxPt) maxPt = pt;
      }

      vtxNTracksW05_[idx] = nTracksW05;
      vtxSumPt_[idx] = sumPt;
      vtxSumPt2_[idx] = sumPt2;
      vtxAvgTrackPt_[idx] = (vtxNTracks_[idx] > 0) ? sumPt / vtxNTracks_[idx] : 0;
      vtxMaxTrackPt_[idx] = maxPt;

      // Flags
      vtxIsFake_[idx] = vtx.isFake();
      vtxIsValid_[idx] = vtx.isValid();
      
      // "Good" vertex criteria
      float rho = std::sqrt(vtx.x() * vtx.x() + vtx.y() * vtx.y());
      vtxIsGood_[idx] = (!vtx.isFake() && vtx.isValid() && 
                         vtx.ndof() > 4 && std::abs(vtx.z()) < 24 && rho < 2);

      // Track best vertex
      if (sumPt2 > maxSumPt2) {
        maxSumPt2 = sumPt2;
        bestVtxIdx_ = idx;
      }

      nGoodVtx_++;
    }
  }

  tree_->Fill();
}

DEFINE_FWK_MODULE(VertexAnalyzer);
