// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      BDiLeptonProducer
// 
/**\class BDiLeptonProducer BDiLeptonProducer.cc VertexCompositeAnalysis/VertexCompositeProducer/src/BDiLeptonProducer.cc

 Description: EDProducer for B and Bc meson reconstruction from dileptons

 Implementation:
     Produces B+, B0, and Bc candidates from dilepton+track combinations
     Supports both dimuon (HiOnia2MuMuPAT) and dielectron (HiOnia2EEPAT) inputs
*/

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/BDiLeptonProducer.h"

BDiLeptonProducer::BDiLeptonProducer(const edm::ParameterSet& iConfig) :
  useAnyMVA_(iConfig.getParameter<bool>("useAnyMVA")),
  doBPlus_(iConfig.getParameter<bool>("doBPlus")),
  doBZero_(iConfig.getParameter<bool>("doBZero")),
  doBc_(iConfig.getParameter<bool>("doBc")),
  theVees(iConfig, consumesCollector())
{
  if (doBPlus_) {
    produces<CCC>("BPlus");
    if (useAnyMVA_) produces<MVACollection>("BPlusMVAValues");
  }
  if (doBZero_) {
    produces<CCC>("BZero");
    if (useAnyMVA_) produces<MVACollection>("BZeroMVAValues");
  }
  if (doBc_) {
    produces<CCC>("Bc");
    if (useAnyMVA_) produces<MVACollection>("BcMVAValues");
  }
}

BDiLeptonProducer::~BDiLeptonProducer() {
}

void BDiLeptonProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  theVees.resetAll();
  theVees.fitAll(iEvent, iSetup);

  if (doBPlus_) {
    auto bPlusCands = std::make_unique<CCC>(theVees.getBPlus());
    iEvent.put(std::move(bPlusCands), "BPlus");
    
    if (useAnyMVA_) {
      auto bPlusMVA = std::make_unique<MVACollection>(theVees.getMVAVals());
      iEvent.put(std::move(bPlusMVA), "BPlusMVAValues");
    }
  }

  if (doBZero_) {
    auto bZeroCands = std::make_unique<CCC>(theVees.getBZero());
    iEvent.put(std::move(bZeroCands), "BZero");
    
    if (useAnyMVA_) {
      auto bZeroMVA = std::make_unique<MVACollection>(theVees.getMVAVals());
      iEvent.put(std::move(bZeroMVA), "BZeroMVAValues");
    }
  }

  if (doBc_) {
    auto bcCands = std::make_unique<CCC>(theVees.getBc());
    iEvent.put(std::move(bcCands), "Bc");
    
    if (useAnyMVA_) {
      auto bcMVA = std::make_unique<MVACollection>(theVees.getMVAVals());
      iEvent.put(std::move(bcMVA), "BcMVAValues");
    }
  }
}

void BDiLeptonProducer::beginJob() {
}

void BDiLeptonProducer::endJob() {
}

#include "FWCore/PluginManager/interface/ModuleDef.h"
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(BDiLeptonProducer);
