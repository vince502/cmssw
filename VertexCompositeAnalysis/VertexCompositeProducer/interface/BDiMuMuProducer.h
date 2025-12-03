// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      BDiMuMuProducer
// 
/**\class BDiMuMuProducer BDiMuMuProducer.h VertexCompositeAnalysis/VertexCompositeProducer/interface/BDiMuMuProducer.h

 Description: EDProducer for B and Bc meson reconstruction from dimuons and additional tracks

 Implementation:
     Uses BDiMuMuFitter to reconstruct B+ -> J/psi K+, B0 -> J/psi K*0, and Bc+ -> J/psi pi+ decays
     Produces collections of B and Bc meson candidates with associated MVA values
*/
//
// Original Author:  Based on existing producers, enhanced for dimuon+track combinations
//
//

#ifndef VertexCompositeAnalysis__BDIMUMUPRODUCER_H
#define VertexCompositeAnalysis__BDIMUMUPRODUCER_H

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
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/BDiMuMuFitter.h"

class BDiMuMuProducer : public edm::one::EDProducer<> {
public:
  using CCC = pat::CompositeCandidateCollection;
  using MVACollection = std::vector<float>;

  explicit BDiMuMuProducer(const edm::ParameterSet&);
  ~BDiMuMuProducer();

private:
  virtual void beginJob();
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void endJob();

  bool useAnyMVA_;
  bool doBPlus_;
  bool doBZero_;
  bool doBc_;

  BDiMuMuFitter theVees;
};

#endif