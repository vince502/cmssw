// -*- C++ -*-
//
// Package:    VertexCompositeProducer
//
// Class:      D04PProducer
// 
/**\class D04PProducer D04PProducer.cc VertexCompositeAnalysis/VertexCompositeProducer/src/D04PProducer.cc

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

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/D04PProducer.h"

// Constructor
D04PProducer::D04PProducer(const edm::ParameterSet& iConfig) :
 theVees(iConfig, consumesCollector())
{
  useAnyMVA_ = false;
  if(iConfig.exists("useAnyMVA")) useAnyMVA_ = iConfig.getParameter<bool>("useAnyMVA");
 
  produces< reco::VertexCompositeCandidateCollection >("D04P");
  if(useAnyMVA_) produces<MVACollection>("MVAValuesD0");
}

// (Empty) Destructor
D04PProducer::~D04PProducer() {
}


//
// Methods
//

// Producer Method
void D04PProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
   using namespace edm;

   // Create D0Fitter object which reconstructs the vertices and creates
//   D0Fitter theVees(theParams, iEvent, iSetup);

   theVees.fitAll(iEvent, iSetup);

   // Create auto_ptr for each collection to be stored in the Event
//   std::auto_ptr< reco::VertexCompositeCandidateCollection >
//     d0Candidates( new reco::VertexCompositeCandidateCollection );
//
   auto d0Candidates = std::make_unique<reco::VertexCompositeCandidateCollection>();
   d0Candidates->reserve( theVees.getD0().size() );

   std::copy( theVees.getD0().begin(),
              theVees.getD0().end(),
              std::back_inserter(*d0Candidates) );

   // Write the collections to the Event
   iEvent.put( std::move(d0Candidates), std::string("D04P") );
    
   if(useAnyMVA_) 
   {
     auto mvas = std::make_unique<MVACollection>(theVees.getMVAVals().begin(),theVees.getMVAVals().end());
     iEvent.put(std::move(mvas), std::string("MVAValuesD04P"));
   }

   theVees.resetAll();
}


//void D04PProducer::beginJob() {
void D04PProducer::beginJob() {
}


void D04PProducer::endJob() {
}

//define this as a plug-in
#include "FWCore/PluginManager/interface/ModuleDef.h"

DEFINE_FWK_MODULE(D04PProducer);
