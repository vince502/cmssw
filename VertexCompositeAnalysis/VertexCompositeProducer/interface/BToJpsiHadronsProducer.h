// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      BToJpsiHadronsProducer
//
/**\class BToJpsiHadronsProducer BToJpsiHadronsProducer.h
 *
 * Description: Unified B meson producer for multiple decay modes
 *
 * Decay modes (decayId):
 *   0 = B+  -> J/psi K+    (1 hadron, K mass hypothesis)
 *   1 = B+  -> J/psi pi+   (1 hadron, pi mass hypothesis)
 *   2 = B0  -> J/psi K pi  (2 hadrons, K* -> K pi)
 *   3 = Bs  -> J/psi K K   (2 hadrons, phi -> K K)
 *   4 = Bc+ -> J/psi pi+   (1 hadron, pi mass hypothesis, Bc mass window)
 *   5 = X   -> J/psi pi pi (2 hadrons, generic X resonance)
 *
 * Input assumptions:
 *   - jpsiSrc: pat::CompositeCandidate collection of J/psi candidates
 *     with 2 lepton daughters (mu or e), fitted vertex available
 *   - trackSrc: pat::PackedCandidate or reco::Track collection
 *
 * Output:
 *   - Single std::vector<pat::CompositeCandidate> collection
 *   - Each candidate has userInt("decayId"), userInt("nTracks"), userInt("hasDaughter2")
 *   - userInt("jpsiIdx") for cross-reference with hionia tree
 *
 * Author: Auto-generated for CMS B-physics analysis
 */

#ifndef VertexCompositeAnalysis_BToJpsiHadronsProducer_h
#define VertexCompositeAnalysis_BToJpsiHadronsProducer_h

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
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/Math/interface/deltaR.h"

#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"

#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"

namespace BDecay {
  // Decay mode enumeration
  enum DecayId {
    BplusToJpsiK   = 0,  // B+ -> J/psi K+
    BplusToJpsiPi  = 1,  // B+ -> J/psi pi+
    B0ToJpsiKPi    = 2,  // B0 -> J/psi K* (K pi)
    BsToJpsiKK     = 3,  // Bs -> J/psi phi (K K)
    BcToJpsiPi     = 4,  // Bc+ -> J/psi pi+
    XToJpsiPiPi    = 5   // X -> J/psi pi pi
  };

  // PDG masses in GeV
  constexpr double muMass    = 0.1056583745;
  constexpr double elMass    = 0.000510998946;
  constexpr double piMass    = 0.13957039;
  constexpr double kMass     = 0.493677;
  constexpr double jpsiMass  = 3.0969;
  constexpr double phiMass   = 1.019461;
  constexpr double kstarMass = 0.89166;
  constexpr double bPlusMass = 5.27934;
  constexpr double b0Mass    = 5.27965;
  constexpr double bsMass    = 5.36688;
  constexpr double bcMass    = 6.2749;
}

class BToJpsiHadronsProducer : public edm::stream::EDProducer<> {
public:
  explicit BToJpsiHadronsProducer(const edm::ParameterSet&);
  ~BToJpsiHadronsProducer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginRun(edm::Run const&, edm::EventSetup const&) override;
  void produce(edm::Event&, const edm::EventSetup&) override;

  // Helper struct for track info
  struct TrackInfo {
    math::XYZTLorentzVector p4_pi;  // 4-vector with pion mass
    math::XYZTLorentzVector p4_K;   // 4-vector with kaon mass
    int charge;
    size_t index;
    const reco::Track* track;
    reco::TransientTrack transTrack;
    bool valid;
  };

  // Helper methods
  bool passTrackCuts(const pat::PackedCandidate& cand) const;
  bool passJpsiCuts(const pat::CompositeCandidate& jpsi) const;
  bool passBCuts(const math::XYZTLorentzVector& p4) const;
  
  std::vector<TrackInfo> selectTracks(const edm::Handle<pat::PackedCandidateCollection>& tracks,
                                      const pat::CompositeCandidate& jpsi);
  
  TransientVertex fitVertex(const std::vector<reco::TransientTrack>& tracks);
  
  void computeVertexQuantities(const TransientVertex& vertex,
                               const math::XYZTLorentzVector& p4,
                               const reco::Vertex& pv,
                               float& lxy, float& lxySig, float& cosAlpha,
                               float& l3D, float& l3DSig) const;

  pat::CompositeCandidate buildCandidate(const pat::CompositeCandidate& jpsi,
                                         int jpsiIdx,
                                         const TrackInfo& trk1,
                                         const TrackInfo* trk2,  // nullptr for 1-track modes
                                         BDecay::DecayId decayId,
                                         const reco::Vertex& pv);

  bool isOverlapping(const pat::CompositeCandidate& jpsi, const TrackInfo& trk) const;
  bool isOverlapping(const TrackInfo& trk1, const TrackInfo& trk2) const;

  // Tokens
  edm::EDGetTokenT<pat::CompositeCandidateCollection> jpsiToken_;
  edm::EDGetTokenT<pat::PackedCandidateCollection> trackToken_;
  edm::EDGetTokenT<reco::VertexCollection> pvToken_;
  edm::EDGetTokenT<reco::BeamSpot> bsToken_;
  
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> bFieldToken_;
  edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> ttbToken_;

  const MagneticField* bField_;
  const TransientTrackBuilder* ttBuilder_;

  // Configuration parameters
  // Track cuts
  double minTrackPt_;
  double maxTrackEta_;
  double minTrackDxySig_;
  double minTrackDzSig_;
  int minTrackNhits_;
  bool requireHighPurity_;
  
  // J/psi cuts
  double minJpsiPt_;
  double maxJpsiAbsY_;
  double jpsiMassMin_;
  double jpsiMassMax_;
  
  // B candidate cuts
  double minBPt_;
  double maxBAbsY_;
  double minVtxProb_;
  
  // Resonance mass windows
  double phiMassWindow_;
  double kstarMassWindow_;
  double xPiPiMassMin_;
  double xPiPiMassMax_;
  
  // B mass windows for each decay mode
  double bPlusMassWindow_;
  double b0MassWindow_;
  double bsMassWindow_;
  double bcMassWindow_;
  double xMassMin_;
  double xMassMax_;
  
  // Flags to enable/disable modes
  bool doBplusToJpsiK_;
  bool doBplusToJpsiPi_;
  bool doB0ToJpsiKPi_;
  bool doBsToJpsiKK_;
  bool doBcToJpsiPi_;
  bool doXToJpsiPiPi_;
  
  // Other options
  bool doVertexFit_;
  double maxDeltaR_overlap_;
};

#endif
