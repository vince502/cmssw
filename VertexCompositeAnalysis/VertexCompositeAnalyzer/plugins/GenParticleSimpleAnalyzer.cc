// -*- C++ -*-
//
// Package:    VertexCompositeAnalyzer
// Class:      GenParticleSimpleAnalyzer
//
// Description: Simple analyzer for debugging GenParticle collections
//
// Author: Soohwan Lee
//

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

class GenParticleSimpleAnalyzer : public edm::one::EDAnalyzer<> {
public:
  explicit GenParticleSimpleAnalyzer(const edm::ParameterSet&);
  ~GenParticleSimpleAnalyzer() override = default;

private:
  void beginJob() override {}
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override {}

  std::vector<int> pdgIds_;
  edm::EDGetTokenT<reco::GenParticleCollection> tok_genParticle_;
};

GenParticleSimpleAnalyzer::GenParticleSimpleAnalyzer(const edm::ParameterSet& iConfig)
  : pdgIds_(iConfig.getUntrackedParameter<std::vector<int>>("pdgIDs", {})),
    tok_genParticle_(consumes<reco::GenParticleCollection>(
      iConfig.getUntrackedParameter<edm::InputTag>("GenParticleCollection", edm::InputTag("genParticles"))))
{}

void GenParticleSimpleAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup&) {
  using namespace std;
  
  edm::Handle<reco::GenParticleCollection> genParticles;
  iEvent.getByToken(tok_genParticle_, genParticles);
  
  if(!genParticles.isValid()) {
    cout << "GenParticle collection not valid!" << endl;
    return;
  }

  cout << "=== Event " << iEvent.id().event() << " ===" << endl;
  cout << "Total GenParticles: " << genParticles->size() << endl;

  for(const auto& gen : *genParticles) {
    // If pdgIds_ is empty, print all; otherwise filter
    if(!pdgIds_.empty()) {
      auto it = std::find(pdgIds_.begin(), pdgIds_.end(), gen.pdgId());
      if(it == pdgIds_.end()) continue;
    }

    cout << "PDG: " << gen.pdgId() 
         << " | status: " << gen.status()
         << " | pt: " << gen.pt()
         << " | eta: " << gen.eta()
         << " | nDau: " << gen.numberOfDaughters();
    
    if(gen.numberOfDaughters() > 0) {
      cout << " -> (";
      for(unsigned int i = 0; i < gen.numberOfDaughters(); ++i) {
        const auto* dau = gen.daughter(i);
        cout << dau->pdgId();
        if(i < gen.numberOfDaughters() - 1) cout << ", ";
      }
      cout << ")";
    }
    cout << endl;
  }
  cout << endl;
}

DEFINE_FWK_MODULE(GenParticleSimpleAnalyzer);
