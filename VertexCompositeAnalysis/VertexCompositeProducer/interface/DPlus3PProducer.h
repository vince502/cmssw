// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      DPlus3PProducer
// 
/**\class DPlus3PProducer DPlus3PProducer.h VertexCompositeAnalysis/VertexCompositeProducer/interface/DPlus3PProducer.h

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Wei Li 
//
//

#ifndef VertexCompositeAnalysis__LAMC3P_PRODUCER_H
#define VertexCompositeAnalysis__LAMC3P_PRODUCER_H

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
#include "DataFormats/Candidate/interface/VertexCompositeCandidate.h"

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/DPlus3PFitter.h"

class DPlus3PProducer : public edm::one::EDProducer<> {
public:
  using MVACollection = std::vector<float>;

  explicit DPlus3PProducer(const edm::ParameterSet&);
  ~DPlus3PProducer();

private:
  //virtual void beginJob() ;
  virtual void beginJob();
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  bool useAnyMVA_;

  DPlus3PFitter theVees; 
//  edm::ParameterSet theParams;
};

#endif
