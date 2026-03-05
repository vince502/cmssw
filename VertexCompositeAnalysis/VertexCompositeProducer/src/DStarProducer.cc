// -*- C++ -*-
//
// Package:    VertexCompositeProducer
//
// Class:      DStarroducer
// 
/**\class DStarProducer DStarProducer.cc VertexCompositeAnalysis/VertexCompositeProducer/src/DStarProducer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Wei Li
//
//


// system include files
#include <memory>

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/DStarProducer.h"

// Constructor
DStarProducer::DStarProducer(const edm::ParameterSet& iConfig) :
 theVees(iConfig, consumesCollector())
{
  useAnyMVA_ = false;
  if(iConfig.exists("useAnyMVA")) useAnyMVA_ = iConfig.getParameter<bool>("useAnyMVA");
 
  produces<CCC>("DStar");
  if(useAnyMVA_) produces<MVACollection>("MVAValuesDStar");
  produces<std::vector<float > >("DCAValuesDStar");
  produces<std::vector<float > >("DCAErrorsDStar");
}

// (Empty) Destructor
DStarProducer::~DStarProducer() {
}


//
// Methods
//

// Producer Method
void DStarProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
   using namespace edm;

   // Create DStarFitter object which reconstructs the vertices and creates
//   DStarFitter theVees(theParams, iEvent, iSetup);

   theVees.fitAll(iEvent, iSetup);

   // Create auto_ptr for each collection to be stored in the Event
//   std::auto_ptr< reco::VertexCompositeCandidateCollection >
//     d0Candidates( new reco::VertexCompositeCandidateCollection );
//
   auto d0Candidates = std::make_unique<CCC>();
   d0Candidates->reserve( theVees.getDStar().size() );

   std::copy( theVees.getDStar().begin(),
              theVees.getDStar().end(),
              std::back_inserter(*d0Candidates) );

   // Write the collections to the Event
   iEvent.put( std::move(d0Candidates), std::string("DStar") );
    
   if(useAnyMVA_) 
   {
     auto mvas = std::make_unique<MVACollection>(theVees.getMVAVals().begin(),theVees.getMVAVals().end());
     iEvent.put(std::move(mvas), std::string("MVAValuesDStar"));
   }
   auto dcaVals = std::make_unique<std::vector<float > >(theVees.getDCAVals().begin(), theVees.getDCAVals().end());
   iEvent.put(std::move(dcaVals), std::string("DCAValuesDStar"));
   auto dcaErrs = std::make_unique<std::vector<float > >(theVees.getDCAErrs().begin(), theVees.getDCAErrs().end());
   iEvent.put(std::move(dcaErrs), std::string("DCAErrorsDStar"));

   theVees.resetAll();
}


//void DStarProducer::beginJob() {
void DStarProducer::beginJob() {
}


void DStarProducer::endJob() {
}

//define this as a plug-in
#include "FWCore/PluginManager/interface/ModuleDef.h"

DEFINE_FWK_MODULE(DStarProducer);
