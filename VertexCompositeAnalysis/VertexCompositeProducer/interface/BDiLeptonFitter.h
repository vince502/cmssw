// -*- C++ -*-
//
// Package:    VertexCompositeProducer
// Class:      BDiLeptonFitter
// 
/**\class BDiLeptonFitter BDiLeptonFitter.h VertexCompositeAnalysis/VertexCompositeProducer/interface/BDiLeptonFitter.h

 Description: B and Bc meson reconstruction from dileptons (dimuon or dielectron) and additional tracks
              Supports both HiOnia2MuMuPAT and HiOnia2EEPAT as input
              Unified output format regardless of lepton channel

 Implementation:
     Combines dilepton candidates (J/psi, psi(2S), Upsilon -> mu+mu- or e+e-) with additional tracks
     Reconstructs B+ -> J/psi K+, B0 -> J/psi K*0 (K*0 -> K+ pi-), and Bc+ -> J/psi pi+ decays
     Uses kinematic vertex fitting
*/
//

#ifndef VertexCompositeAnalysis__BDILEPTONFITTER_H
#define VertexCompositeAnalysis__BDILEPTONFITTER_H

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/Candidate/interface/VertexCompositeCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/Math/interface/angle.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "DataFormats/TrackReco/interface/DeDxData.h"

#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/AdaptiveVertexFit/interface/AdaptiveVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/TransientTrackKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"

#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackFromFTSFactory.h"

#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "MagneticField/VolumeBasedEngine/interface/VolumeBasedMagneticField.h"

#include "Geometry/CommonDetUnit/interface/TrackingGeometry.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"

#include <string>
#include <vector>
#include <memory>

class BDiLeptonFitter {
 public:
  enum class LeptonChannel { Unknown = 0, Muon = 1, Electron = 2 };

  BDiLeptonFitter(const edm::ParameterSet& theParams, edm::ConsumesCollector && iC);
  ~BDiLeptonFitter();

  void fitAll(const edm::Event& iEvent, const edm::EventSetup& iSetup);

  const pat::CompositeCandidateCollection& getBPlus() const;
  const pat::CompositeCandidateCollection& getBZero() const;
  const pat::CompositeCandidateCollection& getBc() const;
  const std::vector<float>& getMVAVals() const;

  void resetAll();

 private:
  pat::CompositeCandidateCollection theBPlus;
  pat::CompositeCandidateCollection theBZero;
  pat::CompositeCandidateCollection theBc;
  std::vector<float> mvaVals_;

  const TrackerGeometry* trackerGeom;
  const MagneticField* magField;

  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> bField_esToken_;

  // Input collections
  edm::EDGetTokenT<pat::CompositeCandidateCollection> token_dileptons;
  edm::EDGetTokenT<reco::TrackCollection> token_tracks;
  edm::EDGetTokenT<reco::VertexCollection> token_vertices;
  edm::EDGetTokenT<edm::ValueMap<reco::DeDxData>> token_dedx;
  edm::EDGetTokenT<reco::BeamSpot> token_beamSpot;

  // Track selection cuts
  double tkChi2Cut;
  int    tkNhitsCut;
  double tkPtErrCut;
  double tkPtCut;
  double tkEtaCut;
  double tkDCACut;
  double tkDCACutLow;
  
  // Dilepton selection cuts
  double dileptonMassMin;
  double dileptonMassMax;
  double dileptonPtCut;
  double dileptonYCut;
  
  // B meson selection cuts
  double bPlusMassCut;
  double bZeroMassCut;
  double bPtCut;
  double bYCut;
  
  // Bc meson selection cuts
  double bcMassCut;
  double bcPtCut;
  double bcYCut;
  
  // Vertex quality cuts
  double vtxChi2Cut;
  double vtxProbCut;
  double rVtxCut;
  double rVtxSigCut;
  double lVtxCut;
  double lVtxSigCut;
  double collinCut2D;
  double collinCut3D;
  double alphaCut;
  double alpha2DCut;
  
  // Impact parameter cuts
  double dauTransImpactSigCut;
  double dauLongImpactSigCut;
  
  // K*0 reconstruction cuts (for B0)
  double kstarMassMin;
  double kstarMassMax;
  double kstarPtCut;
  
  // Physics constants
  static constexpr double muonMass = 0.1056583745;
  static constexpr double electronMass = 0.000510998946;
  static constexpr double pionMass = 0.13957018;
  static constexpr double kaonMass = 0.493677;
  static constexpr double jpsiMass = 3.096916;
  static constexpr double psi2SMass = 3.686109;
  static constexpr double upsilonMass = 9.4603;
  static constexpr double kstarMass = 0.89555;
  static constexpr double bPlusMass = 5.27932;
  static constexpr double bZeroMass = 5.27964;
  static constexpr double bcMass = 6.2756;
  
  // Mass uncertainties for kinematic fitting
  static constexpr double muonMass_sigma = 1.0e-6;
  static constexpr double electronMass_sigma = 1.0e-9;
  static constexpr double pionMass_sigma = 3.5e-7;
  static constexpr double kaonMass_sigma = 1.6e-5;
  static constexpr double jpsiMass_sigma = 1.0e-4;
  static constexpr double bcMass_sigma = 1.0e-4;
  
  bool isWrongSign;
  bool doJPsi;
  bool doPsi2S;
  bool doUpsilon;
  bool doBPlus;
  bool doBZero;
  bool doBc;
  
  std::vector<reco::TrackBase::TrackQuality> qualities;

  // Helper methods
  LeptonChannel detectLeptonChannel(const pat::CompositeCandidate& dilepton);
  double getLeptonMass(LeptonChannel channel);
  bool getLeptonTracks(const pat::CompositeCandidate& dilepton, 
                       LeptonChannel channel,
                       const reco::Track*& trk1, 
                       const reco::Track*& trk2);
  
  bool passTrackCuts(const reco::TrackRef& track, const math::XYZPoint& bestvtx, 
                     double xVtxError, double yVtxError, double zVtxError);
  bool passDileptonCuts(const pat::CompositeCandidate& dilepton);
  
  pat::CompositeCandidate createBPlus(const pat::CompositeCandidate& dilepton, 
                                     const reco::TrackRef& kaonTrack,
                                     const reco::Vertex& vertex,
                                     const TransientVertex& bVertex,
                                     LeptonChannel channel);
  pat::CompositeCandidate createBZero(const pat::CompositeCandidate& dilepton,
                                     const reco::TrackRef& kaonTrack,
                                     const reco::TrackRef& pionTrack,
                                     const reco::Vertex& vertex,
                                     const TransientVertex& bVertex,
                                     LeptonChannel channel);
  pat::CompositeCandidate createBc(const pat::CompositeCandidate& dilepton,
                                  const reco::TrackRef& pionTrack,
                                  const reco::Vertex& vertex,
                                  const TransientVertex& bcVertex,
                                  LeptonChannel channel);
  TransientVertex fitBVertex(const std::vector<reco::TransientTrack>& tracks,
                            const std::vector<double>& masses);
  double calculatePointingAngle(const math::XYZVector& momentum,
                               const math::XYZVector& displacement);
  bool isSameTrack(const reco::Track* trk1, const reco::Track* trk2, double tolerance = 0.001);
};

#endif
