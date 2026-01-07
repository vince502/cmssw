// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      D04PProducer
// 
/**\class D04PProducer D04PProducer.h VertexCompositeAnalysis/VertexCompositeProducer/interface/D04PProducer.h

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
//  Author: Soohwan Lee
//
//

#ifndef VertexCompositeAnalysis__D0_4P_PRODUCER_H
#define VertexCompositeAnalysis__D0_4P_PRODUCER_H

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

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/D04PFitter.h"

class D04PProducer : public edm::one::EDProducer<> {
public:
  using CC = pat::CompositeCandidate;
  using CCC = pat::CompositeCandidateCollection;
  using MVACollection = std::vector<float>;

  explicit D04PProducer(const edm::ParameterSet&);
  ~D04PProducer();

private:
  //virtual void beginJob() ;
  virtual void beginJob();
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  bool useAnyMVA_;

  D04PFitter theVees; 
//  edm::ParameterSet theParams;
};

#endif
