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
// Original Author:  Soohwan Lee
//
//


// system include files
#include <memory>

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/DStar5PProducer.h"

// Constructor
DStar5PProducer::DStar5PProducer(const edm::ParameterSet& iConfig) :
 theVees(iConfig, consumesCollector())
{
  useAnyMVA_ = false;
  if(iConfig.exists("useAnyMVA")) useAnyMVA_ = iConfig.getParameter<bool>("useAnyMVA");
 
  produces<CCC>("DStar5P");
  if(useAnyMVA_) produces<MVACollection>("MVAValuesDStar5P");
}

// (Empty) Destructor
DStar5PProducer::~DStar5PProducer() {
}


//
// Methods
//

// Producer Method
void DStar5PProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
   using namespace edm;

   // Create DStarFitter object which reconstructs the vertices and creates
//   DStarFitter theVees(theParams, iEvent, iSetup);

   theVees.fitAll(iEvent, iSetup);

   // Create auto_ptr for each collection to be stored in the Event
//   std::auto_ptr< reco::VertexCompositeCandidateCollection >
//     d0Candidates( new reco::VertexCompositeCandidateCollection );
//
   auto dStarCandidates = std::make_unique<CCC>();
   dStarCandidates->reserve( theVees.getDStar().size() );

   std::copy( theVees.getDStar().begin(),
              theVees.getDStar().end(),
              std::back_inserter(*dStarCandidates) );

   // Write the collections to the Event
   iEvent.put( std::move(dStarCandidates), std::string("DStar5P") );
    
   if(useAnyMVA_) 
   {
     auto mvas = std::make_unique<MVACollection>(theVees.getMVAVals().begin(),theVees.getMVAVals().end());
     iEvent.put(std::move(mvas), std::string("MVAValuesDStar5P"));
   }

   theVees.resetAll();
}


//void DStar5PProducer::beginJob() {
void DStar5PProducer::beginJob() {
}


void DStar5PProducer::endJob() {
}

//define this as a plug-in
#include "FWCore/PluginManager/interface/ModuleDef.h"

DEFINE_FWK_MODULE(DStar5PProducer);
