// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      BDiLeptonProducer
// 
/**\class BDiLeptonProducer BDiLeptonProducer.h VertexCompositeAnalysis/VertexCompositeProducer/interface/BDiLeptonProducer.h

 Description: EDProducer for B and Bc meson reconstruction from dileptons (dimuon or dielectron)

 Implementation:
     Uses BDiLeptonFitter to reconstruct B+ -> J/psi K+, B0 -> J/psi K*0, and Bc+ -> J/psi pi+ decays
     Produces collections of B and Bc meson candidates with unified format for both channels
     Input can be from HiOnia2MuMuPAT or HiOnia2EEPAT
*/
//

#ifndef VertexCompositeAnalysis__BDILEPTONPRODUCER_H
#define VertexCompositeAnalysis__BDILEPTONPRODUCER_H

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

#include "VertexCompositeAnalysis/VertexCompositeProducer/interface/BDiLeptonFitter.h"

class BDiLeptonProducer : public edm::one::EDProducer<> {
public:
  using CCC = pat::CompositeCandidateCollection;
  using MVACollection = std::vector<float>;

  explicit BDiLeptonProducer(const edm::ParameterSet&);
  ~BDiLeptonProducer();

private:
  virtual void beginJob();
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void endJob();

  bool useAnyMVA_;
  bool doBPlus_;
  bool doBZero_;
  bool doBc_;

  BDiLeptonFitter theVees;
};

#endif
