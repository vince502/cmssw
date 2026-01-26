#ifndef GENTRACKNTUPLIZER_H
#define GENTRACKNTUPLIZER_H

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
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

#include "TTree.h"

// Forward declarations for HepMC
namespace HepMC {
  class GenEvent;
  class GenParticle;
  class GenVertex;
}

class GenTrackNtuplizer : public edm::one::EDAnalyzer<> {
public:
  explicit GenTrackNtuplizer(const edm::ParameterSet&);
  ~GenTrackNtuplizer() override;

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  void clearVectors();

  const edm::EDGetTokenT<reco::GenParticleCollection> genParticleSrc_;
  const edm::EDGetTokenT<edm::HepMCProduct> hepmcSrc_;
  const bool useHepMC_;
  const bool onlyStable_;
  const bool onlyCharged_;

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
  
  // Additional gen-specific variables
  std::vector<int> trkPdgId;
  std::vector<int> trkStatus;
  std::vector<float> trkVx;
  std::vector<float> trkVy;
  std::vector<float> trkVz;
};

#endif
