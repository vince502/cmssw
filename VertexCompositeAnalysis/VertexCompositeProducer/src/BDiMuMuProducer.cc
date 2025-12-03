// -*- C++ -*-
//
// Package:    VertexCompositeProducer
//
// Class:      BDiMuMuProducer
// 
/**\class BDiMuMuProducer BDiMuMuProducer.cc VertexCompositeAnalysis/VertexCompositeProducer/src/BDiMuMuProducer.cc

 Description: EDProducer for B meson reconstruction from dimuons and additional tracks

 Implementation:
     Produces B+ and B0 meson candidates by combining dimuon resonances with kaon tracks
*/
//
// Original Author:  Based on existing producers
//
//

// system include files
#include <memory>

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/BDiMuMuProducer.h"

// Constructor
BDiMuMuProducer::BDiMuMuProducer(const edm::ParameterSet& iConfig) :
 theVees(iConfig, consumesCollector())
{
  useAnyMVA_ = false;
  if(iConfig.exists("useAnyMVA")) useAnyMVA_ = iConfig.getParameter<bool>("useAnyMVA");
  
  doBPlus_ = false;
  if(iConfig.exists("doBPlus")) doBPlus_ = iConfig.getParameter<bool>("doBPlus");
  
  doBZero_ = false;
  if(iConfig.exists("doBZero")) doBZero_ = iConfig.getParameter<bool>("doBZero");
  
  doBc_ = false;
  if(iConfig.exists("doBc")) doBc_ = iConfig.getParameter<bool>("doBc");

  // Produce B+ collection if enabled
  if(doBPlus_) {
    produces<CCC>("BPlus");
    if(useAnyMVA_) produces<MVACollection>("MVAValuesBPlus");
  }
  
  // Produce B0 collection if enabled
  if(doBZero_) {
    produces<CCC>("BZero");
    if(useAnyMVA_) produces<MVACollection>("MVAValuesBZero");
  }
  
  // Produce Bc collection if enabled
  if(doBc_) {
    produces<CCC>("Bc");
    if(useAnyMVA_) produces<MVACollection>("MVAValuesBc");
  }
}

// (Empty) Destructor
BDiMuMuProducer::~BDiMuMuProducer() {
}

// Methods

// Producer Method
void BDiMuMuProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
   using namespace edm;

   // Create BDiMuMuFitter object which reconstructs the vertices and creates
   // B meson candidates
   theVees.fitAll(iEvent, iSetup);

   // Create and fill B+ collection
   if(doBPlus_) {
     auto bPlusCandidates = std::make_unique<CCC>();
     bPlusCandidates->reserve(theVees.getBPlus().size());

     std::copy(theVees.getBPlus().begin(),
               theVees.getBPlus().end(),
               std::back_inserter(*bPlusCandidates));

     // Write B+ collection to the Event
     iEvent.put(std::move(bPlusCandidates), std::string("BPlus"));
     
     if(useAnyMVA_) {
       auto mvas = std::make_unique<MVACollection>(theVees.getMVAVals().begin(), theVees.getMVAVals().end());
       iEvent.put(std::move(mvas), std::string("MVAValuesBPlus"));
     }
   }

   // Create and fill B0 collection
   if(doBZero_) {
     auto bZeroCandidates = std::make_unique<CCC>();
     bZeroCandidates->reserve(theVees.getBZero().size());

     std::copy(theVees.getBZero().begin(),
               theVees.getBZero().end(),
               std::back_inserter(*bZeroCandidates));

     // Write B0 collection to the Event
     iEvent.put(std::move(bZeroCandidates), std::string("BZero"));
     
     if(useAnyMVA_) {
       auto mvas = std::make_unique<MVACollection>(theVees.getMVAVals().begin(), theVees.getMVAVals().end());
       iEvent.put(std::move(mvas), std::string("MVAValuesBZero"));
     }
   }

   // Create and fill Bc collection
   if(doBc_) {
     auto bcCandidates = std::make_unique<CCC>();
     bcCandidates->reserve(theVees.getBc().size());

     std::copy(theVees.getBc().begin(),
               theVees.getBc().end(),
               std::back_inserter(*bcCandidates));

     // Write Bc collection to the Event
     iEvent.put(std::move(bcCandidates), std::string("Bc"));
     
     if(useAnyMVA_) {
       auto mvas = std::make_unique<MVACollection>(theVees.getMVAVals().begin(), theVees.getMVAVals().end());
       iEvent.put(std::move(mvas), std::string("MVAValuesBc"));
     }
   }

   theVees.resetAll();
}

void BDiMuMuProducer::beginJob() {
}

void BDiMuMuProducer::endJob() {
}

//define this as a plug-in
#include "FWCore/PluginManager/interface/ModuleDef.h"

DEFINE_FWK_MODULE(BDiMuMuProducer);