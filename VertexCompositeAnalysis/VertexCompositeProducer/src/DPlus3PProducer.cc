// -*- C++ -*-
//
// Package:    VertexCompositeProducer
//
// Class:      DPlus3PProducer
// 
/**\class DPlus3PProducer DPlus3PProducer.cc VertexCompositeAnalysis/VertexCompositeProducer/src/DPlus3PProducer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Wei Li
//
//
//


// system include files
#include <memory>

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/DPlus3PProducer.h"

// Constructor
DPlus3PProducer::DPlus3PProducer(const edm::ParameterSet& iConfig) :
 theVees(iConfig, consumesCollector())
{
  useAnyMVA_ = false;
  if(iConfig.exists("useAnyMVA")) useAnyMVA_ = iConfig.getParameter<bool>("useAnyMVA");
 
  produces< reco::VertexCompositeCandidateCollection >("DPlus3P");
  if(useAnyMVA_) produces<MVACollection>("MVAValuesDPlus3P");
  produces<std::vector<float > >("DCAValuesDPlus3P");
  produces<std::vector<float > >("DCAErrorsDPlus3P");
}

// (Empty) Destructor
DPlus3PProducer::~DPlus3PProducer() {
}


//
// Methods
//

// Producer Method
void DPlus3PProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
   using namespace edm;
   using std::cout;
   using std::endl;

   // Create DPlus3PFitter object which reconstructs the vertices and creates
//   DPlus3PFitter theVees(theParams, iEvent, iSetup);

   theVees.fitAll(iEvent, iSetup);

   // Create auto_ptr for each collection to be stored in the Event
//   std::auto_ptr< reco::VertexCompositeCandidateCollection >
//     DPlusCandidates( new reco::VertexCompositeCandidateCollection );
//
   auto DPlusCandidates = std::make_unique<reco::VertexCompositeCandidateCollection>();
   DPlusCandidates->reserve( theVees.getDPlus3P().size() );

   std::copy( theVees.getDPlus3P().begin(),
              theVees.getDPlus3P().end(),
              std::back_inserter(*DPlusCandidates) );

   // Write the collections to the Event
   iEvent.put( std::move(DPlusCandidates), std::string("DPlus3P") );
    
   if(useAnyMVA_) 
   {
     auto mvas = std::make_unique<MVACollection>(theVees.getMVAVals().begin(),theVees.getMVAVals().end());
     iEvent.put(std::move(mvas), std::string("MVAValuesDPlus3P"));
   }
   auto dcaVals = std::make_unique<std::vector<float > >(theVees.getDCAVals().begin(), theVees.getDCAVals().end());
   iEvent.put(std::move(dcaVals), std::string("DCAValuesDPlus3P"));
   auto dcaErrs = std::make_unique<std::vector<float > >(theVees.getDCAErrs().begin(), theVees.getDCAErrs().end());
   iEvent.put(std::move(dcaErrs), std::string("DCAErrorsDPlus3P"));

   theVees.resetAll();
}


//void DPlus3PProducer::beginJob() {
void DPlus3PProducer::beginJob() {
}


void DPlus3PProducer::endJob() {
}

//define this as a plug-in
#include "FWCore/PluginManager/interface/ModuleDef.h"

DEFINE_FWK_MODULE(DPlus3PProducer);
