// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      OniaPhotonProducer
//
/**\class OniaPhotonProducer OniaPhotonProducer.h
 *
 * Description: Producer for Onia + Photon (conversion) candidates
 *              Reconstructs chi_c -> J/psi + gamma, chi_b -> Upsilon + gamma, D*0 -> D0 + gamma
 *
 * Input:
 *   - oniaSrc: pat::CompositeCandidateCollection (J/psi, Upsilon, or D0 candidates)
 *   - conversionSrc: pat::CompositeCandidateCollection from OniaPhotonConversionProducer
 *
 * Output:
 *   - pat::CompositeCandidateCollection with combined candidates
 *
 * Based on chi-analysis-miniaod package by Alberto Sanchez-Hernandez
 * Adapted for VertexCompositeAnalysis framework
 */

#ifndef VertexCompositeAnalysis_OniaPhotonProducer_h
#define VertexCompositeAnalysis_OniaPhotonProducer_h

#include <memory>
#include <vector>
#include <string>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/ESGetToken.h"

#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/TrackReco/interface/Track.h"

#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"

#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"

namespace OniaPhoton {
  // PDG masses in GeV
  constexpr double muMass    = 0.1056583745;
  constexpr double elMass    = 0.000510998946;
  constexpr double piMass    = 0.13957039;
  constexpr double kMass     = 0.493677;
  constexpr double jpsiMass  = 3.0969;
  constexpr double psi2SMass = 3.68610;
  constexpr double ups1SMass = 9.46030;
  constexpr double ups2SMass = 10.02326;
  constexpr double ups3SMass = 10.35520;
  constexpr double d0Mass    = 1.86484;
  constexpr double dstarMass = 2.00685;
  
  // Chi_c masses for reference
  constexpr double chic0Mass = 3.41471;
  constexpr double chic1Mass = 3.51067;
  constexpr double chic2Mass = 3.55617;
}

class OniaPhotonProducer : public edm::stream::EDProducer<> {
public:
  explicit OniaPhotonProducer(const edm::ParameterSet&);
  ~OniaPhotonProducer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  // Helper methods
  pat::CompositeCandidate makeChiCandidate(const pat::CompositeCandidate& onia,
                                           const pat::CompositeCandidate& photon,
                                           int oniaIdx, int convIdx);
  
  float computeDz(const pat::CompositeCandidate& conv, const reco::Candidate::Point& vtxPos);
  
  bool passDeltaMassCut(const pat::CompositeCandidate& chiCand, 
                        const pat::CompositeCandidate& oniaCand);
  
  bool passConversionCuts(const pat::CompositeCandidate& conv);
  
  // Kinematic refit
  pat::CompositeCandidate doKinematicRefit(const pat::CompositeCandidate& chiCand,
                                           const pat::CompositeCandidate& oniaCand,
                                           const pat::CompositeCandidate& photonCand,
                                           const reco::Vertex& pv,
                                           int oniaIdx);

  // Tokens
  edm::EDGetTokenT<pat::CompositeCandidateCollection> oniaToken_;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> conversionToken_;
  edm::EDGetTokenT<reco::VertexCollection> pvToken_;
  edm::EDGetTokenT<reco::BeamSpot> bsToken_;
  
  edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> ttbToken_;

  const TransientTrackBuilder* ttBuilder_;

  // Configuration parameters
  // Conversion cuts
  double minConvPt_;
  double minConvRho_;         // minimum conversion radius
  bool rejectPi0_;            // use pi0 rejection flag
  
  // Onia cuts
  double minOniaPt_;
  double maxOniaAbsY_;
  bool requireTriggerMatch_;
  
  // Matching cuts
  double dzMax_;              // max |dz| between conversion and onia vertex
  std::vector<double> deltaMassRange_;  // [min, max] for M(chi) - M(onia)
  
  // Combined candidate cuts
  double minCandPt_;
  double maxCandAbsY_;
  
  // Kinematic refit options
  bool doKinematicRefit_;
  double constraintMass_;     // mass constraint for onia (J/psi, Upsilon)
  
  // Parent type
  std::string parentType_;    // "jpsi", "upsilon1S", "upsilon2S", "upsilon3S", "d0"
  
  // Counters for endJob summary
  std::atomic<int> nCandidates_;
  std::atomic<int> nDeltaMassFail_;
  std::atomic<int> nDzFail_;
  std::atomic<int> nPi0Fail_;
};

#endif
