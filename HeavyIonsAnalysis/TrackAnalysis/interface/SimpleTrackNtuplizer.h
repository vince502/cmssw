#ifndef SIMPLETRACKNTUPLIZER_H
#define SIMPLETRACKNTUPLIZER_H

#include <iostream>
#include <vector>

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "TTree.h"

class SimpleTrackNtuplizer : public edm::one::EDAnalyzer<> {
public:
  explicit SimpleTrackNtuplizer(const edm::ParameterSet&);
  ~SimpleTrackNtuplizer() override;

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  void clearVectors();

  const edm::EDGetTokenT<reco::TrackCollection> trackSrc_;

  edm::Service<TFileService> fs;
  TTree* trackTree_;

  int nRun;
  int nEv;
  int nLumi;
  int nTrk;

  std::vector<float> trkPt;
  std::vector<float> trkEta;
  std::vector<float> trkPhi;
  std::vector<float> trkP;
  std::vector<float> trkEnergy;
  std::vector<int> trkCharge;
  std::vector<float> trkChi2;
  std::vector<float> trkNdof;
  std::vector<float> trkNormChi2;
  std::vector<int> trkNHits;
  std::vector<int> trkNPixelHits;
  std::vector<int> trkNStripHits;
  std::vector<bool> trkHighPurity;
  std::vector<float> trkDxy;
  std::vector<float> trkDz;
  std::vector<float> trkPtError;
  std::vector<float> trkEtaError;
  std::vector<float> trkPhiError;
};

#endif
