// -*- C++ -*-
//
// Package:    VertexCompositeProducer
//
// Class:      D0Producer
// 
/**\class D0Producer D0Producer.cc VertexCompositeAnalysis/VertexCompositeProducer/src/D0Producer.cc

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

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/D0Producer.h"

// Constructor
D0Producer::D0Producer(const edm::ParameterSet& iConfig, const ONNXRuntime *cache) :
 theVees(iConfig, consumesCollector(), cache)
{
  useAnyMVA_ = false;
  if(iConfig.exists("useAnyMVA")) useAnyMVA_ = iConfig.getParameter<bool>("useAnyMVA");
 
  produces< CCC >("D0");
  if(useAnyMVA_) produces<MVACollection>("MVAValuesD0");
}

// (Empty) Destructor
D0Producer::~D0Producer() {
}
std::unique_ptr<ONNXRuntime> D0Producer::initializeGlobalCache(const edm::ParameterSet &iConfig) {
   bool useAnyMVA = iConfig.exists("useAnyMVA") ? iConfig.getParameter<bool>("useAnyMVA") : false;
   
   if (!useAnyMVA) return nullptr;
   
   if (iConfig.exists("onnxModelFileName")) {
      // Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ONNXRuntime");
      // Ort::SessionOptions session_options;
      // session_options.SetIntraOpNumThreads(1)
     std::string onnxModelPath = iConfig.getParameter<std::string>("onnxModelFileName");
     
     edm::FileInPath fip(Form("VertexCompositeAnalysis/VertexCompositeProducer/data/%s", onnxModelPath.c_str()));
     std::string fullPath = fip.fullPath();
     
     std::ifstream testFile(fullPath);
     if (!testFile.good()) {
       throw cms::Exception("Configuration") << "cannot find ONNX Model in : " << fullPath;
     }
     testFile.close();
   
      return std::make_unique<ONNXRuntime>(fip.fullPath());
      
   }
   
   return nullptr;
}
 void D0Producer::globalEndJob(const ONNXRuntime *cache) {}



//
// Methods
//

// Producer Method
void D0Producer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
   using namespace edm;

   // Create D0Fitter object which reconstructs the vertices and creates
//   D0Fitter theVees(theParams, iEvent, iSetup);

   theVees.fitAll(iEvent, iSetup);

   // Create auto_ptr for each collection to be stored in the Event
//   std::auto_ptr< reco::VertexCompositeCandidateCollection >
//     d0Candidates( new reco::VertexCompositeCandidateCollection );
//
   auto d0Candidates = std::make_unique<CCC>();
   d0Candidates->reserve( theVees.getD0().size() );

   std::copy( theVees.getD0().begin(),
              theVees.getD0().end(),
              std::back_inserter(*d0Candidates) );

   // Write the collections to the Event
   iEvent.put( std::move(d0Candidates), std::string("D0") );
    
   if(useAnyMVA_) 
   {
     auto mvas = std::make_unique<MVACollection>(theVees.getMVAVals().begin(),theVees.getMVAVals().end());
     iEvent.put(std::move(mvas), std::string("MVAValuesD0"));
   }

   theVees.resetAll();
}


//void D0Producer::beginJob() {
void D0Producer::beginJob() {
}


void D0Producer::endJob() {
}

//define this as a plug-in
#include "FWCore/PluginManager/interface/ModuleDef.h"

DEFINE_FWK_MODULE(D0Producer);
