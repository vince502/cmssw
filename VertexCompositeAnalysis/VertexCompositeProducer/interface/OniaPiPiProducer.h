// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      OniaPiPiProducer
//
/**\class OniaPiPiProducer OniaPiPiProducer.h
 *
 * Description: Producer for Onia + Pi+ Pi- and Onia + e+ e- (conversion) candidates
 *              Reconstructs:
 *                - X(3872) -> J/psi + pi+ pi-
 *                - psi(2S) -> J/psi + pi+ pi-
 *                - chi_c -> J/psi + gamma -> J/psi + e+ e- (conversion from general tracks)
 *
 * Input:
 *   - oniaSrc: pat::CompositeCandidateCollection (J/psi candidates)
 *   - trackSrc: reco::TrackCollection (general tracks for pions/electrons)
 *   - vertexSrc: reco::VertexCollection
 *
 * Output:
 *   - pat::CompositeCandidateCollection with combined candidates
 */

#ifndef VertexCompositeAnalysis_OniaPiPiProducer_h
#define VertexCompositeAnalysis_OniaPiPiProducer_h

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
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/DeDxData.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"

#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"

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
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"

namespace OniaPiPi {
  // PDG masses in GeV
  constexpr double muMass    = 0.1056583745;
  constexpr double elMass    = 0.000510998946;
  constexpr double piMass    = 0.13957039;
  constexpr double jpsiMass  = 3.0969;
  constexpr double psi2SMass = 3.68610;
  constexpr double x3872Mass = 3.87165;
  
  // Chi_c masses for reference
  constexpr double chic0Mass = 3.41471;
  constexpr double chic1Mass = 3.51067;
  constexpr double chic2Mass = 3.55617;
}

class OniaPiPiProducer : public edm::stream::EDProducer<> {
public:
  explicit OniaPiPiProducer(const edm::ParameterSet&);
  ~OniaPiPiProducer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  // Helper methods for pi+pi- mode
  pat::CompositeCandidate makeOniaPiPiCandidate(
      const pat::CompositeCandidate& onia,
      const reco::TrackRef& piPlusTrack,
      const reco::TrackRef& piMinusTrack,
      const reco::Vertex& pv,
      int oniaIdx,
      double dedxPiPlus,
      double dedxPiMinus);

  // Helper methods for e+e- (conversion) mode  
  pat::CompositeCandidate makeOniaEECandidate(
      const pat::CompositeCandidate& onia,
      const reco::TrackRef& ePlusTrack,
      const reco::TrackRef& eMinusTrack,
      const reco::Vertex& pv,
      int oniaIdx,
      double dedxEPlus,
      double dedxEMinus);

  // Kinematic fitting
  bool fitVertex(const reco::TransientTrack& tk1, const reco::TransientTrack& tk2,
                 RefCountedKinematicTree& vertexTree, double mass1, double mass2);
  
  // Track quality cuts
  bool passTrackCuts(const reco::Track& track);

  // Tokens
  edm::EDGetTokenT<pat::CompositeCandidateCollection> oniaToken_;
  edm::EDGetTokenT<reco::TrackCollection> trackToken_;
  edm::EDGetTokenT<reco::VertexCollection> pvToken_;
  edm::EDGetTokenT<reco::BeamSpot> bsToken_;
  
  // dEdx tokens
  edm::EDGetTokenT<edm::ValueMap<reco::DeDxData>> dedxToken_;
  edm::EDGetTokenT<std::vector<edm::Ptr<pat::PackedCandidate>>> track2pcToken_;
  bool useDeDx_;
  
  edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> ttbToken_;
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> bFieldToken_;

  const TransientTrackBuilder* ttBuilder_;
  const MagneticField* magField_;

  // Configuration parameters
  // Mode selection
  bool doPiPi_;          // X(3872), psi(2S) -> J/psi pi+ pi-
  bool doConversion_;    // chi_c -> J/psi gamma -> J/psi e+ e-
  
  // Onia cuts
  double minOniaPt_;
  double maxOniaAbsY_;
  
  // Track cuts
  double tkPtCut_;
  double tkEtaCut_;
  double tkChi2Cut_;
  int tkNhitsCut_;
  double tkPtErrCut_;
  double tkDCACut_;        // DCA between track pair
  
  // Di-track cuts (pi+pi- or e+e-)
  double ditrackPtCut_;
  double ditrackMassMin_;
  double ditrackMassMax_;
  
  // Combined candidate cuts
  double candMassMin_;
  double candMassMax_;
  double candPtCut_;
  double candAbsYCut_;
  
  // Vertex cuts
  double vtxChi2Cut_;
  double vtxSignif3DCut_;
  double alphaCut_;
  double alpha2DCut_;
  
  // For conversion mode
  double convRadiusMin_;   // minimum conversion radius
  double convRadiusMax_;   // maximum conversion radius
  
  // Wrong-sign option
  bool isWrongSign_;
};

#endif
