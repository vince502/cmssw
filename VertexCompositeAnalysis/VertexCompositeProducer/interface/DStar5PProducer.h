// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      DStar5PProducer
// 
/**\class DStar5PProducer DStar5PProducer.h VertexCompositeAnalysis/VertexCompositeProducer/interface/DStar5PProducer.h

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Class Author:  Soohwan Lee
//
//

#ifndef VertexCompositeAnalysis__DStar_5P_PRODUCER_H
#define VertexCompositeAnalysis__DStar_5P_PRODUCER_H

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

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/DStar5PFitter.h"

class DStar5PProducer : public edm::one::EDProducer<> {
public:
  using CC = pat::CompositeCandidate;
  using CCC = pat::CompositeCandidateCollection;
  using MVACollection = std::vector<float>;

  explicit DStar5PProducer(const edm::ParameterSet&);
  ~DStar5PProducer();

private:
  //virtual void beginJob() ;
  virtual void beginJob();
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  bool useAnyMVA_;

  DStar5PFitter theVees; 
//  edm::ParameterSet theParams;
};

#endif
