// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      D0Producer
// 
/**\class D0Producer D0Producer.h VertexCompositeAnalysis/VertexCompositeProducer/interface/D0Producer.h

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Wei Li 
//
//

#ifndef VertexCompositeAnalysis__D0_PRODUCER_H
#define VertexCompositeAnalysis__D0_PRODUCER_H

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
// #include "DataFormats/Candidate/interface/VertexCompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/D0Fitter.h"

#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "PhysicsTools/ONNXRuntime/interface/ONNXRuntime.h"


class D0Producer : public edm::stream::EDProducer<edm::GlobalCache<ONNXRuntime>> {
public:
  using CC = pat::CompositeCandidate;
  using CCC = pat::CompositeCandidateCollection;
  using MVACollection = std::vector<float>;

  explicit D0Producer(const edm::ParameterSet&,const ONNXRuntime *);
  static std::unique_ptr<ONNXRuntime> initializeGlobalCache(const edm::ParameterSet &);
  static void globalEndJob(const ONNXRuntime *);
  ~D0Producer();

private:
  //virtual void beginJob() ;
  virtual void beginJob();
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  bool useAnyMVA_;
  std::string onnxModelPath_;

  D0Fitter theVees; 
//  edm::ParameterSet theParams;
};

#endif
