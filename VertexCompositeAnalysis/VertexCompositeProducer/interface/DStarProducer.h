// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      DStarProducer
// 
/**\class DStarProducer DStarProducer.h VertexCompositeAnalysis/VertexCompositeProducer/interface/DStarProducer.h

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Wei Li 
//
//

#ifndef VertexCompositeAnalysis__DStar_PRODUCER_H
#define VertexCompositeAnalysis__DStar_PRODUCER_H

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

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/DStarFitter.h"

class DStarProducer : public edm::one::EDProducer<> {
public:
  using CC = pat::CompositeCandidate;
  using CCC = pat::CompositeCandidateCollection;
  using MVACollection = std::vector<float>;

  explicit DStarProducer(const edm::ParameterSet&);
  ~DStarProducer();

private:
  //virtual void beginJob() ;
  virtual void beginJob();
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  bool useAnyMVA_;

  DStarFitter theVees; 
//  edm::ParameterSet theParams;
};

#endif
