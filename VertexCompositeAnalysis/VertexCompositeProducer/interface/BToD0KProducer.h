// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      BToD0KProducer
//
/**\class BToD0KProducer BToD0KProducer.h
 *
 * Reconstructs B+/- -> D0 K+/- candidates from an existing D0
 * pat::CompositeCandidate collection and packed charged candidates.
 *
 * The default selection builds right-sign B candidates:
 *   B+ -> anti-D0 K+
 *   B- -> D0 K-
 *
 * Output:
 *   std::vector<pat::CompositeCandidate> with product label "BToD0K"
 */

#ifndef VertexCompositeAnalysis_BToD0KProducer_h
#define VertexCompositeAnalysis_BToD0KProducer_h

#include <memory>
#include <vector>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/ESGetToken.h"

#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/TrackReco/interface/DeDxData.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"

#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"

class BToD0KProducer : public edm::stream::EDProducer<> {
public:
  explicit BToD0KProducer(const edm::ParameterSet&);
  ~BToD0KProducer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  struct KaonInfo {
    reco::Candidate::LorentzVector p4;
    int charge;
    size_t index;
    reco::Track track;
    reco::TransientTrack transientTrack;
    float dedx;
    bool valid;
  };

  bool passD0Cuts(const pat::CompositeCandidate& d0) const;
  bool passKaonCuts(const pat::PackedCandidate& kaon) const;
  bool passBCuts(const reco::Candidate::LorentzVector& p4) const;
  bool passChargeSelection(const pat::CompositeCandidate& d0, const KaonInfo& kaon) const;
  bool isOverlappingWithD0(const pat::CompositeCandidate& d0, const KaonInfo& kaon) const;

  std::vector<KaonInfo> selectKaons(const edm::Handle<pat::PackedCandidateCollection>& kaons,
                                    const pat::CompositeCandidate& d0,
                                    const edm::Handle<edm::ValueMap<reco::DeDxData>>& dEdxHandle) const;
  bool getD0DaughterTracks(const pat::CompositeCandidate& d0,
                           std::vector<reco::TransientTrack>& tracks) const;
  TransientVertex fitVertex(const std::vector<reco::TransientTrack>& tracks) const;
  void computeVertexQuantities(const TransientVertex& vertex,
                               const reco::Candidate::LorentzVector& p4,
                               const reco::Vertex& pv,
                               float& lxy,
                               float& lxySig,
                               float& l3D,
                               float& l3DSig,
                               float& alpha2D,
                               float& alpha3D,
                               float& cosAlpha,
                               float& dca3D,
                               float& dca3DErr,
                               float& track3DDCA,
                               float& track3DDCAErr) const;

  pat::CompositeCandidate buildCandidate(const pat::CompositeCandidate& d0,
                                         int d0Idx,
                                         const KaonInfo& kaon,
                                         const reco::Vertex& pv) const;

  edm::EDGetTokenT<pat::CompositeCandidateCollection> d0Token_;
  edm::EDGetTokenT<pat::PackedCandidateCollection> kaonToken_;
  edm::EDGetTokenT<reco::VertexCollection> pvToken_;
  edm::EDGetTokenT<reco::BeamSpot> bsToken_;
  edm::EDGetTokenT<edm::ValueMap<reco::DeDxData>> dedxToken_;
  edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> ttbToken_;

  const TransientTrackBuilder* ttBuilder_;

  double minKaonPt_;
  double maxKaonEta_;
  int minKaonNhits_;
  bool requireHighPurity_;

  double minD0Pt_;
  double maxD0AbsY_;
  double d0MassMin_;
  double d0MassMax_;
  bool requireD0PdgId_;
  bool isWrongSign_;

  double minBPt_;
  double maxBAbsY_;
  double bMassMin_;
  double bMassMax_;
  double minVtxProb_;

  bool doVertexFit_;
  double maxDeltaR_overlap_;
  bool useDeDx_;
};

#endif
